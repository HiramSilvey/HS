#!/bin/bash
src_dir=$(pwd)
cd ../..
protoc --cpp_out=$src_dir profiles.proto
cd $src_dir
mv profiles.pb.cc profiles.pb.cpp
