#!/bin/bash
# 
PORT=9999
COMMAND=sumo-gui
# IP=`ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1'

echo "Welcome to the roVer sumo Docker Container."
echo ""
echo "The sumo-launchd is waiting for incoming TraCI connections on port: $PORT"
echo "($COMMAND will be started automatically for incoming TraCI connections.)"
echo ""
echo "If you want to start sumo-gui manually, you can do this via docker exec:"
echo "  docker exec sumo sumo-gui" 

export PATH=/opt/sumo/sumo/bin:$PATH

# start sumo-launchd (command: sumo-gui, sumo)
/opt/veins/sumo-launchd.py -v --bind=0.0.0.0 --command=sumo-gui 

echo "Container terminated."
