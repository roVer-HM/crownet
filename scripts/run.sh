#!/bin/bash

# Function to run commands in a new terminal window
run_in_new_window () {
    gnome-terminal -- bash -c "$1; exec bash"
}

# Go into /scripts and run source crownetenv then sumo
cd /scripts
source crownetenv
sumo &

# Go into /scripts and run source crownetenv then omnetpp-ide in a new terminal
command="cd /scripts; source crownetenv; omnetpp-ide"
run_in_new_window "$command"
