#!/bin/bash
#
echo "Welcome to the roVer vadere Docker Container."
echo ""

# default jar to be started when container is launched:
JAR_PATH=/opt/vadere/vadere/VadereGui/target/vadere-gui.jar

if [ ! -f $JAR_PATH ]; then
    echo "Error: jar file $JAR_PATH not found."
    echo "Hint:  Within the container, only specific paths - e.g. your home directory $HOME - are visible!"
    exit 1
fi

echo ""

# start vadere GUI
java -jar $JAR_PATH $2 $3 $4 $5 $6 $7 $8 $9

echo "Container terminated."
