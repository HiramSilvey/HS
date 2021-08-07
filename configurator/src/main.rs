// Copyright 2021 Hiram Silvey

use anyhow::{anyhow, Result};
use configurator::encoder;
use configurator::profiles;
use druid::kurbo::{Circle, Line};
use druid::piet::StrokeStyle;
use druid::widget::prelude::*;
use druid::widget::{Button, Flex, Label, Painter};
use druid::{
    theme, AppLauncher, Color, Data, Lens, PlatformError, Point, Rect, RenderContext, Widget,
    WidgetExt, WindowDesc,
};
use serialport::SerialPort;
use std::path::Path;
use std::thread::sleep;
use std::time::{Duration, Instant};

const MAX_EEPROM_BYTES: usize = 1064;
const JOYSTICK_CURSOR_RADIUS: f64 = 3.0;
const JOYSTICK_BOX_LENGTH: f64 = 400.;

#[derive(Clone, Copy, Data, Lens)]
struct Bounds {
    center: Point,
    range: f64,
}

impl Bounds {
    pub fn new() -> Bounds {
        Bounds {
            center: Point::new(0., 0.),
            range: 0.,
        }
    }

    fn get_x_min(&self) -> f64 {
        self.center.x - self.range
    }

    fn get_x_max(&self) -> f64 {
        self.center.x + self.range
    }

    fn get_y_min(&self) -> f64 {
        self.center.y - self.range
    }

    fn get_y_max(&self) -> f64 {
        self.center.y + self.range
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
}

#[derive(Clone, Data, Lens)]
struct JoystickState {
    cursor: Point,
    default_bounds: Bounds,
    custom_bounds: Bounds,
    tick: f64,
}

impl JoystickState {
    pub fn new() -> JoystickState {
        JoystickState {
            cursor: Point::new(0., 0.),
            default_bounds: Bounds::new(),
            custom_bounds: Bounds::new(),
            tick: 0.,
        }
    }

    fn set_cursor(&mut self, x: f64, y: f64) {
        self.cursor.x = x;
        self.cursor.y = y;
    }

    fn set_bounds(&mut self, x: f64, y: f64, range: f64) {
        self.default_bounds.center.x = x;
        self.default_bounds.center.y = y;
        self.default_bounds.range = range;
        self.custom_bounds = self.default_bounds;
        self.tick = range / 1000.;
    }

