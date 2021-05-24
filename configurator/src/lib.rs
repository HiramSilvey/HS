use anyhow::Result;
use profiles::profile::Platform;
use profiles::profile::Platform::{Pc, Unknown};
use profiles::Profile;
use prost::Message;
use std::fmt;
use std::fs;
use std::io::Cursor;
use std::path::Path;
use std::time::Duration;
use std::vec::Vec;

const PLATFORMS: [Platform; 1] = [Pc];

pub mod profiles {
    include!(concat!(env!("OUT_DIR"), "/configurator.profiles.rs"));
}

#[derive(Debug, Eq, Ord, PartialEq, PartialOrd)]
struct PlatformProperties {
    priority: u8,
    position: u8,
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

    fn compress(&self) -> Option<Vec<u8>> {
        // list of profiles
        // byte 0: 8 bit console map
        // byte 1, up to 4: each nibble represents position per console according to console bitmap
        // next byte: length of profile data (to find next profile)
        let mut compressed: Vec<u8> = Vec::new();
        for profile in &self.profiles {
            let mut properties: Vec<PlatformProperties> = Vec::new();
            for config in &profile.platform_config {
                if config.platform == Unknown as i32 {
                    return None;
                }
                if config.position < 0 || config.position > 255 {
                    return None;
                }
                let mut priority: u8 = 0;
                for i in 0..PLATFORMS.len() {
                    if config.platform == PLATFORMS[i] as i32 {
                        priority = i as u8;
                        break;
                    }
                }
                properties.push(PlatformProperties {
                    priority: priority,
                    position: config.position as u8,
                });
            }
            properties.sort();
            let mut platform_bitmap: u8 = 0;
            let mut positions: Vec<u8> = Vec::new();
            let mut curr_positions: u8 = 0;
            for i in 0..properties.len() {
                let property = &properties[i];
                platform_bitmap |= 1 << property.priority + 7;
                if i % 2 == 0 {
                    curr_positions |= property.position << 4;
                } else {
                    curr_positions |= property.position;
                    positions.push(curr_positions);
                }
            }
            if curr_positions > 0 {
                positions.push(curr_positions);
            }
            compressed.push(platform_bitmap);
            compressed.append(&mut positions);
            // TODO(hiram): Compress data, get length of data
        }
        Some(compressed)
    }

    pub fn upload(&self) -> Result<()> {
        let mut controller = serialport::new(self.port, 9600)
            .timeout(Duration::from_millis(10))
            .open()?;
        Ok(())
    }
}

impl fmt::Display for Profile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.name)
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
