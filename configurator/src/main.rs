// Copyright 2021 Hiram Silvey

use anyhow::{anyhow, Result};
use configurator::encoder;
use configurator::profiles;
use druid::kurbo::{Circle, Line};
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
            Direction::Up => self.custom_bounds.center.y += self.tick,
            Direction::Down => self.custom_bounds.center.y -= self.tick,
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
            let size = Size::new(250.0, 250.0);
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

        // Mutable bounds.
        let mut bounds_size =
            Size::new(data.custom_bounds.range * 2., data.custom_bounds.range * 2.);
        let bounds = Rect::from_center_size(data.custom_bounds.center, bounds_size);
        ctx.stroke(bounds, &env.get(theme::PRIMARY_LIGHT), 1.0);

        // Mutable bound guidelines.
        let min_x = bounds.min_x();
        let max_x = bounds.max_x();
        let min_y = bounds.min_y();
        let max_y = bounds.max_y();
        let center_x = bounds.center().x;
        let center_y = bounds.center().y;

        ctx.stroke(
            Line::new(Point::new(min_x, min_y), Point::new(max_x, max_y)),
            &env.get(theme::BACKGROUND_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(Point::new(min_x, max_y), Point::new(max_x, min_y)),
            &env.get(theme::BACKGROUND_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(Point::new(min_x, center_y), Point::new(max_x, center_y)),
            &env.get(theme::BACKGROUND_LIGHT),
            1.0,
        );
        ctx.stroke(
            Line::new(Point::new(center_x, min_y), Point::new(center_x, max_y)),
            &env.get(theme::BACKGROUND_LIGHT),
            1.0,
        );

        // Cursor.
        let cursor_color = env.get(theme::CURSOR_COLOR);
        ctx.paint_with_z_index(1, move |ctx| {
            let cursor = Circle::new(rect.center(), JOYSTICK_CURSOR_RADIUS);
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
    state.set_cursor(50., 50.);
    state.set_bounds(100., 100., 100.);

    AppLauncher::with_window(
        WindowDesc::new(build_ui())
            .title("HS Configurator")
            .window_size((600.0, 600.0)),
    )
    .launch(state)?;
    Ok(())
}
