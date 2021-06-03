#!/bin/bash
src_dir=$(pwd)
cd ../..
protoc --nanopb_out=$src_dir profiles.proto
