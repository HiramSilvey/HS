use std::io::Result;

fn main() -> Result<()> {
    prost_build::compile_protos(
        &["/home/hiram/Projects/HS/src/profile.proto"],
        &["/home/hiram/Projects/HS/src/"],
    )?;
    Ok(())
}
