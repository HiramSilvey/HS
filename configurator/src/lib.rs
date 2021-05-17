use anyhow::Result;
use profiles::Profile;
use std::fs;
use std::path::Path;
use std::time::Duration;
use std::vec::Vec;

mod profiles {
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
        Configurator::load_profiles(path)?;
        Ok(Configurator {
            port: port,
            path: path,
            profiles: Vec::new(),
        })
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

    fn find_profile(&self, name: &str) -> Option<usize> {
        for i in 0..self.profiles.len() {
            if self.profiles[i].name == name {
                return Some(i);
            }
        }
        None
    }

    fn load_profiles(path: &Path) -> Result<()> {
        for entry in fs::read_dir(path)? {
            let entry = entry?;
            let p = entry.path();
            if p.is_dir() {
                println!("{}", p.display());
            }
        }
        Ok(())
    }

    fn save_profiles(&self) -> Result<()> {
        Ok(())
    }

    fn write(&self) -> Result<()> {
        let mut controller = serialport::new(self.port, 9600)
            .timeout(Duration::from_millis(10))
            .open()?;
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
