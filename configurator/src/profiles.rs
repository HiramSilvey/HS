use anyhow::{anyhow, Result};
use profile::Profile;
use prost::Message;
use std::fmt;
use std::fs;
use std::io::Cursor;
use std::path::Path;
use std::time::Duration;
use std::vec::Vec;

use crate::encoder;

const MAX_EEPROM_BYTES: usize = 1064;

pub fn load_all(path: &Path, profiles: &Vec<Profile>) -> Result<()> {
    for entry in fs::read_dir(path)? {
        let entry = entry?;
        let p = entry.path();
        if p.is_dir() {
            load_all(&p, profiles)?;
        } else {
            let buf = fs::read(p)?;
            let profile = Profile::decode(&mut Cursor::new(buf))?;
            profiles.push(profile);
        }
    }
    Ok(())
}

pub fn save_all(profiles: &Vec<Profile>, path: &Path) -> Result<()> {
    for profile in profiles {
        let mut buf = Vec::new();
        buf.reserve(profile.encoded_len());
        profile.encode(&mut buf)?;
        let path = path.join(Path::new(&profile.name));
        fs::write(path, buf)?;
    }
    Ok(())
}

pub fn find(profiles: &Vec<Profile>, name: &str) -> Option<usize> {
    for i in 0..profiles.len() {
        if self.profiles[i].name == name {
            return Some(i);
        }
    }
    None
}

pub fn upload(profiles: &Vec<Profile>, port: &str) -> Result<()> {
    let encoded = encoder::encode(profiles)?;
    if encoded.len() > MAX_EEPROM_BYTES {
        return Err(anyhow!(
            "Encoded length of {} bytes exceeds controller maximum of {} bytes.",
            encoded.len(),
            MAX_EEPROM_BYTES,
        ));
    }
    let mut controller = serialport::new(port, 9600)
        .timeout(Duration::from_millis(10))
        .open()?;
    controller.write_all(&encoded)?;
    Ok(())
}