#!/bin/bash
# 
echo "Welcome to the roVer sumo Docker Container."

export PATH=/opt/sumo/sumo/bin:$PATH

# start sumo GUI
sumo-gui

echo "Container terminated."
