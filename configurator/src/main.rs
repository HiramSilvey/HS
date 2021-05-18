use configurator::Configurator;

fn main() {
    let c = Configurator::new("/some/port", "../profiles").unwrap();
    println!("{}", c);
}
