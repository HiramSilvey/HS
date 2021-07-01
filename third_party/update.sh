#!/bin/bash

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$src_dir/.."
git fetch nsgadget master
git subtree pull --prefix third_party/nsgadget nsgadget master --squash
