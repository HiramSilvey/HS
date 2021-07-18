// Copyright 2021 Hiram Silvey

use profile::profile::layout::action::ActionType::{Analog, Digital};
use profile::profile::layout::Action;
use profile::profile::Layout;
use profile::Profile;
use std::fmt;

pub mod encoder;
pub mod profile {
    include!(concat!(env!("OUT_DIR"), "/hs.profile.rs"));
}
pub mod profiles;

impl fmt::Display for Action {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let action_type = match &self.action_type {
            Some(x) => x,
            None => return Err(fmt::Error),
        };
        match action_type {
            Digital(x) => write!(f, "{}", x)?,
            Analog(x) => write!(f, "{}, {}", x.id, x.value)?,
        }
        Ok(())
    }
}

impl fmt::Display for Layout {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let actions = [
            ("thumb top", self.thumb_top.as_ref()),
            ("thumb middle", self.thumb_middle.as_ref()),
            ("thumb bottom", self.thumb_bottom.as_ref()),
            ("index top", self.index_top.as_ref()),
            ("index middle", self.index_middle.as_ref()),
            ("middle top", self.middle_top.as_ref()),
            ("middle middle", self.middle_middle.as_ref()),
            ("middle bottom", self.middle_bottom.as_ref()),
            ("ring top", self.ring_top.as_ref()),
            ("ring middle", self.ring_middle.as_ref()),
            ("ring bottom", self.ring_bottom.as_ref()),
            ("pinky top", self.pinky_top.as_ref()),
            ("pinky middle", self.pinky_middle.as_ref()),
            ("pinky bottom", self.pinky_bottom.as_ref()),
            ("left index extra", self.left_index_extra.as_ref()),
            ("left middle extra", self.left_middle_extra.as_ref()),
            ("left ring extra", self.left_ring_extra.as_ref()),
            ("right index extra", self.right_index_extra.as_ref()),
            ("right middle extra", self.right_middle_extra.as_ref()),
            ("right ring extra", self.right_ring_extra.as_ref()),
        ];
        for action in actions.iter() {
            let unwrapped = match action.1 {
                Some(x) => x,
                None => return Err(fmt::Error),
            };
            writeln!(f, "\t{}: {}", action.0, unwrapped)?;
        }
        Ok(())
    }
}

impl fmt::Display for Profile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "{{")?;
        writeln!(f, "\tname: {}", self.name)?;
        for platform_config in &self.platform_config {
            writeln!(f, "\tplatform: {}", platform_config.platform)?;
            writeln!(f, "\tposition: {}", platform_config.position)?;
        }
        let layout = match &self.layout {
            Some(x) => x,
            None => return Err(fmt::Error),
        };
        write!(f, "{}", layout)?;
        writeln!(f, "}}")?;
        Ok(())
    }
}
