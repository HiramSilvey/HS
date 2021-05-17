use configurator::Configurator;

fn main() {
    let mut c = Configurator::new("/some/port", "../profiles").expect("Failed!");
    let mut p = c
        .create_profile("TEST".to_string())
        .expect("Already exists!");
    p.position = 5;
    println!("{}", p.position);
}
