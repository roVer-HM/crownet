#!/bin/bash
#
# Script method library for the roVer project
#
# This bash script contains bash functions/methods used in various other
# scripts in order to avoid redundant code.


# Run a docker container allowing X11 output.
# (For an interactive shell, add the options -ti .)
# @param $@ parameters to be forwarded to the docker run command
function run_container_X11() {
	# allow X11 access for the docker user
	xhost +"local:docker@"
	# xhost +"local:root"

	echo "Run docker container with access to X11 and your home directory..."

	# docker run -ti \
	docker run -t  \
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
}

# check if we already have an existing docker container - run it otherwise
# @param $1 Container short name   (to be used as short container and hostname)
# @param $2 Container full name
# @param $3 Docker image name
function run_start_container() {
	if [ ! "$(docker ps -q -f name=$2)" ]; then
	    if [ "$(docker ps -aq -f status=exited -f name=$2)" ]; then
		# cleanup
		docker rm $2
	    fi
	    # run the container via the X11 docker script
	    run_container_X11 --name $1 --hostname $1 $3
	else
	   # container already exists - start it
	   docker start -i $2
	fi
}
