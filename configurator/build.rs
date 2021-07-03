use std::io::Result;
use std::env;

fn main() -> Result<()> {
    prost_build::compile_protos(
        &[env::var("HS_PATH_TO_PROTO").unwrap()],
        &[env::var("HS_PATH").unwrap()],
    )?;
    Ok(())
}
