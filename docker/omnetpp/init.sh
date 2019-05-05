#!/bin/bash
# 
echo "Welcome to the roVer OMNeT++ Docker Container."

export PATH=/opt/omnetpp/omnetpp/bin:$PATH

# start omnet
/opt/omnetpp/omnetpp/bin/omnetpp

# start shell so user can enter additional commands (in interactive mode)
# /bin/bash

PID=`pidof omnetpp`

# omnetpp is special since the start script in its bin directory returns immediately
# Therefore, we wait until the omnetpp process terminates. (cannot use wait since it is not a child process)

if [ -n "$PID" ]; then
   while [ -e /proc/$PID ]; do sleep 0.1; done
fi

echo "Container terminated."
