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
	  log_ "Creating $INTNET network ..."
	  docker network create $INTNET
	# else
	  # log_ "Network $INTNET for communication between the roVer containers exists."
	fi
}

function log_() {
    if [[ "$SILENT" != "y" ]]; then
        echo  $*
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
           WRAPPER="/bin/bash -c \"cd $(pwd);"
           WRAPPER_END="\""
    fi
    # not printet to shell
    echo "$WRAPPER $@ $WRAPPER_END"
}

# Create name for container by adding incrementing numbers to the base name.
# Needed because network dns is based on the container name not on the
# hostname inside the container. See https://stackoverflow.com/a/43033828/12079469
# $1 shortname
function create_name(){
	CURR_NAME=$1
	i=0

    #{
        #flock --verbose -w 10 200 || { log_ "lock timeout on $lock"; exit -1; }
        #log_ "$$: Lock recieved $lock"
        #log_ "$$: received fd"
        #log_ "$$: $1"
        #log_ "$$: $1" > "$nfile"
        #sleep 1
        #prev=$(cat $nfile)
        #log_ "$$: prev $prev"
        #log_ "$$: Lock released"
        #flock -u 200
    #} 200>"$lock"

	while [[ "$(docker ps -aq -f name=^/${CURR_NAME}$)" ]]; do
		if [ $i -gt 200 ]; then
				echo "Error finding a new name for container $1. (Stopt at ${CURR_NAME})"
				exit -1
		fi
		CURR_NAME="${CURR_NAME}${i}"
		((i++))
	done
	echo $CURR_NAME
}

# Run a docker container allowing X11 output.
# (If an tty in foreground is detected, the option -i is automatically added.)

# @param $@ parameters to be forwarded to the docker run command
function run_container_X11() {
	check_create_network
	# allow X11 access for the docker user
	if [[ "$OSTYPE" == "darwin"* ]];then
 		xhost +"localhost"
	else
		xhost +"local:docker@" >/dev/null
		# xhost +"local:root"
	fi

	# Detect if a tty is connected as standard input
	# https://gist.github.com/davejamesmiller/1966557
	if [[ -t 0 ]] # called normally - Terminal input (keyboard) - interactive
	then
    		MODE="-t"
	else # input from pipe or file - do not allocate a pseudo-terminal
    		MODE=""
	fi

	# Detect if we are running in foreground or background
        # (running --interactive in background leads to a high load docker process)
	case $(ps -o stat= -p $$) in
	  *+*) MODE="$MODE --interactive" ;;
	  *) # Running in background - do not add the --interactive flag ;;
	esac

	# log_ "Running docker container with access to X11 and your home directory..."

	CMD_ARR=(docker run $MODE)
	CMD_ARR+=(--cap-add=SYS_PTRACE)
	CMD_ARR+=(--user $(id -u))
	CMD_ARR+=(--network $INTNET)
	if [[ "$OSTYPE" == "darwin"* ]];then
		CMD_ARR+=(--env DISPLAY="host.docker.internal:0")
	else
		CMD_ARR+=(--env DISPLAY=$DISPLAY)
	fi
	if [[ ! -z ${NEDPATH} ]];then
	  # set NEDPATH in container to simplify opp_run commands.
		CMD_ARR+=(--env NEDPATH=$NEDPATH)
	fi
	if [[ ! -z ${TRACI_PORT} ]];then
		log_ "container> set var TRACI_PORT=$TRACI_PORT"
		CMD_ARR+=(--env TRACI_PORT=$TRACI_PORT)
	fi
	if [[ -z ${TRACI_GUI} ]];then
		CMD_ARR+=(--env TRACI_GUI="false")
	else
		log_ "container> set var TRACI_GUI=$TRACI_GUI"
		CMD_ARR+=(--env TRACI_GUI=${TRACI_GUI})
	fi
	if [[ ! -z ${TRACI_DEBUG} ]];then
  	    log_ "container> set var TRACI_DEBUG=$TRACI_DEBUG"
		CMD_ARR+=(--env TRACI_DEBUG=${TRACI_DEBUG})
	fi
    if [[ ! -z ${VADERE_LOG_LEVEL} ]];then
        log_ "container> set var VADERE_LOG_LEVEL=$VADERE_LOG_LEVEL"
        CMD_ARR+=(--env VADERE_LOG_LEVEL=${VADERE_LOG_LEVEL})
	fi
    if [[ ! -z ${VADERE_LOG} ]];then
        log_ "container> set var VADERE_LOG=$VADERE_LOG"
        CMD_ARR+=(--env VADERE_LOG=${VADERE_LOG})
	fi
    if [[ ! -z ${SILENT} ]];then
        log_ "container> set var SILENT=$SILENT"
        CMD_ARR+=(--env SILENT=${SILENT})
	fi
    CMD_ARR+=(--env CROWNET_HOME=${CROWNET_HOME})
	CMD_ARR+=(--workdir=$(pwd))
	if [[ "$OSTYPE" == "darwin"* ]]; then
		CMD_ARR+=(--volume="/Users/$USER:/home/$USER")
	else
		CMD_ARR+=(--volume="/home/$USER:/home/$USER")
	fi
	if [[ "$OSTYPE" == "linux-gnu"* ]]; then
		CMD_ARR+=(--volume="/etc/group:/etc/group:ro")
		CMD_ARR+=(--volume="/etc/passwd:/etc/passwd:ro")
		CMD_ARR+=(--volume="/etc/shadow:/etc/shadow:ro")
		CMD_ARR+=(--volume="/etc/sudoers.d:/etc/sudoers.d:ro")
	fi
	CMD_ARR+=(--volume="/tmp/.X11-unix:/tmp/.X11-unix")
    if [[ ! -z ${OPP_EXTERN_DATA_MNT} ]];then
	    CMD_ARR+=(--volume="$OPP_EXTERN_DATA_MNT")
    fi
	if [[ ! -z ${DOCKER_VADERE_CACHE_LOCATION} && ($2 == *"vadere"* || $3 == *"vadere"* ) ]];then
		# (default not set.) only used for vadere containers.
		# set DOCKER_VADERE_CACHE_LOCATION to new location of vadere cache.
		# the default location of the chache is in '/home/$USER/.cache/vadere' and
		# is alrady accessable to the container. If the cache is symlinked to another
		# location outside of /home/$USER/ the container cannot follow the link.
		# Adding a volume at the location of the symlink will allow access to the
		# new cache location. (e.g. DOCKER_VADERE_CACHE_LOCATION=/opt/vadere-cache)
		log_ "container> mount cache location '${DOCKER_VADERE_CACHE_LOCATION}' to '/home/$USER/.cache/vadere'"
		CMD_ARR+=(--volume="/home/$USER/.cache/vadere:${DOCKER_VADERE_CACHE_LOCATION}")
	fi
	CMD_ARR+=($@)
	#log_ "${CMD_ARR[@]}"
	eval ${CMD_ARR[@]}

}

