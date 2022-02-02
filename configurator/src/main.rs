// Copyright 2021 Hiram Silvey

use anyhow::{anyhow, Result};
use configurator::encoder;
use configurator::profiles;
use crossbeam_channel::{unbounded, Receiver, Sender};
use druid::kurbo::{Circle, Line};
use druid::piet::StrokeStyle;
use druid::widget::prelude::*;
use druid::widget::{Button, Flex, Label, Painter};
use druid::{
    theme, AppLauncher, Color, Data, ExtEventSink, Lens, PlatformError, Point, Rect, RenderContext,
    Selector, Target, Widget, WidgetExt, WindowDesc,
};
use serialport::SerialPort;
use std::path::Path;
use std::thread;
use std::thread::sleep;
use std::time::{Duration, Instant};

const MAX_EEPROM_BYTES: usize = 1064;
const JOYSTICK_CURSOR_RADIUS: f64 = 3.;
const JOYSTICK_BOX_LENGTH: f64 = 400.;
const SET_CURSOR: Selector<Point> = Selector::new("hs.set-cursor");
const SET_DEFAULT_BOUNDS: Selector<Bounds> = Selector::new("hs.set-default-bounds");

#[derive(Clone, Copy)]
enum Command {
    FetchStoredBounds,
    FetchJoystickCoords,
    CalibrateJoystick,
    SaveCalibration,
    StoreProfiles,
}

#[derive(Clone, Copy, Data, Lens)]
struct Bounds {
    center: Point,
    range: f64,
    angle_ticks: i16,
}

impl Bounds {
    pub fn new() -> Bounds {
        Bounds {
            center: Point::new(0.0, 0.0),
            range: 0.0,
            angle_ticks: 0,
        }
    }

    pub fn from_parts(center_x: f64, center_y: f64, range: f64, angle_ticks: i16) -> Bounds {
        Bounds {
            center: Point::new(center_x, center_y),
            range: range,
            angle_ticks: angle_ticks,
        }
    }

    fn get_x_min(&self) -> f64 {
        self.center.x - self.range
    }

    fn get_x_max(&self) -> f64 {
        self.center.x + self.range
    }

    fn get_y_min(&self) -> f64 {
        self.center.y + self.range
    }

    fn get_y_max(&self) -> f64 {
        self.center.y - self.range
    }
}

#[derive(Clone, Copy)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
    In,
    Out,
    AngleDec,
    AngleInc,
}

#[derive(Clone, Data, Lens)]
struct JoystickState {
    cursor: Point,
    default_bounds: Bounds,
    custom_bounds: Bounds,
    digital_threshold: f64,
}

impl JoystickState {
    pub fn new() -> JoystickState {
        JoystickState {
            cursor: Point::new(0.0, 0.0),
            default_bounds: Bounds::new(),
            custom_bounds: Bounds::new(),
            digital_threshold: 0.0,
        }
    }

    fn set_cursor(&mut self, point: &Point) {
        self.cursor = *point;
    }

    fn set_bounds(&mut self, bounds: &Bounds) {
        self.default_bounds = *bounds;
        self.custom_bounds = *bounds;

        println!(
            "Set bounds to x={}, y={}, range={}, angle_ticks={}",
            self.default_bounds.center.x,
            self.default_bounds.center.y,
            self.default_bounds.range,
            self.default_bounds.angle_ticks
        );
    }

    fn shift_bounds(&mut self, direction: Direction) {
        let tick = self.default_bounds.range / 1000.0;

        match direction {
            Direction::Up => self.custom_bounds.center.y += tick,
            Direction::Down => self.custom_bounds.center.y -= tick,
            Direction::Left => self.custom_bounds.center.x -= tick,
            Direction::Right => self.custom_bounds.center.x += tick,
            Direction::In => self.custom_bounds.range -= tick,
            Direction::Out => self.custom_bounds.range += tick,
            Direction::AngleDec => self.custom_bounds.angle_ticks -= 1,
            Direction::AngleInc => self.custom_bounds.angle_ticks += 1,
        }
    }
}

fn get_button_painter() -> Painter<JoystickState> {
    Painter::new(|ctx, _, env| {
        let bounds = ctx.size().to_rect();

        ctx.fill(bounds, &env.get(theme::PRIMARY_DARK));

        if ctx.is_hot() {
            ctx.stroke(bounds.inset(-0.5), &Color::WHITE, 1.0);
        }

        if ctx.is_active() {
            ctx.fill(bounds, &env.get(theme::PRIMARY_LIGHT));
        }
    })
}

