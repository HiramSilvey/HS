use std::io::Result;

fn main() -> Result<()> {
    prost_build::compile_protos(
        &["/home/hiram/Projects/HS/src/profiles.proto"],
        &["/home/hiram/Projects/HS/src/"],
    )?;
    Ok(())
}
