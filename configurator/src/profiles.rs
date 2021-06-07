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

pub struct Profiles {
    dir: Path,
    data: Vec<Profile>,
}

impl<'a> Profiles<'a> {
    fn load_all(&self, path: &Path) -> Result<()> {
        for entry in fs::read_dir(path)? {
            let entry = entry?;
            let p = entry.path();
            if p.is_dir() {
                self.load_from(&p)?;
            } else {
                let buf = fs::read(p)?;
                let profile = Profile::decode(&mut Cursor::new(buf))?;
                self.data.push(profile);
            }
        }
        Ok(())
    }

    fn load(&self) -> Result<()> {
        self.load_from(&self.dir)
    }

    pub fn load_all(dir: &str) -> Result<Profiles> {
        let profiles = Profiles {
            dir: Path::new(dir),
            data: Vec::new(),
        };
        profiles.load()?;
        Ok(profiles)
    }

    pub fn save_all(&self) -> Result<()> {
        for profile in &self.data {
            let mut buf = Vec::new();
            buf.reserve(profile.encoded_len());
            profile.encode(&mut buf)?;
            let path = self.path.join(Path::new(&profile.name));
            fs::write(path, buf)?;
        }
        Ok(())
    }

    pub fn new(&mut self, name: String) -> Option<&mut Profile> {
        let mut profile = Profile::default();
        profile.name = name;
        self.profiles.push(profile);
        let i = self.profiles.len() - 1;
        Some(&mut self.profiles[i])
    }

    pub fn get_profile(&mut self, name: &str) -> Option<&mut Profile> {
        let i = self.find_profile(name)?;
        Some(&mut self.profiles[i])
    }

    fn find_profile(&self, name: &str) -> Option<usize> {
        for i in 0..self.profiles.len() {
            if self.profiles[i].name == name {
                return Some(i);
            }
        }
        None
    }

    pub fn upload(&self) -> Result<()> {
        let encoded = encoder::encode(&self.profiles)?;
        if encoded.len() > MAX_EEPROM_BYTES {
            return Err(anyhow!(
                "Encoded length of {} bytes exceeds controller maximum of {} bytes.",
                encoded.len(),
                MAX_EEPROM_BYTES,
            ));
        }
        let mut controller = serialport::new(self.port, 9600)
            .timeout(Duration::from_millis(10))
            .open()?;
        controller.write_all(&encoded)?;
        Ok(())
    }
}

impl<'a, 'b> fmt::Display for Profiles<'a, 'b> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "port: {}\npath: {}", self.port, self.path.display())?;
        for profile in &self.profiles {
            write!(f, "\n{}", profile)?;
        }
        Ok(())
    }
}