fn edit_bounds_button(direction: Direction) -> impl Widget<JoystickState> {
    let symbol = match direction {
        Direction::Up => '↑',
        Direction::Down => '↓',
        Direction::Left => '←',
        Direction::Right => '→',
        Direction::In => '-',
        Direction::Out => '+',
        Direction::AngleDec => '⟲',
        Direction::AngleInc => '⟳',
    };

    Label::new(format!("{}", symbol))
        .with_text_size(14.0)
        .center()
        .background(get_button_painter())
        .expand()
        .on_click(move |_ctx, data: &mut JoystickState, _env| data.shift_bounds(direction))
}

fn edit_threshold_button(direction: Direction) -> impl Widget<JoystickState> {
    let (symbol, val) = match direction {
        Direction::In => ("D-", -1.0),
        Direction::Out => ("D+", 1.0),
        _ => ("", 0.0),
    };

    Label::new(format!("{}", symbol))
        .with_text_size(14.0)
        .center()
        .background(get_button_painter())
        .expand()
        .on_click(move |_ctx, data: &mut JoystickState, _env| data.digital_threshold += val)
}

fn map(x: f64, in_min: f64, in_max: f64, out_min: f64, out_max: f64) -> f64 {
    (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
}

fn relative_rotated_point(x: f64, y: f64, origin_x: f64, origin_y: f64, angle_ticks: i16) -> Point {
    let relative_x = x - origin_x;
    let relative_y = y - origin_y;

    // It takes 1000 angle ticks to reach PI/2, or 90 degrees. We alse flip the
    // sign since ultimately we will be rotating the joystick point and not the
    // axes.
    let angle = -(std::f64::consts::PI * angle_ticks as f64) / 2000.;

    let mut p = Point::new(
        (relative_x * angle.cos()) + (relative_y * angle.sin()),
        (-relative_x * angle.sin()) + (relative_y * angle.cos()),
    );
    p.x += origin_x;
    p.y += origin_y;
    p
}

impl Widget<JoystickState> for JoystickState {
    fn event(&mut self, _ctx: &mut EventCtx, event: &Event, data: &mut JoystickState, _env: &Env) {
        match event {
            Event::Command(cmd) if cmd.is(SET_CURSOR) => {
                let point = cmd.get_unchecked(SET_CURSOR).clone();
                data.set_cursor(&point);
            }
            Event::Command(cmd) if cmd.is(SET_DEFAULT_BOUNDS) => {
                let bounds = cmd.get_unchecked(SET_DEFAULT_BOUNDS).clone();
                data.set_bounds(&bounds);
            }
            _ => (),
        }
    }

    fn lifecycle(
        &mut self,
        _ctx: &mut LifeCycleCtx,
        _event: &LifeCycle,
        _data: &JoystickState,
        _env: &Env,
    ) {
    }

    fn update(
        &mut self,
        ctx: &mut UpdateCtx,
        _old_data: &JoystickState,
        _data: &JoystickState,
        _env: &Env,
    ) {
        ctx.request_paint();
    }

    fn layout(
        &mut self,
        _layout_ctx: &mut LayoutCtx,
        bc: &BoxConstraints,
        _data: &JoystickState,
        _env: &Env,
    ) -> Size {
        if bc.is_width_bounded() | bc.is_height_bounded() {
            let size = Size::new(JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH);
            bc.constrain(size)
        } else {
            bc.max()
        }
    }

    fn paint(&mut self, ctx: &mut PaintCtx, data: &JoystickState, env: &Env) {
        // Absolute bounds.
        let size = ctx.size();
        let rect = size.to_rect();
        ctx.fill(rect, &env.get(theme::BACKGROUND_DARK));
        ctx.stroke(rect, &env.get(theme::BACKGROUND_LIGHT), 1.0);

        let default_range = (size.width * 0.875) / 2.0;
        let default_x_min = rect.center().x - default_range;
        let default_x_max = rect.center().x + default_range;
        let default_y_min = rect.center().y - default_range;
        let default_y_max = rect.center().y + default_range;

        // Default bounds.
        let default_bounds = Rect::from_points(
            Point::new(default_x_min, default_y_min),
            Point::new(default_x_max, default_y_max),
        );
        ctx.stroke(default_bounds, &env.get(theme::BACKGROUND_LIGHT), 1.0);

        // Mutable bounds.
        let mutable_x_min = map(
            data.custom_bounds.get_x_min(),
            data.default_bounds.get_x_min(),
            data.default_bounds.get_x_max(),
            default_x_min,
            default_x_max,
        );
        let mutable_x_max = map(
            data.custom_bounds.get_x_max(),
            data.default_bounds.get_x_min(),
            data.default_bounds.get_x_max(),
            default_x_min,
            default_x_max,
        );
        let mutable_x_center = mutable_x_min + (mutable_x_max - mutable_x_min) / 2.0;
        let mutable_y_min = map(
            data.custom_bounds.get_y_min(),
            data.default_bounds.get_y_min(),
            data.default_bounds.get_y_max(),
            default_y_min,
            default_y_max,
        );
        let mutable_y_max = map(
            data.custom_bounds.get_y_max(),
            data.default_bounds.get_y_min(),
            data.default_bounds.get_y_max(),
            default_y_min,
            default_y_max,
        );
        let mutable_y_center = mutable_y_min + (mutable_y_max - mutable_y_min) / 2.0;
        let mutable_center = Point::new(mutable_x_center, mutable_y_center);

        let mutable_top_left = relative_rotated_point(
            mutable_x_min,
            mutable_y_min,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_top_right = relative_rotated_point(
            mutable_x_max,
            mutable_y_min,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_bottom_left = relative_rotated_point(
            mutable_x_min,
            mutable_y_max,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_bottom_right = relative_rotated_point(
            mutable_x_max,
            mutable_y_max,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_center_left = relative_rotated_point(
            mutable_x_min,
            mutable_y_center,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_center_right = relative_rotated_point(
            mutable_x_max,
            mutable_y_center,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_center_top = relative_rotated_point(
            mutable_x_center,
            mutable_y_min,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );
        let mutable_center_bottom = relative_rotated_point(
            mutable_x_center,
            mutable_y_max,
            mutable_x_center,
            mutable_y_center,
            data.custom_bounds.angle_ticks,
        );

        // Mutable bounds' box.
        ctx.stroke(
            Line::new(mutable_top_left, mutable_bottom_left),
            &env.get(theme::PRIMARY_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(mutable_top_left, mutable_top_right),
            &env.get(theme::PRIMARY_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(mutable_bottom_right, mutable_top_right),
            &env.get(theme::PRIMARY_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(mutable_bottom_right, mutable_bottom_left),
            &env.get(theme::PRIMARY_LIGHT),
            1.0,
        );

        // Mutable bounds' guidelines.
        let stroke_style = StrokeStyle::new().dash_pattern(&[1.0, 4.0]);
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_center_left),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_top_left),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_center_top),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_top_right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_center_right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_bottom_right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_center_bottom),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(mutable_center, mutable_bottom_left),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );

        // Mutable digital threshold.
        let threshold_x_range =
            (mutable_x_max - mutable_x_center) * (data.digital_threshold / 100.0);
        let threshold_y_range =
            (mutable_y_max - mutable_y_center) * (data.digital_threshold / 100.0);
        let threshold = Rect::from_points(
            Point::new(
                mutable_x_center - threshold_x_range,
                mutable_y_center - threshold_y_range,
            ),
            Point::new(
                mutable_x_center + threshold_x_range,
                mutable_y_center + threshold_y_range,
            ),
        );
        ctx.stroke(threshold, &env.get(theme::PRIMARY_LIGHT), 1.0);

        // Cursor.
        let cursor_x = map(
            data.cursor.x,
            data.default_bounds.get_x_min(),
            data.default_bounds.get_x_max(),
            default_x_min,
            default_x_max,
        );
        let cursor_y = map(
            data.cursor.y,
            data.default_bounds.get_y_min(),
            data.default_bounds.get_y_max(),
            default_y_min,
            default_y_max,
        );
        let cursor_color = env.get(theme::CURSOR_COLOR);
        ctx.paint_with_z_index(1, move |ctx| {
            let cursor = Circle::new(Point::new(cursor_x, cursor_y), JOYSTICK_CURSOR_RADIUS);
            ctx.stroke(cursor, &cursor_color, 2.0);
        });
    }
}

fn connect() -> Result<Box<dyn SerialPort>> {
    let port = "/dev/ttyACM0";
    match serialport::new(port, 9600)
        .timeout(Duration::from_millis(10))
        .open()
    {
        Ok(x) => Ok(x),
        Err(e) => Err(anyhow!(
            "Unable to connect to HS via serial port {}: {}",
            port,
            e
        )),
    }
}

fn wait_for_data(hs: &mut Box<dyn SerialPort>, buf: &mut Vec<u8>) -> Result<()> {
    let now = Instant::now();
    let timeout = Duration::new(70, 0);
    let mut status = hs.read_exact(buf);
    while status.is_err() && now.elapsed() < timeout {
        // println!("Waiting...");
        // sleep(Duration::new(1, 0));
        status = hs.read_exact(buf);
    }
    if status.is_err() {
        return Err(anyhow!(
            "Failed waiting for data from HS: {}",
            status.unwrap_err()
        ));
    }
    Ok(())
}

fn wait_for_ack(hs: &mut Box<dyn SerialPort>) -> Result<()> {
    let mut buf = vec![0u8; 1];
    wait_for_data(hs, &mut buf)?;
    if buf[0] != 0 {
        return Err(anyhow!("Failed to receive ACK. Wanted 0, got {}.", buf[0]));
    }
    Ok(())
}

fn bytes_to_short(bytes: &Vec<u8>) -> Result<i16> {
    if bytes.len() != 2 {
        return Err(anyhow!(
            "Failed converting bytes to short. Expected 2 bytes, found {} bytes.",
            bytes.len()
        ));
    }
    Ok(((bytes[0] as i16) << 8) | bytes[1] as i16)
}

fn bytes_to_float(bytes: &Vec<u8>) -> Result<f64> {
    if bytes.len() != 4 {
        return Err(anyhow!(
            "Failed converting bytes to float. Expected 4 bytes, found {} bytes.",
            bytes.len()
        ));
    }
    Ok((((bytes[0] as i32) << 24)
        | ((bytes[1] as i32) << 16)
        | ((bytes[2] as i32) << 8)
        | bytes[3] as i32) as f64)
}

fn short_to_bytes(data: i16) -> Vec<u8> {
    vec![(data >> 8 & 0xFF) as u8, (data & 0xFF) as u8]
}

fn float_to_bytes(data: f64) -> Vec<u8> {
    let i32_data = data as i32;
    vec![
        (i32_data >> 24) as u8,
        (i32_data >> 16 & 0xFF) as u8,
        (i32_data >> 8 & 0xFF) as u8,
        (i32_data & 0xFF) as u8,
    ]
}

fn fetch_stored_bounds(hs: &mut Box<dyn SerialPort>, event_sink: &ExtEventSink) -> Result<()> {
    let mut center_x = vec![0u8; 4];
    let mut center_y = vec![0u8; 4];
    let mut range = vec![0u8; 4];
    // let mut angle_ticks = vec![0u8; 2];
    wait_for_data(hs, &mut center_x)?;
    wait_for_data(hs, &mut center_y)?;
    wait_for_data(hs, &mut range)?;
    // wait_for_data(hs, &mut angle_ticks)?;

    event_sink.submit_command(
        SET_DEFAULT_BOUNDS,
        Bounds::from_parts(
            bytes_to_float(&center_x)?,
            bytes_to_float(&center_y)?,
            bytes_to_float(&range)?,
            0, // bytes_to_short(&angle_ticks)?,
        ),
        Target::Auto,
    )?;

    Ok(())
}

fn fetch_joystick_coords(hs: &mut Box<dyn SerialPort>, event_sink: &ExtEventSink) -> Result<()> {
    let mut x = vec![0u8; 4];
    let mut y = vec![0u8; 4];
    wait_for_data(hs, &mut x)?;
    wait_for_data(hs, &mut y)?;

    event_sink.submit_command(
        SET_CURSOR,
        Point::new(bytes_to_float(&x)?, bytes_to_float(&y)?),
        Target::Auto,
    )?;

    Ok(())
}

fn calibrate_joystick(hs: &mut Box<dyn SerialPort>, sender: &Sender<f64>) -> Result<()> {
    let mut center_x_buf = vec![0u8; 4];
    let mut center_y_buf = vec![0u8; 4];
    let mut range_buf = vec![0u8; 4];
    wait_for_data(hs, &mut center_x_buf)?;
    wait_for_data(hs, &mut center_y_buf)?;
    wait_for_data(hs, &mut range_buf)?;

    sender.send(bytes_to_float(&center_x_buf)?)?;
    sender.send(bytes_to_float(&center_y_buf)?)?;
    sender.send(bytes_to_float(&range_buf)?)?;

    Ok(())
}

fn save_calibration(
    hs: &mut Box<dyn SerialPort>,
    sender: &Sender<f64>,
    receiver: &Receiver<f64>,
) -> Result<()> {
    let center_x = receiver.recv()?;
    let center_y = receiver.recv()?;
    let range = receiver.recv()?;
    let angle_ticks = receiver.recv()?;

    hs.write_all(&float_to_bytes(center_x))?;
    hs.write_all(&float_to_bytes(center_y))?;
    hs.write_all(&float_to_bytes(range))?;
    hs.write_all(&short_to_bytes(angle_ticks as i16))?;

    wait_for_ack(hs)?;

    sender.send(0.0)?;
    Ok(())
}

fn store_profiles(hs: &mut Box<dyn SerialPort>, sender: &Sender<f64>) -> Result<()> {
    let profiles = match profiles::load_all(&Path::new("../profiles")) {
        Ok(x) => x,
        Err(e) => return Err(anyhow!("Unable to load profiles: {}", e)),
    };

    // Print as useful debug information.
    for profile in &profiles {
        println!("{}", profile);
    }

    let encoded = match encoder::encode(&profiles) {
        Ok(x) => x,
        Err(e) => return Err(anyhow!("Unable to encode profiles: {}", e)),
    };
    if encoded.len() > MAX_EEPROM_BYTES {
        return Err(anyhow!(
            "Encoded length of {} bytes exceeds HS maximum of {} bytes.",
            encoded.len(),
            MAX_EEPROM_BYTES,
        ));
    }

    let size = encoded.len();
    hs.write_all(&[(size >> 8) as u8, (size & 0xFF) as u8])?;
    hs.write_all(&encoded)?;
    wait_for_ack(hs)?;

    sender.send(0.0)?;
    Ok(())
}

fn build_ui(
    cmd_sender: Sender<Command>,
    data_sender: Sender<f64>,
    receiver: Receiver<f64>,
) -> impl Widget<JoystickState> {
    let (cmd_sender2, receiver2) = (cmd_sender.clone(), receiver.clone());
    let (cmd_sender3, receiver3) = (cmd_sender.clone(), receiver.clone());

    let threshold_display = Label::new(|data: &f64, _env: &_| format!("{}%", data))
        .with_text_size(14.0)
        .lens(JoystickState::digital_threshold);

    Flex::column()
        .with_child(JoystickState::new())
        .with_flex_child(
            Flex::row()
                .with_flex_child(edit_bounds_button(Direction::Left), 1.0)
                .with_spacer(1.0)
                .with_flex_child(
                    Flex::column()
                        .with_flex_child(edit_bounds_button(Direction::Up), 1.0)
                        .with_spacer(1.0)
                        .with_flex_child(edit_bounds_button(Direction::Down), 1.0),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(edit_bounds_button(Direction::Right), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_bounds_button(Direction::In), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_bounds_button(Direction::Out), 1.0)
                .with_spacer(1.0)
                .with_flex_child(threshold_display, 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_threshold_button(Direction::In), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_threshold_button(Direction::Out), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_bounds_button(Direction::AngleDec), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_bounds_button(Direction::AngleInc), 1.0),
            1.0,
        )
        .with_flex_child(
            Button::new("Calibrate Joystick").on_click(
                move |_event, data: &mut JoystickState, _env| {
                    match cmd_sender.send(Command::CalibrateJoystick) {
                        Ok(()) => println!("Calibration starting..."),
                        Err(e) => {
                            println!("Failed issuing 'calibrate joystick' command: {}", e);
                            return;
                        }
                    }
                    let center_x = match receiver.recv() {
                        Ok(x) => x,
                        Err(e) => {
                            println!("Failed getting joystick center x value: {}", e);
                            return;
                        }
                    };
                    let center_y = match receiver.recv() {
                        Ok(y) => y,
                        Err(e) => {
                            println!("Failed getting joystick center y value: {}", e);
                            return;
                        }
                    };
                    let range = match receiver.recv() {
                        Ok(r) => r,
                        Err(e) => {
                            println!("Failed getting joystick range value: {}", e);
                            return;
                        }
                    };
                    println!(
                        "Setting bounds to x={}, y={}, range={}, angle_ticks={}",
                        center_x, center_y, range, 0
                    );
                    data.set_bounds(&Bounds {
                        center: Point::new(center_x, center_y),
                        range: range,
                        angle_ticks: 0,
                    });
                },
            ),
            1.0,
        )
        .with_flex_child(
            Button::new("Save Calibration").on_click(
                move |_event, data: &mut JoystickState, _env| {
                    match cmd_sender2.send(Command::SaveCalibration) {
                        Ok(()) => println!("Saving calibration..."),
                        Err(e) => {
                            println!("Failed issuing 'save calibration' command: {}", e);
                            return;
                        }
                    }
                    match data_sender.send(data.custom_bounds.center.x) {
                        Ok(()) => println!("Sent center x coordinate."),
                        Err(e) => {
                            println!("Failed to send center x coordinate: {}", e);
                            return;
                        }
                    }
                    match data_sender.send(data.custom_bounds.center.y) {
                        Ok(()) => println!("Sent center y coordinate."),
                        Err(e) => {
                            println!("Failed to send center y coordinate: {}", e);
                            return;
                        }
                    }
                    match data_sender.send(data.custom_bounds.range) {
                        Ok(()) => println!("Sent range value."),
                        Err(e) => {
                            println!("Failed to send range value: {}", e);
                            return;
                        }
                    }
                    match data_sender.send(data.custom_bounds.angle_ticks.into()) {
                        Ok(()) => println!("Sent angle ticks."),
                        Err(e) => {
                            println!("Failed to send angle ticks: {}", e);
                            return;
                        }
                    }

                    if let Err(e) = receiver2.recv() {
                        println!("Failed to save calibration: {}", e);
                    };
                },
            ),
            1.0,
        )
        .with_flex_child(
            Button::new("Store Profiles").on_click(move |_event, _data, _env| {
                match cmd_sender3.send(Command::StoreProfiles) {
                    Ok(()) => println!("Storing profiles..."),
                    Err(e) => {
                        println!("Failed issuing 'store profiles' command: {}", e);
                        return;
                    }
                }
                if let Err(e) = receiver3.recv() {
                    println!("Failed to store profiles: {}", e);
                };
            }),
            1.0,
        )
}

fn drive(
    sender: Sender<f64>,
    data_receiver: Receiver<f64>,
    command_receiver: Receiver<Command>,
    event_sink: ExtEventSink,
) -> Result<()> {
    let mut hs = connect()?;

    hs.write_all(&[Command::FetchStoredBounds as u8])?;
    wait_for_ack(&mut hs)?;
    fetch_stored_bounds(&mut hs, &event_sink)?;

    loop {
        let cmd = match command_receiver.try_recv() {
            Ok(x) => x,
            Err(_) => Command::FetchJoystickCoords,
        };

        hs.write_all(&[cmd as u8])?;
        wait_for_ack(&mut hs)?;
        match cmd {
            Command::FetchStoredBounds => fetch_stored_bounds(&mut hs, &event_sink)?,
            Command::FetchJoystickCoords => fetch_joystick_coords(&mut hs, &event_sink)?,
            Command::CalibrateJoystick => calibrate_joystick(&mut hs, &sender)?,
            Command::SaveCalibration => save_calibration(&mut hs, &sender, &data_receiver)?,
            Command::StoreProfiles => store_profiles(&mut hs, &sender)?,
        };
    }
}

pub fn main() -> Result<(), PlatformError> {
    let (cmd_sender, cmd_receiver) = unbounded();
    let (child_data_sender, parent_data_receiver) = unbounded();
    let (parent_data_sender, child_data_receiver) = unbounded();

    let launcher = AppLauncher::with_window(
        WindowDesc::new(build_ui(
            cmd_sender,
            parent_data_sender,
            parent_data_receiver,
        ))
        .title("HS Configurator")
        .window_size((JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH + 150.0))
        .with_min_size((JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH)),
    );

    let event_sink = launcher.get_external_handle();
    thread::spawn(move || -> Result<()> {
        drive(
            child_data_sender,
            child_data_receiver,
            cmd_receiver,
            event_sink,
        )
    });

    launcher.launch(JoystickState::new())?;

    Ok(())
}
