// Copyright 2021 Hiram Silvey

use anyhow::{anyhow, Result};
use configurator::encoder;
use configurator::profiles;
use druid::kurbo::Circle;
use druid::widget::prelude::*;
use druid::widget::{Button, Flex};
use druid::{AppLauncher, Color, Lens, PlatformError, WindowDesc};
use serialport::SerialPort;
use std::path::Path;
use std::thread::sleep;
use std::time::{Duration, Instant};

const MAX_EEPROM_BYTES: usize = 1064;
const JOYSTICK_CURSOR_RADIUS: f64 = 5.0;

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
    pointer: Point,
    default_bounds: Bounds,
    custom_bounds: Bounds,
    tick: f64,
}

impl JoystickState {
    pub fn new() -> JoystickState {
        JoystickState {
            pointer: Point::new(0., 0.),
            default_bounds: Bounds::new(),
            custom_bounds: Bounds::new(),
            tick: 0.,
        }
    }

    fn set_pointer(&mut self, x: f64, y: f64) {
        self.pointer.x = x;
        self.pointer.y = y;
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

fn joystick() -> impl Widget<JoystickState> {
    let painter = Painter::new(|ctx, _, env| {
        let size = ctx.size();
        let rect = size.to_rect();
        ctx.fill(rect, &env.get(theme::BACKGROUND_DARK));

        ctx.paint_with_z_index(1, move |ctx| {
            let cursor = Circle::new(rect.center(), JOYSTICK_CURSOR_RADIUS);
            ctx.fill(cursor, &env.get(theme::PRIMARY_LIGHT));
        });

        let mut bounds_size = size;
        bounds_size.width -= size.width * .125;
        bounds_size.height -= size.height * .125;
        let bounds = Square::from_center_size(rect.center(), bounds_size);
        ctx.fill(bounding_ring, &env.get(theme::PRIMARY_LIGHT));
    });
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
        .with_child(JoystickDisplay::new())
}

pub fn main() -> Result<(), PlatformError> {
    AppLauncher::with_window(
        WindowDesc::new(build_ui())
            .title("HS Configurator")
            .window_size((600.0, 600.0)),
    )
    .launch(JoystickState { x: 5, y: 5 })?;
    Ok(())
}
