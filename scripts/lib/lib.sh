#!/bin/bash
#
# Script method library for the roVer project
#
# This bash script contains bash functions/methods used in various other
# scripts in order to avoid redundant code.

INTNET="rovernet"

#
function check_create_network() {
	if [ ! "$(docker network ls | grep $INTNET)" ]; then
	  echo "Creating $INTNET network ..."
	  docker network create $INTNET
	else
	  echo "Network $INTNET for communication between the roVer containers exists."
	fi
}

# Run a docker container allowing X11 output.
# (For an interactive shell, add the options -ti .)
# @param $@ parameters to be forwarded to the docker run command
function run_container_X11() {
	check_create_network
	# allow X11 access for the docker user
	xhost +"local:docker@"
	# xhost +"local:root"

	echo "Run docker container with access to X11 and your home directory..."

	if [[ $@ == *"bash"* ]]; then
	  MODE="-ti"
	else
	  MODE="-t"
        fi

	docker run $MODE  \
        --cap-add=SYS_PTRACE \
	--user $(id -u) \
        --network $INTNET \
	--env DISPLAY=$DISPLAY \
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
# @param $4 - $9 are passed to the container
function run_start_container() {
	if [ ! "$(docker ps -q -f name=$1)" ]; then
	    if [ "$(docker ps -aq -f status=exited -f name=$1)" ]; then
		# cleanup
		docker rm $1
	    fi
	    # run the container via the X11 docker script
	    run_container_X11 --name $1 --hostname $1 $3 $4 $5 $6 $7 $8 $9
 	else
	   # container already exists - start it
	   docker start -i $1 $4 $5 $6 $7 $8 $9
	fi
}


function check_ptrace() {
	read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
        if [[ $value = 1 ]]
        then 
           echo "Warning: It seems that your system does not allow ptrace - debugging will not work."
           echo "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
        fi
}
