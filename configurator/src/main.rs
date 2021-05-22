use configurator::Configurator;

fn main() {
    let config = Configurator::new("/some/port", "../profiles").unwrap();
    println!("{}", config);
}
