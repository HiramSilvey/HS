use configurator::profiles;
use std::path::Path;

fn main() {
    let profiles = profiles::load_all(&Path::new("../profiles")).unwrap();
    for profile in &profiles {
        println!("{}", profile);
    }
    profiles::upload(&profiles, "/dev/ttyACM0").unwrap();
}
