use configurator::profiles::profile;
use configurator::Configurator;

fn main() {
    let mut config = Configurator::new("/some/port", "../profiles").unwrap();
    let mut profile = config.create_profile("TEST".to_string()).unwrap();
    profile.platform.push(profile::Platform::Pc as i32);
    profile.position = 0;
    let mut layout = profile::Layout::default();
    layout.thumb_top = Some(profile::layout::Action {
        action_type: Some(profile::layout::action::ActionType::Digital(
            profile::layout::DigitalAction::A as i32,
        )),
    });
    profile.layout = Some(layout);

    config.save_profiles();
    println!("{}", config);
}
