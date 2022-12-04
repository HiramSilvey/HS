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
    theme, AppLauncher, Color, Data, ExtEventSink, Lens, PlatformError, Point, RenderContext,
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
const ZERO_STATE: Selector<()> = Selector::new("hs.zero-state");

#[derive(Clone, Copy)]
enum Command {
    FetchJoystickCoords,
    CalibrateJoystick,
    SaveCalibration,
    StoreProfiles,
    IncNeutralX,
    DecNeutralX,
    IncNeutralY,
    DecNeutralY,
    IncRange,
    DecRange,
    IncXYAngle,
    DecXYAngle,
    IncXZAngle,
    DecXZAngle,
    IncYZAngle,
    DecYZAngle,
}

#[derive(Clone, Copy)]
enum Action {
    Inc,
    Dec,
}

#[derive(Clone, Copy)]
enum Field {
    NeutralX,
    NeutralY,
    Range,
    XYAngle,
    XZAngle,
    YZAngle,
}

#[derive(Clone, Data, Lens)]
struct JoystickState {
    cursor: Point,
    neutral_x: i32,
    neutral_y: i32,
    range: i32,
    xy_angle: i32,
    xz_angle: i32,
    yz_angle: i32,
}

impl JoystickState {
    pub fn new() -> JoystickState {
        JoystickState {
            cursor: Point::new(0.0, 0.0),
            neutral_x: 0,
            neutral_y: 0,
            range: 0,
            xy_angle: 0,
            xz_angle: 0,
            yz_angle: 0,
        }
    }

    fn zero_state(&mut self) {
        self.neutral_x = 0;
        self.neutral_y = 0;
        self.range = 0;
        self.xy_angle = 0;
        self.xz_angle = 0;
        self.yz_angle = 0;
    }

    fn set_cursor(&mut self, point: &Point) {
        self.cursor = *point;
    }
    fn inc_neutral_x(&mut self) {
        self.neutral_x += 1;
    }
    fn dec_neutral_x(&mut self) {
        self.neutral_x -= 1;
    }
    fn inc_neutral_y(&mut self) {
        self.neutral_y += 1;
    }
    fn dec_neutral_y(&mut self) {
        self.neutral_y -= 1;
    }
    fn inc_range(&mut self) {
        self.range += 1;
    }
    fn dec_range(&mut self) {
        self.range -= 1;
    }
    fn inc_xy_angle(&mut self) {
        self.xy_angle += 1;
    }
    fn dec_xy_angle(&mut self) {
        self.xy_angle -= 1;
    }
    fn inc_xz_angle(&mut self) {
        self.xz_angle += 1;
    }
    fn dec_xz_angle(&mut self) {
        self.xz_angle -= 1;
    }
    fn inc_yz_angle(&mut self) {
        self.yz_angle += 1;
    }
    fn dec_yz_angle(&mut self) {
        self.yz_angle -= 1;
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

fn edit_button(
    action: Action,
    field: Field,
    cmd_sender: Sender<Command>,
) -> impl Widget<JoystickState> {
    let symbol = match action {
        Action::Inc => "+",
        Action::Dec => "-",
    };
    Label::new(format!("{}", symbol))
        .with_text_size(14.0)
        .center()
        .background(get_button_painter())
        .expand()
        .on_click(move |_ctx, data: &mut JoystickState, _env| match field {
            Field::NeutralX => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncNeutralX) {
                        println!("Failed issuing 'increment neutral x' command: {}", e);
                        return;
                    }
                    data.inc_neutral_x();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecNeutralX) {
                        println!("Failed issuing 'decrement neutral x' command: {}", e);
                        return;
                    }
                    data.dec_neutral_x();
                }
            },
            Field::NeutralY => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncNeutralY) {
                        println!("Failed issuing 'increment neutral y' command: {}", e);
                        return;
                    }
                    data.inc_neutral_y();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecNeutralY) {
                        println!("Failed issuing 'decrement neutral y' command: {}", e);
                        return;
                    }
                    data.dec_neutral_y();
                }
            },
            Field::Range => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncRange) {
                        println!("Failed issuing 'increment range' command: {}", e);
                        return;
                    }
                    data.inc_range();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecRange) {
                        println!("Failed issuing 'decrement range' command: {}", e);
                        return;
                    }
                    data.dec_range();
                }
            },
            Field::XYAngle => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncXYAngle) {
                        println!("Failed issuing 'increment xy angle' command: {}", e);
                        return;
                    }
                    data.inc_xy_angle();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecXYAngle) {
                        println!("Failed issuing 'decrement xy angle' command: {}", e);
                        return;
                    }
                    data.dec_xy_angle();
                }
            },
            Field::XZAngle => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncXZAngle) {
                        println!("Failed issuing 'increment xz angle' command: {}", e);
                        return;
                    }
                    data.inc_xz_angle();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecXZAngle) {
                        println!("Failed issuing 'decrement xz angle' command: {}", e);
                        return;
                    }
                    data.dec_xz_angle();
                }
            },
            Field::YZAngle => match action {
                Action::Inc => {
                    if let Err(e) = cmd_sender.send(Command::IncYZAngle) {
                        println!("Failed issuing 'increment yz angle' command: {}", e);
                        return;
                    }
                    data.inc_yz_angle();
                }
                Action::Dec => {
                    if let Err(e) = cmd_sender.send(Command::DecYZAngle) {
                        println!("Failed issuing 'decrement yz angle' command: {}", e);
                        return;
                    }
                    data.dec_yz_angle();
                }
            },
        })
}

