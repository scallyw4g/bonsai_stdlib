#! /bin/bash

# build_script_dir="$0"

echo "build script dir : ($stdlib_build_scripts)"

. $stdlib_build_scripts/set_platform.sh
. $stdlib_build_scripts/check_if_terminal_interactive.sh

if [ $RunningInInteractiveTerminal == 1 ]; then
  . $stdlib_build_scripts/colors.sh
fi

