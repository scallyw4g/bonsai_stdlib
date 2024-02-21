#! /bin/bash

# build_script_dir="$0"
# echo "build script dir : ($stdlib_build_scripts)"

source "$(dirname ${BASH_SOURCE[0]})/set_platform.sh"
source "$(dirname ${BASH_SOURCE[0]})/check_if_terminal_interactive.sh"

POOF_COLOR_FLAG=""

if [ $RunningInInteractiveTerminal == 1 ]; then
  source "$(dirname ${BASH_SOURCE[0]})/colors.sh"
else
  POOF_COLOR_FLAG="--colors-off"
fi

