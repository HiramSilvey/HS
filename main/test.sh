#!/bin/bash

# Copyright 2024 Hiram Silvey

src_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
cd "$src_dir/build"
cmake .. && cmake --build . --verbose && {
	./configurator_test
	./controller_test
	./decoder_test
	./hall_joystick_test
	./ns_controller_test
	./pc_controller_test
	./pins_test
	./util_test
}
