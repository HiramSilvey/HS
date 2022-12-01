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
use std::time::{Duration, Instant};

const MAX_EEPROM_BYTES: usize = 1064;
const JOYSTICK_CURSOR_RADIUS: f64 = 3.;
const JOYSTICK_BOX_LENGTH: f64 = 400.;
const SET_CURSOR: Selector<Point> = Selector::new("hs.set-cursor");

#[derive(Clone, Copy)]
enum Command {
    FetchJoystickCoords,
    CalibrateJoystick,
    SaveCalibration,
    StoreProfiles,
    SetXYIn,
    IncXYAngle,
    DecXYAngle,
    IncXZAngle,
    DecXZAngle,
    IncYZAngle,
    DecYZAngle,
}

#[derive(Clone, Copy)]
enum Direction {
    Up,
    Down,
    Left,
    Right,
    In,
    Out,
    Inc,
    Dec,
}

#[derive(Clone, Data, Lens)]
struct JoystickState {
    cursor: Point,
}

impl JoystickState {
    pub fn new() -> JoystickState {
        JoystickState {
            cursor: Point::new(0.0, 0.0),
        }
    }

    fn set_cursor(&mut self, point: &Point) {
        self.cursor = *point;
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

fn edit_threshold_button(
    direction: Direction,
    cmd_sender: Sender<Command>,
    data_sender: Sender<f64>,
    receiver: Receiver<f64>,
) -> impl Widget<JoystickState> {
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
        .on_click(move |_ctx, data: &mut JoystickState, _env| {
            match cmd_sender.send(Command::SetThreshold) {
                Ok(()) => println!("Setting digital threshold..."),
                Err(e) => {
                    println!("Failed issuing 'set threshold' command: {}", e);
                    return;
                }
            }
            match data_sender.send(data.digital_threshold) {
                Ok(()) => println!("Sent digital threshold {}.", data.digital_threshold),
                Err(e) => {
                    println!(
                        "Failed to send digital threshold {}: {}",
                        data.digital_threshold, e
                    );
                    return;
                }
            }
            if let Err(e) = receiver.recv() {
                println!("Failed to update threshold: {}", e);
            };
            data.digital_threshold += val
        })
}

fn map(x: f64, in_min: f64, in_max: f64, out_min: f64, out_max: f64) -> f64 {
    (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
}

impl Widget<JoystickState> for JoystickState {
    fn event(&mut self, _ctx: &mut EventCtx, event: &Event, data: &mut JoystickState, _env: &Env) {
        match event {
            Event::Command(cmd) if cmd.is(SET_CURSOR) => {
                let point = cmd.get_unchecked(SET_CURSOR).clone();
                data.set_cursor(&point);
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
        // Bounds.
        let size = ctx.size();
        let rect = size.to_rect();
        ctx.fill(rect, &env.get(theme::BACKGROUND_DARK));
        ctx.stroke(rect, &env.get(theme::PRIMARY_LIGHT), 1.0);

        // Guidelines.
        let top = Point::new(rect.center().x, rect.min_y());
        let bot = Point::new(rect.center().x, rect.max_y());
        let left = Point::new(rect.min_x(), rect.center().y);
        let right = Point::new(rect.max_x(), rect.center().y);

        let top_left = Point::new(rect.min_x(), rect.min_y());
        let top_right = Point::new(rect.max_x(), rect.min_y());
        let bot_left = Point::new(rect.min_x(), rect.max_y());
        let bot_right = Point::new(rect.max_x(), rect.max_y());

        let stroke_style = StrokeStyle::new().dash_pattern(&[1.0, 4.0]);
        ctx.stroke_styled(
            Line::new(top, bot),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(left, right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(top_left, bot_right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );
        ctx.stroke_styled(
            Line::new(bot_left, top_right),
            &env.get(theme::PRIMARY_DARK),
            1.0,
            &stroke_style,
        );

        // Cursor.
        let cursor_x = map(data.cursor.x, 0, 1023, rect.min_x(), rect.max_x());
        let cursor_y = map(data.cursor.y, 0, 1023, rect.min_y(), rect.max_y());
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
    let xy_angle_ticks = receiver.recv()?;
    let xz_angle_ticks = receiver.recv()?;
    let yz_angle_ticks = receiver.recv()?;

    hs.write_all(&float_to_bytes(center_x))?;
    hs.write_all(&float_to_bytes(center_y))?;
    hs.write_all(&float_to_bytes(range))?;
    hs.write_all(&short_to_bytes(xy_angle_ticks as i16))?;
    hs.write_all(&short_to_bytes(xz_angle_ticks as i16))?;
    hs.write_all(&short_to_bytes(yz_angle_ticks as i16))?;

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

fn set_xy_in(hs: &mut Box<dyn SerialPort>, sender: &Sender<f64>) -> Result<()> {}

fn build_ui(
    cmd_sender: Sender<Command>,
    data_sender: Sender<f64>,
    receiver: Receiver<f64>,
) -> impl Widget<JoystickState> {
    let (cmd_sender2, data_sender2, receiver2) =
        (cmd_sender.clone(), data_sender.clone(), receiver.clone());
    let (cmd_sender3, data_sender3, receiver3) =
        (cmd_sender.clone(), data_sender.clone(), receiver.clone());
    let (cmd_sender4, data_sender4, receiver4) =
        (cmd_sender.clone(), data_sender.clone(), receiver.clone());
    let (cmd_sender5, data_sender5, receiver5) =
        (cmd_sender.clone(), data_sender.clone(), receiver.clone());
    let (cmd_sender6, receiver6) = (cmd_sender.clone(), receiver.clone());
    let (cmd_sender7, receiver7) = (cmd_sender.clone(), receiver.clone());

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
                .with_flex_child(
                    edit_threshold_button(Direction::In, cmd_sender, data_sender, receiver),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_threshold_button(Direction::Out, cmd_sender, data_sender, receiver),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_xy_angle_button(Direction::Dec, cmd_sender2, data_sender2, receiver2),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_xy_angle_button(Direction::Inc, cmd_sender2, data_sender2, receiver2),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_xz_angle_button(Direction::Dec, cmd_sender3, data_sender3, receiver3),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_xz_angle_button(Direction::Inc, cmd_sender3, data_sender3, receiver3),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_yz_angle_button(Direction::Dec, cmd_sender4, data_sender4, receiver4),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(
                    edit_yz_angle_button(Direction::Inc, cmd_sender4, data_sender4, receiver4),
                    1.0,
                ),
            1.0,
        )
        .with_flex_child(
            Button::new("Calibrate Joystick").on_click(
                move |_event, data: &mut JoystickState, _env| {
                    match cmd_sender5.send(Command::CalibrateJoystick) {
                        Ok(()) => println!("Calibration starting..."),
                        Err(e) => {
                            println!("Failed issuing 'calibrate joystick' command: {}", e);
                            return;
                        }
                    }
                    let center_x = match receiver5.recv() {
                        Ok(x) => x,
                        Err(e) => {
                            println!("Failed getting joystick center x value: {}", e);
                            return;
                        }
                    };
                    let center_y = match receiver5.recv() {
                        Ok(y) => y,
                        Err(e) => {
                            println!("Failed getting joystick center y value: {}", e);
                            return;
                        }
                    };
                    let range = match receiver5.recv() {
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
                    match cmd_sender6.send(Command::SaveCalibration) {
                        Ok(()) => println!("Saving calibration..."),
                        Err(e) => {
                            println!("Failed issuing 'save calibration' command: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.center.x) {
                        Ok(()) => println!("Sent center x coordinate."),
                        Err(e) => {
                            println!("Failed to send center x coordinate: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.center.y) {
                        Ok(()) => println!("Sent center y coordinate."),
                        Err(e) => {
                            println!("Failed to send center y coordinate: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.range) {
                        Ok(()) => println!("Sent range value."),
                        Err(e) => {
                            println!("Failed to send range value: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.xy_angle_ticks.into()) {
                        Ok(()) => println!("Sent XY angle ticks."),
                        Err(e) => {
                            println!("Failed to send XY angle ticks: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.xz_angle_ticks.into()) {
                        Ok(()) => println!("Sent XZ angle ticks."),
                        Err(e) => {
                            println!("Failed to send XZ angle ticks: {}", e);
                            return;
                        }
                    }
                    match data_sender5.send(data.custom_bounds.yz_angle_ticks.into()) {
                        Ok(()) => println!("Sent YZ angle ticks."),
                        Err(e) => {
                            println!("Failed to send YZ angle ticks: {}", e);
                            return;
                        }
                    }

                    if let Err(e) = receiver6.recv() {
                        println!("Failed to save calibration: {}", e);
                    };
                },
            ),
            1.0,
        )
        .with_flex_child(
            Button::new("Store Profiles").on_click(move |_event, _data, _env| {
                match cmd_sender7.send(Command::StoreProfiles) {
                    Ok(()) => println!("Storing profiles..."),
                    Err(e) => {
                        println!("Failed issuing 'store profiles' command: {}", e);
                        return;
                    }
                }
                if let Err(e) = receiver7.recv() {
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

    loop {
        let cmd = match command_receiver.try_recv() {
            Ok(x) => x,
            Err(_) => Command::FetchJoystickCoords,
        };

        hs.write_all(&[cmd as u8])?;
        wait_for_ack(&mut hs)?;
        match cmd {
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