    fn shift_bounds(&mut self, direction: Direction) {
        match direction {
            Direction::Up => self.custom_bounds.center.y -= self.tick,
            Direction::Down => self.custom_bounds.center.y += self.tick,
            Direction::Left => self.custom_bounds.center.x -= self.tick,
            Direction::Right => self.custom_bounds.center.x += self.tick,
            Direction::In => self.custom_bounds.range -= self.tick,
            Direction::Out => self.custom_bounds.range += self.tick,
        }
    }
}

fn symbol_button(direction: Direction) -> impl Widget<JoystickState> {
    let painter = Painter::new(|ctx, _, env| {
        let bounds = ctx.size().to_rect();

        ctx.fill(bounds, &env.get(theme::PRIMARY_DARK));

        if ctx.is_hot() {
            ctx.stroke(bounds.inset(-0.5), &Color::WHITE, 1.0);
        }

        if ctx.is_active() {
            ctx.fill(bounds, &env.get(theme::PRIMARY_LIGHT));
        }
    });

    let symbol = match direction {
        Direction::Up => 'ᐱ',
        Direction::Down => 'ᐯ',
        Direction::Left => 'ᐸ',
        Direction::Right => 'ᐳ',
        Direction::In => '-',
        Direction::Out => '+',
    };

    Label::new(format!("{}", symbol))
        .with_text_size(24.)
        .center()
        .background(painter)
        .expand()
        .on_click(move |_ctx, data: &mut JoystickState, _env| data.shift_bounds(direction))
}

fn map(x: f64, in_min: f64, in_max: f64, out_min: f64, out_max: f64) -> f64 {
    (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
}

impl Widget<JoystickState> for JoystickState {
    fn event(
        &mut self,
        _ctx: &mut EventCtx,
        _event: &Event,
        _data: &mut JoystickState,
        _env: &Env,
    ) {
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

        let default_range = (size.width * 0.875) / 2.;
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

        // Mutable bounds & guidelines.
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
        let mutable_x_center = mutable_x_min + (mutable_x_max - mutable_x_min) / 2.;
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
        let mutable_y_center = mutable_y_min + (mutable_y_max - mutable_y_min) / 2.;

        let mutable_top_left = Point::new(mutable_x_min, mutable_y_min);
        let mutable_top_right = Point::new(mutable_x_max, mutable_y_min);
        let mutable_bottom_left = Point::new(mutable_x_min, mutable_y_max);
        let mutable_bottom_right = Point::new(mutable_x_max, mutable_y_max);
        let mutable_center = Point::new(mutable_x_center, mutable_y_center);
        let mutable_center_left = Point::new(mutable_x_min, mutable_y_center);
        let mutable_center_right = Point::new(mutable_x_max, mutable_y_center);
        let mutable_center_top = Point::new(mutable_x_center, mutable_y_min);
        let mutable_center_bottom = Point::new(mutable_x_center, mutable_y_max);

        let mutable_bounds = Rect::from_points(mutable_top_left, mutable_bottom_right);
        ctx.stroke(mutable_bounds, &env.get(theme::PRIMARY_LIGHT), 1.0);

        let stroke_style = StrokeStyle::new().dash_pattern(&[1., 4.]);
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

fn wait_for_ack(hs: &mut Box<dyn SerialPort>) -> Result<()> {
    println!("Waiting for ACK.");
    let now = Instant::now();
    let timeout = Duration::new(70, 0);
    let mut buf = vec![0u8; 1];
    let mut ack = hs.read_exact(&mut buf);
    while ack.is_err() && now.elapsed() < timeout {
        println!("Waiting...");
        sleep(Duration::new(5, 0));
        ack = hs.read_exact(&mut buf);
    }
    if ack.is_err() {
        return Err(anyhow!(
            "Failure waiting for OK from HS: {}",
            ack.unwrap_err()
        ));
    }
    println!("ACK received.");
    Ok(())
}

fn calibrate_joystick() -> Result<()> {
    let mut hs = connect()?;
    hs.write_all(&[2])?;
    wait_for_ack(&mut hs)?;
    wait_for_ack(&mut hs)?;
    Ok(())
}

fn store_profiles() -> Result<()> {
    let profiles = match profiles::load_all(&Path::new("../profiles")) {
        Ok(x) => x,
        Err(e) => return Err(anyhow!("Unable to load profiles: {}", e)),
    };
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
    let mut hs = connect()?;
    hs.write_all(&[1])?;
    wait_for_ack(&mut hs)?;
    let size = encoded.len();
    hs.write_all(&[(size >> 8) as u8, (size & 255) as u8])?;
    hs.write_all(&encoded)?;
    wait_for_ack(&mut hs)?;
    Ok(())
}

fn build_ui() -> impl Widget<JoystickState> {
    Flex::column()
        .with_child(JoystickState::new())
        .with_flex_child(
            Flex::row()
                .with_flex_child(symbol_button(Direction::Left), 1.0)
                .with_spacer(1.0)
                .with_flex_child(
                    Flex::column()
                        .with_flex_child(symbol_button(Direction::Up), 1.0)
                        .with_spacer(1.0)
                        .with_flex_child(symbol_button(Direction::Down), 1.0),
                    1.0,
                )
                .with_spacer(1.0)
                .with_flex_child(symbol_button(Direction::Right), 1.0)
                .with_spacer(1.0)
                .with_flex_child(symbol_button(Direction::In), 1.0)
                .with_spacer(1.0)
                .with_flex_child(symbol_button(Direction::Out), 1.0),
            1.0,
        )
        .with_flex_child(
            Button::new("Calibrate Joystick").on_click(|_event, _data, _env| {
                match calibrate_joystick() {
                    Ok(()) => println!("Calibration complete!"),
                    Err(e) => println!("Calibration failed: {}", e),
                }
            }),
            1.0,
        )
        .with_flex_child(
            Button::new("Store Profiles").on_click(|_event, _data, _env| match store_profiles() {
                Ok(()) => println!("All profiles stored successfully!"),
                Err(e) => println!("Storing profiles failed: {}", e),
            }),
            1.0,
        )
}

pub fn main() -> Result<(), PlatformError> {
    let mut state = JoystickState::new();

    AppLauncher::with_window(
        WindowDesc::new(build_ui())
            .title("HS Configurator")
            .window_size((600., 600.))
            .with_min_size((JOYSTICK_BOX_LENGTH, JOYSTICK_BOX_LENGTH)),
    )
    .launch(state)?;
    Ok(())
}
