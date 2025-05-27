#!/bin/bash
#

echo "Welcome to the crowd-guidance Docker Dev-Container ($RELEASE).(Uses code in ${CROWNET_HOME}/flowcontrol)"
echo ""


# load enviroment
. "/opt/local_venv/bin/activate"

# install from source
export PYTHONPATH="${PYTHONPATH}:$CROWNET_HOME/flowcontrol"

# execute control script
python3 $@

echo "Container terminated."

