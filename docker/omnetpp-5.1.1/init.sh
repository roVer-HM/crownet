#!/bin/bash
# 
echo "Welcome to the roVer OMNeT++ Docker Container."

export PATH=/opt/omnetpp/omnetpp/bin:$PATH

# check if ptrace_scope is enable  (for debugging it must not be enabled)
read _ _ value < <(/sbin/sysctl kernel.yama.ptrace_scope)
if [[ $value = 1 ]]
then 
    echo "Warning: It seems that your host system does not allow ptrace - debugging will not work."
    echo "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
fi

# start omnet
/opt/omnetpp/omnetpp/bin/omnetpp

# start shell so user can enter additional commands (in interactive mode)
# /bin/bash
sleep 3
PID=`pidof omnetpp`

# omnetpp is special since the start script in its bin directory returns immediately
# Therefore, we wait until the omnetpp process terminates. (cannot use wait since it is not a child process)

if [ -n "$PID" ]; then
   while [ -e /proc/$PID ]; do sleep 0.1; done
fi

echo "Container terminated."