fn map(x: f64, in_min: f64, in_max: f64, out_min: f64, out_max: f64) -> f64 {
    (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
}

impl Widget<JoystickState> for JoystickState {
    fn event(&mut self, _ctx: &mut EventCtx, event: &Event, data: &mut JoystickState, _env: &Env) {
        match event {
            Event::Command(cmd) => {
                if cmd.is(SET_CURSOR) {
                    let point = cmd.get_unchecked(SET_CURSOR).clone();
                    data.set_cursor(&point);
                } else if cmd.is(ZERO_STATE) {
                    data.zero_state();
                }
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
        let cursor_x = map(data.cursor.x, 0., 1023., rect.min_x(), rect.max_x());
        let cursor_y = map(data.cursor.y, 0., 1023., rect.min_y(), rect.max_y());
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

fn wait_for_ok(hs: &mut Box<dyn SerialPort>) -> Result<()> {
    let mut buf = vec![0u8; 1];
    wait_for_data(hs, &mut buf)?;
    if buf[0] != 0 {
        return Err(anyhow!("Failed to receive ACK. Wanted 0, got {}.", buf[0]));
    }
    Ok(())
}

fn bytes_to_i32(bytes: &Vec<u8>) -> Result<i32> {
    if bytes.len() != 4 {
        return Err(anyhow!(
            "Failed converting bytes to i32. Expected 4 bytes, found {} bytes.",
            bytes.len()
        ));
    }
    Ok(((bytes[0] as i32) << 24)
        | ((bytes[1] as i32) << 16)
        | ((bytes[2] as i32) << 8)
        | bytes[3] as i32)
}

fn fetch_joystick_coords(hs: &mut Box<dyn SerialPort>, event_sink: &ExtEventSink) -> Result<()> {
    let mut x = vec![0u8; 4];
    let mut y = vec![0u8; 4];
    wait_for_data(hs, &mut x)?;
    wait_for_data(hs, &mut y)?;

    event_sink.submit_command(
        SET_CURSOR,
        Point::new(bytes_to_i32(&x)?.into(), bytes_to_i32(&y)?.into()),
        Target::Auto,
    )?;

    Ok(())
}

fn calibrate_joystick(hs: &mut Box<dyn SerialPort>, event_sink: &ExtEventSink) -> Result<()> {
    wait_for_ok(hs)?;
    event_sink.submit_command(ZERO_STATE, (), Target::Auto)?;

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
    wait_for_ok(hs)?;

    sender.send(0.0)?;
    Ok(())
}

fn build_ui(cmd_sender: Sender<Command>, receiver: Receiver<f64>) -> impl Widget<JoystickState> {
    let (
        cmd_sender2,
        cmd_sender3,
        cmd_sender4,
        cmd_sender5,
        cmd_sender6,
        cmd_sender7,
        cmd_sender8,
        cmd_sender9,
        cmd_sender10,
        cmd_sender11,
        cmd_sender12,
        cmd_sender13,
        cmd_sender14,
        cmd_sender15,
    ) = (
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
        cmd_sender.clone(),
    );
    Flex::column()
        .with_child(JoystickState::new())
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("X").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::NeutralX, cmd_sender), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::NeutralX), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::NeutralX, cmd_sender2), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("Y").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::NeutralY, cmd_sender3), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::NeutralY), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::NeutralY, cmd_sender4), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("Range").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::Range, cmd_sender5), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::Range), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::Range, cmd_sender6), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("XY Angle").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::XYAngle, cmd_sender7), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::XYAngle), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::XYAngle, cmd_sender8), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("XZ Angle").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::XZAngle, cmd_sender9), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::XZAngle), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::XZAngle, cmd_sender10), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Flex::row()
                .with_flex_child(Label::new("YZ Angle").with_text_size(14.0).center(), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Dec, Field::YZAngle, cmd_sender11), 1.0)
                // .with_spacer(1.0)
                // .with_flex_child(state_label(Field::YZAngle), 1.0)
                .with_spacer(1.0)
                .with_flex_child(edit_button(Action::Inc, Field::YZAngle, cmd_sender12), 1.0),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Button::new("Calibrate Joystick").on_click(
                move |_event, _data, _env| match cmd_sender13.send(Command::CalibrateJoystick) {
                    Ok(()) => println!("Calibration starting..."),
                    Err(e) => {
                        println!("Failed issuing 'calibrate joystick' command: {}", e);
                        return;
                    }
                },
            ),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Button::new("Save Calibration").on_click(move |_event, _data, _env| match cmd_sender14
                .send(Command::SaveCalibration)
            {
                Ok(()) => println!("Saving calibration..."),
                Err(e) => {
                    println!("Failed issuing 'save calibration' command: {}", e);
                    return;
                }
            }),
            1.0,
        )
        .with_spacer(1.0)
        .with_flex_child(
            Button::new("Store Profiles").on_click(move |_event, _data, _env| {
                match cmd_sender15.send(Command::StoreProfiles) {
                    Ok(()) => println!("Storing profiles..."),
                    Err(e) => {
                        println!("Failed issuing 'store profiles' command: {}", e);
                        return;
                    }
                }
                if let Err(e) = receiver.recv() {
                    println!("Failed to store profiles: {}", e);
                };
            }),
            1.0,
        )
}

fn drive(
    sender: Sender<f64>,
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
        wait_for_ok(&mut hs)?;
        match cmd {
            Command::FetchJoystickCoords => fetch_joystick_coords(&mut hs, &event_sink)?,
            Command::CalibrateJoystick => calibrate_joystick(&mut hs, &event_sink)?,
            Command::StoreProfiles => store_profiles(&mut hs, &sender)?,
            _ => wait_for_ok(&mut hs)?,
        };
    }
}

pub fn main() -> Result<(), PlatformError> {
    let (cmd_sender, cmd_receiver) = unbounded();
    let (child_data_sender, parent_data_receiver) = unbounded();

    let launcher = AppLauncher::with_window(
        WindowDesc::new(build_ui(cmd_sender, parent_data_receiver))
            .title("HS Configurator")
            .window_size((JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH + 150.0))
            .with_min_size((JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH)),
    );

    let event_sink = launcher.get_external_handle();
    thread::spawn(move || -> Result<()> { drive(child_data_sender, cmd_receiver, event_sink) });

    launcher.launch(JoystickState::new())?;

    Ok(())
}
