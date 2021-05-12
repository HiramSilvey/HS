use std::collections::HashMap;
use std::fs;
use std::time::Duration;

const DEFAULT_FILENAME: &str = "../profiles/PC/default.json";

fn main() {
    let button_id: HashMap<&str, u8> = [
        ("button_1", 1),
        ("button_2", 2),
        ("button_3", 3),
        ("button_4", 4),
        ("button_5", 5),
        ("button_6", 6),
        ("button_7", 7),
        ("button_8", 8),
        ("button_9", 9),
        ("button_10", 10),
        ("button_11", 11),
        ("button_12", 12),
        ("hat_down", 13),
        ("hat_up", 14),
        ("hat_left", 15),
        ("hat_right", 16),
        ("rstick_down", 17),
        ("rstick_up", 18),
        ("rstick_left", 19),
        ("rstick_right", 20),
        ("slider_left_min", 21),
        ("slider_left_max", 22),
        ("slider_right_min", 23),
        ("slider_right_max", 24),
        ("rstick_y", 25),
        ("rstick_x", 26),
        ("slider_left", 27),
        ("slider_right", 28),
    ]
    .iter()
    .cloned()
    .collect();

    let mut port = serialport::new("/dev/ttyACM0", 9600)
        .timeout(Duration::from_millis(10))
        .open()
        .expect("Failed to open port");

    let file_data =
        fs::read_to_string(DEFAULT_FILENAME).expect("Something went wrong reading the file.");
    let default_profile = json::parse(&file_data).unwrap();

    let output = "This is a test. This is only a test.".as_bytes();
    port.write(output).expect("Write failed!");
}
