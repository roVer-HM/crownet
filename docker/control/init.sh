#!/bin/bash
#

echo "Welcome to the crownet flowcontrol Docker Container."
echo ""
echo "Using TRACI_PORT='$TRACI_PORT' TRACI_DEBUG='$TRACI_DEBUG' CONTROL_LOG_LEVEL='$CONTROL_LOG_LEVEL'  CONTROL_LOG='$CONTROL_LOG'"
echo ""

'' TODO

CMD_ARR=(crownetctl)
CMD_ARR+=(--bind 0.0.0.0)
# default to traci port 9999 to allow simultanous use of vadere and sumo and control
if [[ -z $TRACI_PORT ]];then
  CMD_ARR+=(--port 9999)
else
  CMD_ARR+=(--port $TRACI_PORT)
fi

if [[ ! -z $TRACI_DEBUG ]];then
  CMD_ARR+=(--log "DEBUG")
  CMD_ARR+=(--controller-log)
fi



if [[ ! -z $CONTROLLER_LOG_LEVEL ]];then
    CMD_ARR+=(--controller-log-level $VADERE_LOG_LEVEL)
fi

if [[ ! -z $CONTROLLER_LOG ]];then
    CMD_ARR+=(--controller-log $VADERE_LOG)
fi


# echo "${CMD_ARR[@]}"
eval ${CMD_ARR[@]}

echo "Container terminated."
