#!/bin/bash
#

echo "Welcome to the corwd-guidance Docker Dev-Container.(Uses code in ${CROWNET_HOME}/flowcontrol)"
echo ""

ENV="${CROWNET_HOME}/flowcontrol/.venv_dev"

if [[ ! -d "$ENV" ]];then
  python3 -m venv $ENV
fi

# load enviroment
. "${ENV}/bin/activate"

# install from source
PWD_=$(pwd)
cd $CROWNET_HOME/flowcontrol
echo "uninstall flowcontrol" > ${ENV}/log.out
pip3 uninstall flowcontrol --yes -quiet >> ${ENV}/log.out  2>&1
echo "install flowcontrol" >> ${ENV}/log.out
python3 setup.py install --quiet  >> ${ENV}/log.out 2>&1
cd $PWD_


# execute control script
python3 $@

echo "Container terminated."

