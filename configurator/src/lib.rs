use anyhow::Result;
use profiles::profile::layout::action::ActionType::{Analog, Digital};
use profiles::profile::layout::Action;
use profiles::profile::Platform::{Pc, Unknown};
use profiles::profile::{Layout, Platform, PlatformConfig};
use profiles::Profile;
use prost::Message;
use std::cmp;
use std::fmt;
use std::fs;
use std::io::Cursor;
use std::path::Path;
use std::time::Duration;
use std::vec::Vec;

const PLATFORMS: [Platform; 1] = [Pc];
const BUTTON_ID_BITS: usize = 5;
const BUTTON_VALUE_BITS: usize = 10;
const BITMASKS: [i32; 10] = [
    0b1,
    0b11,
    0b111,
    0b1111,
    0b11111,
    0b111111,
    0b1111111,
    0b11111111,
    0b111111111,
    0b1111111111,
];

pub mod profiles {
    include!(concat!(env!("OUT_DIR"), "/configurator.profiles.rs"));
}

#[derive(Debug, Eq, Ord, PartialEq, PartialOrd)]
struct PlatformMask {
    priority: u8,
    position: u8,
}

struct Header {
    platform_bitmap: u8,
    positions: Vec<u8>,
}

struct Button {
    num_bits: u8,
    data: u16,
}

struct Body {
    size: u8,
    data: Vec<u8>,
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

    fn encode_header(configs: &Vec<PlatformConfig>) -> Option<Vec<u8>> {
        let mut masks = Vec::new();
        for config in configs {
            if config.platform == Unknown as i32 || config.position < 0 || config.position > 255 {
                return None;
            }
            masks.push(PlatformMask {
                priority: PLATFORMS
                    .iter()
                    .position(|&x| x as i32 == config.platform)? as u8,
                position: config.position as u8,
            });
        }
        masks.sort();

        let mut header = Header {
            platform_bitmap: 0,
            positions: Vec::new(),
        };
        let mut curr_positions: u8 = 0;
        for i in 0..masks.len() {
            let mask = &masks[i];
            header.platform_bitmap |= 1 << mask.priority + 7;
            if i % 2 == 0 {
                curr_positions |= mask.position << 4;
            } else {
                curr_positions |= mask.position;
                header.positions.push(curr_positions);
            }
        }
        if curr_positions > 0 {
            header.positions.push(curr_positions);
        }
        let mut encoded = Vec::new();
        encoded.push(header.platform_bitmap);
        encoded.append(&mut header.positions);
        Some(encoded)
    }

    fn get_button(action: &Action) -> Option<Button> {
        let mut encoded = Button {
            num_bits: BUTTON_ID_BITS as u8,
            data: 0,
        };
        let id_mask = BITMASKS[BUTTON_ID_BITS - 1];
        let value_mask = BITMASKS[BUTTON_VALUE_BITS - 1];
        match action.action_type.as_ref()? {
            Digital(x) => {
                encoded.data = (x & id_mask) as u16;
            }
            Analog(x) => {
                if x.id != 0 {
                    encoded.num_bits = (BUTTON_ID_BITS + BUTTON_VALUE_BITS) as u8;
                    encoded.data =
                        ((x.id & id_mask << BUTTON_VALUE_BITS) | (x.value & value_mask)) as u16;
                }
            }
        }
        Some(encoded)
    }

    fn encode_body(layout: &Layout) -> Option<Vec<u8>> {
        let actions = [
            layout.thumb_top.as_ref()?,
            layout.thumb_middle.as_ref()?,
            layout.thumb_bottom.as_ref()?,
            layout.index_top.as_ref()?,
            layout.index_middle.as_ref()?,
            layout.middle_top.as_ref()?,
            layout.middle_middle.as_ref()?,
            layout.middle_bottom.as_ref()?,
            layout.ring_top.as_ref()?,
            layout.ring_middle.as_ref()?,
            layout.ring_bottom.as_ref()?,
            layout.pinky_top.as_ref()?,
            layout.pinky_middle.as_ref()?,
            layout.pinky_bottom.as_ref()?,
            layout.left_index_extra.as_ref()?,
            layout.left_middle_extra.as_ref()?,
            layout.left_ring_extra.as_ref()?,
            layout.right_index_extra.as_ref()?,
            layout.right_middle_extra.as_ref()?,
            layout.right_ring_extra.as_ref()?,
        ];
        let mut encoded: Vec<u8> = Vec::new();
        let mut curr_byte: u8 = 0;
        let mut available = 8;
        for action in actions.iter() {
            let button = Self::get_button(action)?;
            let mut remaining = button.num_bits;
            let mut data = button.data;
            while remaining > 0 {
                let num_bits = cmp::min(available, remaining);
                let bits: u8 = (data & BITMASKS[(num_bits as usize) - 1] as u16) as u8;
                curr_byte |= bits << available - num_bits;
                available -= num_bits;
                remaining -= num_bits;
                data >>= num_bits;
                if available == 0 {
                    encoded.push(curr_byte);
                    curr_byte = 0;
                    available = 8;
                }
            }
        }
        if available < 8 {
            encoded.push(curr_byte);
        }
        Some(encoded)
    }

    fn encode_profile(profile: &Profile) -> Option<Vec<u8>> {
        let mut header = Self::encode_header(&profile.platform_config)?;
        let mut body = Self::encode_body(profile.layout.as_ref()?)?;
        let mut encoded: Vec<u8> = Vec::new();
        encoded.append(&mut header);
        encoded.append(&mut body);
        Some(encoded)
    }

    fn encode(&self) -> Option<Vec<u8>> {
        let mut encoded: Vec<u8> = Vec::new();
        for profile in &self.profiles {
            encoded.append(&mut Self::encode_profile(&profile)?);
        }
        Some(encoded)
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