# Deprecated.
# # check if we already have an existing docker container - run it otherwise
# # @param $1 Container short name   (to be used as short container and hostname)
# # @param $2 Container full name
# # @param $3 Docker image name
# # @param $4 - $9 are passed to the container
# function run_start_container() {
# 	COMMAND="${@:4:${#@}+1-4}"
# 	CMD4="$(wrap_command $COMMAND)"
# 	log_ "$1"
# 	if [ ! "$(docker ps -q -f name=^/$1$)" ]; then
# 	    if [ "$(docker ps -aq -f status=exited -f name=^/$1$)" ]; then
# 		# cleanup
# 		docker rm $1
# 	    fi
# 	    # run the container via the X11 docker script
#             run_container_X11 --name $1 --hostname $1 $3 $CMD4
#  	else
# 	    # container already exists - execute command within container
# 						COMMAND="${@:4:${#@}+1-4}"
# 						log_ "CMD4=\"$CMD4\""
#        	    docker exec -i $1 bash -c "cd $(pwd); $COMMAND"
# 	fi
# }

# create a new (randomly named) container and start it with the specified command
# @param $1 Container short name   (to be used as short container and hostname)
# @param $2 Container full name
# @param $3 Docker image name
# @param $4 - $9 are passed to the container
function run_individual_container() {
	COMMAND="${@:4:${#@}+1-4}"
	CMD4="$(wrap_command $COMMAND)"
    log_ $CMD4
    if [[ "$1" == *_rnd ]];then
        # let docker create random names.
        run_container_X11 --rm $3 $CMD4
    else
        NAME=$(create_name $1)
        # log_ "Container name: $NAME"
        run_container_X11 --rm --name $NAME --hostname $NAME $3 $CMD4
    fi
}


function check_ptrace() {
	read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
        if [[ $value = 1 ]]; then
           log_ "Warning: It seems that your system does not allow ptrace - debugging will not work."
           log_ "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
        fi
}
