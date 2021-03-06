#!/bin/bash
#
# This script installs docker on a Ubuntu 18.04 machine.

if (( $EUID == 0 )); then
    echo "This script must NOT be run as a root. Please start it without sudo."
    exit
fi

sudo apt -y install apt-transport-https ca-certificates curl
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt update && sudo apt -y install docker-ce

echo "When queried, please enter login credentials for accessing the Docker registry at sam-dev.cs.hm.edu."
echo ""
if [[ ! -d "$HOME/.docker" ]];then
    mkdir "$HOME/.docker"
fi
sudo docker login sam-dev.cs.hm.edu:5023
sudo chown -R $USER ~/.docker

echo "Adding $USER to docker group (change requires a reboot!)."
sudo usermod -a -G docker $USER
echo "Done. Please reboot now and login again."


