#!/bin/bash

# Build script to compile the profile protobuf for python use.

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$src_dir/../.."
protoc -I="." --python_out="./configurator/profiles/build" "./profile.proto"

python3 configurator/profiles/build/encode.py
