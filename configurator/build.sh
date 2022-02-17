#!/bin/bash

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$src_dir/src"

cargo +nightly build -Z configurable-env
cargo +nightly build -Z configurable-env --target x86_64-pc-windows-gnu
