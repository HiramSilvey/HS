use std::path::Path;

struct Controller {
    port: String,
    save_dir: Path,
    board: Box<dyn SerialPort>,
}

impl Controller {
    fn new(port: String, save_dir: Path) -> Controller {
        Controller {
            port: port,
            save_dir: save_dir,
        }
    }

    fn load_profiles(&self) -> Result<()> {}
    fn save_profiles(&self) -> Result<()> {}

    fn connect(&self) -> Result<()> {
        let board = serialport::new(self.port, 9600)
            .timeout(Duration::from_millis(10))
            .open();
        let mut board = match board {
            Ok(port) => port,
            Err(e) => return Err(e),
        };
        self.board = board;
    }

    fn write(&self) -> Result<()> {}
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
