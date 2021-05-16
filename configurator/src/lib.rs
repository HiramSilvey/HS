mod configurator {
    use anyhow::Result;
    use serialport::SerialPort;
    use std::fs;
    use std::path::Path;
    use std::time::Duration;

    pub struct Configurator<'a> {
        controller: Box<dyn SerialPort>,
        path: &'a Path,
    }

    impl<'a> Configurator<'a> {
        pub fn new(port: &str, profiles_dir: &'a str) -> Result<Configurator<'a>> {
            let controller = Configurator::open_connection(port)?;
            let path = Path::new(profiles_dir);
            Configurator::load_profiles(path)?;
            Ok(Configurator {
                controller: controller,
                path: path,
            })
        }

        fn open_connection(port: &str) -> Result<Box<dyn SerialPort>> {
            Ok(serialport::new(port, 9600)
                .timeout(Duration::from_millis(10))
                .open()?)
        }

        fn load_profiles(path: &Path) -> Result<()> {
            for entry in fs::read_dir(path)? {
                let entry = entry?;
                let p = entry.path();
                if p.is_file() {
                    println!("{}", p.display());
                }
            }
            Ok(())
        }

        fn save_profiles(&self) -> Result<()> {
            Ok(())
        }

        fn write(&self) -> Result<()> {
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
}
