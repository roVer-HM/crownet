#!/bin/bash
#
# This init-script launches a single command within the omnet container.
# The command is supplied by using the run-parameter.
#
# Examples
#
# /bin/bash (default)      starts an interactive shell

export PATH=/opt/omnetpp/omnetpp/bin:~/.local/bin:$PATH

# echo "Command: $1"

if [ -z "$1" ]; then
     CMD="/bin/bash"
else
     CMD="$1"
     if [[ "$CMD" != "/bin/bash" && "$CMD" != "/bin/sh" && "$CMD" != "sh" ]]; then
          SILENT="y"
     fi
fi


if [ -z "$SILENT" ]; then
     echo "Welcome to the CrowNet OMNeT++ Docker Container."

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


if [ -z "$SILENT" ]; then
     if [ $TEST_STATUS -eq 0 ]; then echo "Container terminated."; else echo "Container terminated (ERROR: $TEST_STATUS)."; fi
fi

exit $TEST_STATUS
