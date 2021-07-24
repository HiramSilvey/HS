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
const JOYSTICK_POINT_RADIUS: f64 = 5.0;
const JOYSTICK_DISPLAY_RADIUS: f64 = 125.0;

#[derive(Clone, Data, Lens)]
struct JoystickCoords {
    x: i32,
    y: i32,
}

struct JoystickDisplay;

impl JoystickDisplay {
    fn new() -> JoystickDisplay {
        JoystickDisplay {}
    }
}

impl Widget<JoystickCoords> for JoystickDisplay {
    fn event(
        &mut self,
        _ctx: &mut EventCtx,
        _event: &Event,
        _data: &mut JoystickCoords,
        _env: &Env,
    ) {
    }

    fn lifecycle(
        &mut self,
        _ctx: &mut LifeCycleCtx,
        _event: &LifeCycle,
        _data: &JoystickCoords,
        _env: &Env,
    ) {
    }

    fn update(
        &mut self,
        _ctx: &mut UpdateCtx,
        _old_data: &JoystickCoords,
        _data: &JoystickCoords,
        _env: &Env,
    ) {
    }

    fn layout(
        &mut self,
        _layout_ctx: &mut LayoutCtx,
        bc: &BoxConstraints,
        _data: &JoystickCoords,
        _env: &Env,
    ) -> Size {
        if bc.is_width_bounded() | bc.is_height_bounded() {
            let size = Size::new(250.0, 250.0);
            bc.constrain(size)
        } else {
            bc.max()
        }
    }

    fn paint(&mut self, ctx: &mut PaintCtx, data: &JoystickCoords, env: &Env) {
        let size = ctx.size();
        let rect = size.to_rect();
        ctx.fill(rect, &Color::WHITE);

        ctx.paint_with_z_index(1, move |ctx| {
            let cursor = Circle::new(rect.center(), JOYSTICK_POINT_RADIUS);
            let color = Color::rgb8(255, 0, 0);
            ctx.fill(cursor, &color);
        });

        let bounding_ring = Circle::new(rect.center(), JOYSTICK_DISPLAY_RADIUS);
        let color = Color::rgb8(0, 0, 128);
        ctx.fill(bounding_ring, &color);
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

fn build_ui() -> impl Widget<JoystickCoords> {
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
    .launch(JoystickCoords { x: 5, y: 5 })?;
    Ok(())
}
