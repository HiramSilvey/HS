#[cfg(test)]
use mockall::{automock, mock, predicate::*};
use mockall_double::double;

mod encoder;
pub mod profile {
    include!(concat!(env!("OUT_DIR"), "/hs.profile.rs"));
}
pub mod profiles;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn test_upload_too_many_bytes() {
        let config = Configurator::default();
        let encode_ctx = encoder::encode_context();
        encode_ctx
            .expect()
            .return_const(vec![0u8, MAX_EEPROM_BYTES + 1]);
        config.upload();
    }
}
