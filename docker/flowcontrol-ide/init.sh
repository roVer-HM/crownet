#!/bin/bash

#
# This init-script launches a single command within the omnet container.
# The command is supplied by using the run-parameter.
#
# Examples
#
# pycharm-community.sh (default)   starts the PyCharm IDE
# /bin/bash                        starts an interactive shell

# echo "Command: $1"

if [ -z "$1" ]; then
     CMD="pycharm-community.sh"
else
     CMD="$1"
     if [[ "$CMD" != "pycharm-community.sh" && "$CMD" != "/bin/sh" && "$CMD" != "sh" ]]; then
          SILENT="y"
     fi
fi


if [ -z "$SILENT" ]; then
     echo "Welcome to the flowcontrol IDE Docker Container ($RELEASE)."

     # check if ptrace_scope is enable  (for debugging it must not be enabled)
     read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
     if [[ $value = 1 ]]
     then
         echo "Warning: It seems that your host system does not allow ptrace - debugging will not work."
         echo "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
     fi
fi

# execute command
CMD="$CMD $2 $3 $4 $5 $6 $7 $8 $9 ${10}"

eval $CMD; TEST_STATUS=${PIPESTATUS[0]}

if [[ "$CMD" == "pycharm-community.sh" ]]; then
     sleep 3
     PID=`pidof pycharm-community.sh`

     # idea.sh is special since the start script in its bin directory returns immediately
     # Therefore, we wait until the idea.sh process terminates. (cannot use wait since it is not a child process)

     if [ -n "$PID" ]; then
          while [ -e /proc/$PID ]; do sleep 0.1; done
     fi
fi


if [ -z "$SILENT" ]; then
     if [ $TEST_STATUS -eq 0 ]; then echo "Container terminated."; else echo "Container terminated (ERROR: $TEST_STATUS)."; fi
fi

exit $TEST_STATUS
