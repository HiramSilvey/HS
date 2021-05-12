enum ConsoleID {
    PC,
    N64,
    Gamecube,
    Switch,
}

enum Action {
    A,
    B,
    X,
    Y,
    L1,
    L2,
    R1,
    R2,
    RStickUp,
    RStickDown,
    RStickLeft,
    RStickRight,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    Plus,  // Start, Options, Menu
    Minus, // Share, Change View
    Home,
    Capture,
}

struct Profile {
    console: ConsoleID,
    thumb_top: Action,
    thumb_middle: Action,
    thumb_bottom: Action,
    index_top: Action,
    index_middle: Action,
    middle_top: Action,
    middle_middle: Action,
    middle_bottom: Action,
    ring_top: Action,
    ring_middle: Action,
    ring_bottom: Action,
    pinky_top: Action,
    pinky_middle: Action,
    pinky_bottom: Action,
    left_index_extra: Action,
    left_middle_extra: Action,
    left_ring_extra: Action,
    right_index_extra: Action,
    right_middle_extra: Action,
    right_ring_extra: Action,
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
