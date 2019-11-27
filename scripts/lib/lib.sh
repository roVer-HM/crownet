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

# Wrap a command so that is executed within a shell
# @param $1       command to be executed
# @param $2-${10} arguments to be passed to the executed command
# @return String with the appropriately wrapped command
function wrap_command() {
        # We try to guess if we need to connect the console: For all exept omnetpp
        # we currently connect a console - behavior might need to change in future
	if [[ $1 == *"omnetpp"* ]]; then
           WRAPPER="/waitfor.sh "
           WRAPPER_END=""
	else
           WRAPPER="bash -c \"cd $(pwd);"
           WRAPPER_END="\""
        fi
        echo "$WRAPPER $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} $WRAPPER_END"
}

# Run a docker container allowing X11 output.
# (If an tty is detected, the option -i is automatically added.)

# @param $@ parameters to be forwarded to the docker run command
function run_container_X11() {
	check_create_network
	# allow X11 access for the docker user
	xhost +"local:docker@"
	# xhost +"local:root"

	# Detect if a tty is connected as standard input
	# https://gist.github.com/davejamesmiller/1966557
	if [[ -t 0 ]] # called normally - Terminal input (keyboard) - interactive
	then
    		MODE="-ti"
	else # input from pipe or file - do not allocate a pseudo-terminal
    		MODE="-i"
	fi

	echo "Run docker container with access to X11 and your home directory..."

	CMD_ARR=(docker run $MODE)
	CMD_ARR+=(--cap-add=SYS_PTRACE)
	CMD_ARR+=(--user $(id -u))
	CMD_ARR+=(--network $INTNET)
	CMD_ARR+=(--env DISPLAY=$DISPLAY)
	CMD_ARR+=(--workdir=$(pwd))
	CMD_ARR+=(--volume="/home/$USER:/home/$USER")
	CMD_ARR+=(--volume="/etc/group:/etc/group:ro")
	CMD_ARR+=(--volume="/etc/passwd:/etc/passwd:ro")
	CMD_ARR+=(--volume="/etc/shadow:/etc/shadow:ro")
	CMD_ARR+=(--volume="/etc/sudoers.d:/etc/sudoers.d:ro")
	CMD_ARR+=(--volume="/tmp/.X11-unix:/tmp/.X11-unix")
	if [[ ! -z ${DOCKER_VADERE_CACHE_LOCATION} && $2 == *"vadere"* ]];then
		# (default not set.) only used for vadere containers.
		# set DOCKER_VADERE_CACHE_LOCATION to new location of vadere cache.
		# the default location of the chache is in '/home/$USER/.cache/vadere' and
		# is alrady accessable to the container. If the cache is symlinked to another
		# location outside of /home/$USER/ the container cannot follow the link.
		# Adding a volume at the location of the symlink will allow access to the
		# new cache location. (e.g. DOCKER_VADERE_CACHE_LOCATION=/opt/vadere-cache)
		echo "Use custome cache location for vadere. Mount -> /home/$USER/.cache/vadere:${DOCKER_VADERE_CACHE_LOCATION}"
		CMD_ARR+=(--volume="/home/$USER/.cache/vadere:${DOCKER_VADERE_CACHE_LOCATION}")
	fi
	CMD_ARR+=($@)

	echo "docker run ${CMD_ARR[@]} $@"
	eval ${CMD_ARR[@]}

}

# check if we already have an existing docker container - run it otherwise
# @param $1 Container short name   (to be used as short container and hostname)
# @param $2 Container full name
# @param $3 Docker image name
# @param $4 - $9 are passed to the container
function run_start_container() {
	CMD4="$(wrap_command $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15})"
	if [ ! "$(docker ps -q -f name=^/$1$)" ]; then
	    if [ "$(docker ps -aq -f status=exited -f name=^/$1$)" ]; then
		# cleanup
		docker rm $1
	    fi
	    # run the container via the X11 docker script
            run_container_X11 --name $1 --hostname $1 $3 $CMD4
 	else
	    # container already exists - execute command within container
            echo "CMD4=\"$CMD4\""
       	    docker exec -i $1 bash -c "cd $(pwd); $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15}"
	fi
}

# create a new (randomly named) container and start it with the specified command
# @param $1 Container short name   (to be used as short container and hostname)
# @param $2 Container full name
# @param $3 Docker image name
# @param $4 - $9 are passed to the container
function run_individual_container() {
	CMD4="$(wrap_command $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15})"
        run_container_X11 --rm --hostname $1 $3 $CMD4
}


function check_ptrace() {
	read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
        if [[ $value = 1 ]]
        then
           echo "Warning: It seems that your system does not allow ptrace - debugging will not work."
           echo "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
        fi
}
