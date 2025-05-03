#!/bin/bash
# IP=`ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1'

SUMO_PATH=`readlink /opt/sumo/sumo`
SUMO_VERSION=`ls /opt/sumo | grep v`

echo "Welcome to the CrowNet SUMO ($SUMO_VERSION) Docker Container."
echo " - SUMO installation: $SUMO_PATH"
echo ""
echo "Using TRACI_PORT='$TRACI_PORT' TRACI_GUI='$TRACI_GUI'"
echo ""
echo "If you want to start sumo-gui manually, you can do this via:"
echo "  sumo exec sumo-gui"

export PATH=/opt/sumo/sumo/bin:$PATH

# start sumo-launchd (command: sumo-gui, sumo)
CMD_ARR=(/veins_launchd -v)
CMD_ARR+=(--bind=0.0.0.0)
if [[ -z $TRACI_PORT ]];then
  CMD_ARR+=(--port=9999)
else
  CMD_ARR+=(--port=$TRACI_PORT)
fi
if [[ ${TRACI_GUI} == "true" ]];then
  CMD_ARR+=(--command=sumo-gui)
else
  CMD_ARR+=(--command=sumo)
fi

# echo "${CMD_ARR[@]}"
eval ${CMD_ARR[@]}

echo "Container terminated."
