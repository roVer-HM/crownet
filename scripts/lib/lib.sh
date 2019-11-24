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
# (For an interactive shell, add the options -ti .)
# @param $@ parameters to be forwarded to the docker run command
function run_container_X11() {
	check_create_network
	# allow X11 access for the docker user
	xhost +"local:docker@"
	# xhost +"local:root"

	echo "Run docker container with access to X11 and your home directory..."

        CMD="docker run -ti \
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
        $@"

        # echo "Docker CMD: \"$CMD\""

        eval $CMD
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
	if [[ $4 == "omnetpp" || $4 == "" ]]; then
                # omnetpp is a special case: we must not terminate until the omnetpp process has terminated
                CMD4="/waitfor.sh $4"
        else
                CMD4=$4
        fi

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
