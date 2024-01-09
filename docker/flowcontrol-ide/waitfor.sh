#!/bin/bash
# 
# Start a process and do not terminate until the process exits
# The process to be started has to be the first parameter.
#
# (Useful for example in order to guarantee that a container does not terminate until a specific process exits.)

$@
sleep 3
PID=`pidof $1`

# The first parameter $1 can for example be the omnetpp launcher.
# omnetpp is special since the start script in its bin directory returns immediately
# Therefore, we wait until the omnetpp process terminates. (cannot use wait since it is not a child process)

if [ -n "$PID" ]; then
  while [ -e /proc/$PID ]; do sleep 0.1; done
fi
