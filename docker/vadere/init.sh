#!/bin/bash
#

echo "Welcome to the CrowNet vadere Docker Container ($RELEASE)."
echo ""
echo "Using TRACI_PORT='$TRACI_PORT' TRACI_GUI='$TRACI_GUI' TRACI_DEBUG='$TRACI_DEBUG' VADERE_LOG_LEVEL='$VADERE_LOG_LEVEL'  VADERE_LOG='$VADERE_LOG'"
echo ""
echo "To launch vadere-gui call vadere exec vadere-gui"


# default jar to be started when container is launched:
# JAR_PATH=/opt/vadere/vadere/VadereGui/target/vadere-gui.jar
JAR_PATH=/opt/vadere/vadere/VadereManager/target/vadere-server.jar
LAUCNER_PATH=/opt/vadere/vadere/Tools/vadere-lauchner/vadere-lauchner.py

# print vadere version
java -jar $JAR_PATH --version

CMD_ARR=(python3 $LAUCNER_PATH)
CMD_ARR+=(--bind 0.0.0.0)
CMD_ARR+=(--jar $JAR_PATH)
# default to traci port 9998 to allow simultanous use of vadere and sumo
if [[ -z $TRACI_PORT ]];then
  CMD_ARR+=(--port 9998)
else
  CMD_ARR+=(--port $TRACI_PORT)
fi

if [[ ! -z $TRACI_DEBUG ]];then
  CMD_ARR+=(--log "DEBUG")
  CMD_ARR+=(--vadere-log)
fi

if [[ ${TRACI_GUI} == "true" ]];then
  CMD_ARR+=(--gui-mode)
fi

if [[ ! -z $VADERE_LOG_LEVEL ]];then
    CMD_ARR+=(--vadere-log-level $VADERE_LOG_LEVEL)
fi

if [[ ! -z $VADERE_LOG ]];then
    CMD_ARR+=(--vadere-log $VADERE_LOG)
fi


# echo "${CMD_ARR[@]}"
eval ${CMD_ARR[@]}

echo "Container terminated."
