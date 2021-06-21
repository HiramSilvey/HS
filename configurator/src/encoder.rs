use crate::profile::profile::layout::action::ActionType::{Analog, Digital};
use crate::profile::profile::layout::Action;
use crate::profile::profile::Platform::Unknown;
use crate::profile::profile::{Layout, PlatformConfig};
use crate::profile::Profile;
use anyhow::{anyhow, Result};
use std::cmp;

const BUTTON_ID_BITS: i32 = 5;
const BUTTON_VALUE_BITS: i32 = 10;

#[derive(Debug, Eq, Ord, PartialEq, PartialOrd)]
struct PlatformMask {
    priority: u8,
    position: u8,
}

struct Button {
    num_bits: i32,
    data: i32,
}

fn encode_header(configs: &Vec<PlatformConfig>) -> Result<Vec<u8>> {
    let mut masks = Vec::new();
    for config in configs {
        if config.platform == Unknown as i32 {
            return Err(anyhow!("Required platform is not specified."));
        }
        if config.position < u8::MIN as i32 || config.position > u8::MAX as i32 {
            return Err(anyhow!(
                "Position {} is not able to be represented by an 8-bit unsigned integer.",
                config.position
            ));
        }
        masks.push(PlatformMask {
            priority: config.platform as u8,
            position: config.position as u8,
        });
    }
    masks.sort();

    let mut platform_bitmap: u8 = 0;
    let mut positions: Vec<u8> = Vec::new();
    let mut curr_positions: u8 = 0;
    for i in 0..masks.len() {
        let mask = &masks[i];
        platform_bitmap |= 1 << (8 - mask.priority);
        if i % 2 == 0 {
            curr_positions |= mask.position << 4;
        } else {
            curr_positions |= mask.position;
            positions.push(curr_positions);
        }
    }
    if positions.len() * 2 < masks.len() {
        positions.push(curr_positions);
    }
    println!("platform_bitmap = {}", platform_bitmap);
    println!("positions len = {}", positions.len());
    for position in &positions {
        println!("position = {}", position);
    }
    let mut encoded = Vec::new();
    encoded.push(platform_bitmap);
    encoded.append(&mut positions);
    Ok(encoded)
}

fn get_button(action: &Action) -> Option<Button> {
    let mut encoded = Button {
        num_bits: BUTTON_ID_BITS,
        data: 0,
    };
    match action.action_type.as_ref()? {
        Digital(x) => {
            encoded.data = *x;
        }
        Analog(x) => {
            if x.id != 0 {
                encoded.num_bits = BUTTON_ID_BITS + BUTTON_VALUE_BITS;
                encoded.data = (x.id << BUTTON_VALUE_BITS) | x.value;
            }
        }
    }
    Some(encoded)
}

fn encode_body(layout: &Layout) -> Result<Vec<u8>> {
    let actions = [
        layout.thumb_top.as_ref(),
        layout.thumb_middle.as_ref(),
        layout.thumb_bottom.as_ref(),
        layout.index_top.as_ref(),
        layout.index_middle.as_ref(),
        layout.middle_top.as_ref(),
        layout.middle_middle.as_ref(),
        layout.middle_bottom.as_ref(),
        layout.ring_top.as_ref(),
        layout.ring_middle.as_ref(),
        layout.ring_bottom.as_ref(),
        layout.pinky_top.as_ref(),
        layout.pinky_middle.as_ref(),
        layout.pinky_bottom.as_ref(),
        layout.left_index_extra.as_ref(),
        layout.left_middle_extra.as_ref(),
        layout.left_ring_extra.as_ref(),
        layout.right_index_extra.as_ref(),
        layout.right_middle_extra.as_ref(),
        layout.right_ring_extra.as_ref(),
    ];
    let mut encoded: Vec<u8> = Vec::new();
    let mut curr_byte: u8 = 0;
    let mut available = 8;
    for action in actions.iter() {
        let action = match action {
            Some(x) => x,
            None => return Err(anyhow!("Unable to get action.")),
        };
        let button = match get_button(action) {
            Some(x) => x,
            None => return Err(anyhow!("Unable to get button.")),
        };
        let mut remaining = button.num_bits;
        while remaining > 0 {
            let offset = available - remaining;
            if offset >= 0 {
                curr_byte |= (button.data << offset) as u8;
            } else {
                curr_byte |= (button.data >> i32::abs(offset)) as u8;
            }
            let written = cmp::min(available, remaining);
            available -= written;
            remaining -= written;
            if available == 0 {
                encoded.push(curr_byte);
                curr_byte = 0;
                available = 8;
            }
        }
    }
    if available < 8 {
        encoded.push(curr_byte);
    }
    Ok(encoded)
}

fn encode_profile(profile: &Profile) -> Result<Vec<u8>> {
    let mut header = encode_header(&profile.platform_config)?;
    let layout = match profile.layout.as_ref() {
        Some(x) => x,
        None => return Err(anyhow!("Unable to get layout.")),
    };
    let mut body = encode_body(layout)?;
    let mut encoded: Vec<u8> = Vec::new();
    encoded.append(&mut header);
    encoded.push(body.len() as u8);
    encoded.append(&mut body);
    Ok(encoded)
}

pub fn encode(profiles: &Vec<Profile>) -> Result<Vec<u8>> {
    let mut encoded: Vec<u8> = Vec::new();
    for profile in profiles {
        encoded.append(&mut encode_profile(&profile)?);
    }
    let mut wrapped: Vec<u8> = Vec::new();
    let len = encoded.len() as u16;
    wrapped.push((len >> 8) as u8);
    wrapped.push((len & 0xFF) as u8);
    wrapped.append(&mut encoded);
    Ok(wrapped)
}
