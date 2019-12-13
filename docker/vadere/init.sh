#!/bin/bash
#

echo "Welcome to the roVer vadere Docker Container."
echo ""
echo "Using TRACI_PORT='$TRACI_PORT' TRACI_GUI='$TRACI_GUI'"
echo ""
echo "To launch vadere-gui call vadere exec vadere-gui"

# default jar to be started when container is launched:
# JAR_PATH=/opt/vadere/vadere/VadereGui/target/vadere-gui.jar
JAR_PATH=/opt/vadere/vadere/VadereManager/target/vadere-server.jar


CMD_ARR=(java -jar $JAR_PATH)
# default to traci port 9998 to allow simultanous use of vadere and sumo
if [[ -z $TRACI_PORT ]];then
  CMD_ARR+=(--port 9998)
else
  CMD_ARR+=(--port $TRACI_PORT)
fi

if [[ ${TRACI_GUI} == "true" ]];then
  CMD_ARR+=(--gui-mode)
fi

# echo "${CMD_ARR[@]}"
eval ${CMD_ARR[@]}

echo "Container terminated."
