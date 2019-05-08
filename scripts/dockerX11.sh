#!/bin/bash
#
# Run a docker container allowing X11 output.
# (For an interactive shell, add the options -ti .)

source "$( cd "$(dirname "$0")" ; pwd -P )"/lib/lib.sh

run_container_X11 $@

