#!/bin/bash
# 
# This init-script launches a single command within the omnet container.
# The command is supplied by using the run-parameter. 
#
# Examples
#
# omnetpp (default)      starts the OMNeT++ IDE
# /bin/bash              starts an interactive shell

export PATH=/opt/omnetpp/omnetpp/bin:$PATH

# echo "Command: $1"

if [ -z "$1" ]; then
     CMD="omnetpp"
     SILENT=1
else
     CMD="$1"
     if [ "$CMD" == "/bin/bash" ]; then
          SILENT=1
     fi
fi


if [ -z "$SILENT" ]; then
     echo "Welcome to the roVer OMNeT++ Docker Container."

     # check if ptrace_scope is enable  (for debugging it must not be enabled)
     read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
     if [[ $value = 1 ]]
     then 
         echo "Warning: It seems that your host system does not allow ptrace - debugging will not work."
         echo "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
     fi
fi

# execute command
$CMD

if [ "$CMD" == "omnetpp" ]; then
     sleep 3
     PID=`pidof omnetpp`

     # omnetpp is special since the start script in its bin directory returns immediately
     # Therefore, we wait until the omnetpp process terminates. (cannot use wait since it is not a child process)

     if [ -n "$PID" ]; then
          while [ -e /proc/$PID ]; do sleep 0.1; done
     fi
fi


if [ -z "$SILENT" ]; then
     echo "Container terminated."
fi
