// Copyright 2021 Hiram Silvey
use anyhow::{anyhow, Result};
use configurator::encoder;
use configurator::profiles;
use serialport::SerialPort;
use std::path::Path;
use std::time::{Duration, Instant};

const MAX_EEPROM_BYTES: usize = 1064;

fn connect() -> Result<Box<dyn SerialPort>> {
    let port = "/dev/ttyACM0";
    match serialport::new(port, 9600)
        .timeout(Duration::from_millis(10))
        .open()
    {
        Ok(x) => Ok(x),
        Err(e) => Err(anyhow!(
            "Unable to connect to HS via serial port {}: {:?}",
            port,
            e
        )),
    }
}

fn wait_for_ack(hs: &mut Box<dyn SerialPort>) -> Result<()> {
    let now = Instant::now();
    let timeout = Duration::new(70, 0);

    let mut buf = vec![0u8; 1];
    let mut status = hs.read_exact(&mut buf);
    while status.is_err() && now.elapsed() < timeout {
        status = hs.read_exact(&mut buf);
    }
    if status.is_err() {
        return Err(anyhow!(
            "Failed waiting for data from HS: {}",
            status.unwrap_err()
        ));
    }
    if buf[0] != 0 {
        return Err(anyhow!("Failed to receive ACK. Wanted 0, got {}.", buf[0]));
    }
    Ok(())
}

pub fn main() {
    let mut hs = connect().unwrap();

    let profiles = match profiles::load_all(&Path::new("../profiles")) {
        Ok(x) => x,
        Err(e) => panic!("Unable to load profiles: {:?}", e),
    };
    let encoded = match encoder::encode(&profiles) {
        Ok(x) => x,
        Err(e) => panic!("Unable to encode profiles: {:?}", e),
    };
    if encoded.len() > MAX_EEPROM_BYTES {
        panic!(
            "Encoded length of {} bytes exceeds HS maximum of {} bytes.",
            encoded.len(),
            MAX_EEPROM_BYTES,
        );
    }

    let size = encoded.len();
    hs.write_all(&[(size >> 8) as u8, (size & 0xFF) as u8])
        .unwrap();
    hs.write_all(&encoded).unwrap();
    wait_for_ack(&mut hs).unwrap();

    println!("Success! Wrote {} profiles.", profiles.len());
}
