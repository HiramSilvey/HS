use std::time::Duration;

fn main() {
    let mut port = serialport::new("/dev/ttyACM0", 9600)
        .timeout(Duration::from_millis(10))
        .open()
        .expect("Failed to open port");
    let output = "This is a test. This is only a test.".as_bytes();
    port.write(output).expect("Write failed!");
}
