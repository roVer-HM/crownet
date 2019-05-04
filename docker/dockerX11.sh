#!/bin/bash
#
# Run a docker container allowing X11 output.

xhost +"local:docker@"
# xhost +"local:root"

echo "Run docker container with access to X11 and your home directory..."

docker run -it \
--user $(id -u) \
-e DISPLAY=$DISPLAY \
--workdir=$(pwd) \
--volume="/home/$USER:/home/$USER" \
--volume="/etc/group:/etc/group:ro" \
--volume="/etc/passwd:/etc/passwd:ro" \
--volume="/etc/shadow:/etc/shadow:ro" \
--volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
--volume="/tmp/.X11-unix:/tmp/.X11-unix" \
"$@"

