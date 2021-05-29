use configurator::Configurator;

fn main() {
    let config = Configurator::new("/dev/ttyACM0", "../profiles").unwrap();
    println!("{}", config);
    config.upload().unwrap();
}
