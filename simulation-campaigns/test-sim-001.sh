#!/bin/bash

if [[ -z $ROVER_MAIN ]];then
  echo "Variable ROVER_MAIN not set."
  exit -1
fi

SIM_DIR="$ROVER_MAIN/rover/simulations/muenchnerFreiheit/timoKlein"
OPP_INI="omnetpp.ini"
CONFIG="mf_underground"


RESULTS="$(realpath "$0-results")"
if [[ -d "$RESULTS" ]];then
	rm -rf "$RESULTS"
fi

# run
pushd $SIM_DIR > /dev/null
source $ROVER_MAIN/scripts/nedpath > /dev/null

CMD_ARR=($ROVER_MAIN/scripts/omnetpp exec opp_run)
CMD_ARR+=(-u Cmdenv)
CMD_ARR+=(-c "$CONFIG")
CMD_ARR+=("--cmdenv-redirect-output=true")
CMD_ARR+=("--cmdenv-stop-batch-on-error=false")
CMD_ARR+=(-l "$ROVER_MAIN/inet4/src/INET")
CMD_ARR+=("$OPP_INI")
echo "workingdir: $PWD"
echo "call opp_run: ${CMD_ARR[@]}"
eval ${CMD_ARR[@]}
mv -v ./results $RESULTS
if [[ -f ".cmdenv-log" ]];then
	mv -v ".cmdenv-log" "$RESULTS"
fi
popd > /dev/null
