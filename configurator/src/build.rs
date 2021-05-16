fn main() -> Result<()> {
    prost_build::compile_protos(&["src/profile.proto"], &["src/"])?;
    Ok(())
}
