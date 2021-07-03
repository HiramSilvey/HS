#!/bin/bash

# Build script to compile the profile protobuf to minimal code size .c and .h
# files using nanopb protoc. This script expects the first argument to be the
# path to the directory containing the nanopb protoc binary.

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$src_dir/../.."
protoc="$1/protoc"
$protoc --nanopb_out=$src_dir profile.proto
