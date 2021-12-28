
#!/bin/bash

# Copyright 2021 Hiram Silvey

src_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $src_dir/build
cmake .. && cmake --build . --verbose && { ./controller_test; ./pc_controller_test; }
