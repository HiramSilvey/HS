use anyhow::{anyhow, Result};
use profiles::Profile;
use prost::Message;
use std::fmt;
use std::fs;
use std::io::Cursor;
use std::path::Path;
use std::time::Duration;
use std::vec::Vec;

mod encoder;

const MAX_EEPROM_BYTES: usize = 1064;

pub mod profiles {
    include!(concat!(env!("OUT_DIR"), "/configurator.profiles.rs"));
}

pub struct Configurator<'a, 'b> {
    port: &'a str,
    path: &'b Path,
    profiles: Vec<Profile>,
}

impl<'a, 'b> Configurator<'a, 'b> {
    pub fn new(port: &'a str, profiles_dir: &'b str) -> Result<Configurator<'a, 'b>> {
        let path = Path::new(profiles_dir);
        let mut c = Configurator {
            port: port,
            path: path,
            profiles: Vec::new(),
        };
        c.load_profiles(None)?;
        Ok(c)
    }

    pub fn create_profile(&mut self, name: String) -> Option<&mut Profile> {
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

    pub fn save_profiles(&self) -> Result<()> {
        for profile in &self.profiles {
            let mut buf = Vec::new();
            buf.reserve(profile.encoded_len());
            profile.encode(&mut buf)?;
            let path = self.path.join(Path::new(&profile.name));
            fs::write(path, buf)?;
        }
        Ok(())
    }

    fn find_profile(&self, name: &str) -> Option<usize> {
        for i in 0..self.profiles.len() {
            if self.profiles[i].name == name {
                return Some(i);
            }
        }
        None
    }

    fn load_profiles(&mut self, path: Option<&Path>) -> Result<()> {
        let path = path.unwrap_or(self.path);
        for entry in fs::read_dir(path)? {
            let entry = entry?;
            let p = entry.path();
            if p.is_dir() {
                self.load_profiles(Some(&p))?;
            } else {
                let buf = fs::read(p)?;
                let profile = Profile::decode(&mut Cursor::new(buf))?;
                self.profiles.push(profile);
            }
        }
        Ok(())
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

impl<'a, 'b> fmt::Display for Configurator<'a, 'b> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "port: {}\npath: {}", self.port, self.path.display())?;
        for profile in &self.profiles {
            write!(f, "\n{}", profile)?;
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
