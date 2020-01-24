#!/bin/bash

if [[ -z $ROVER_MAIN ]];then
  echo "Variable ROVER_MAIN not set."
  exit -1
fi

echo "$PWD"

#SIM_DIR="$ROVER_MAIN/simulation-campaigns/testfolder/template2/input"
SIM_DIR="$PWD/input"
OPP_INI="omnetpp1.ini"
CONFIG="vadere00"


EXPERIMENT="experiment-$(date +"%Y-%m-%d-%s")"
RESULT_DIR="$(realpath "output")"
if [[ -d "$RESULT_DIR" ]];then
	rm -rf "$RESULT_DIR"
fi
mkdir "$RESULT_DIR"

# run
pushd $SIM_DIR > /dev/null
source $ROVER_MAIN/scripts/nedpath > /dev/null

CMD_ARR=(exec opp_runall -j $(nproc) opp_run)
CMD_ARR+=(-u Cmdenv)
CMD_ARR+=(-c "$CONFIG")
CMD_ARR+=("--experiment-label=$EXPERIMENT")
CMD_ARR+=("--result-dir=$RESULT_DIR")
CMD_ARR+=(-l "$ROVER_MAIN/inet4/src/INET")
CMD_ARR+=(-l "$ROVER_MAIN/rover/src/ROVER")
CMD_ARR+=(-l "$ROVER_MAIN/simulte/src/lte")
CMD_ARR+=(-l "$ROVER_MAIN/veins/src/veins")
CMD_ARR+=(-l "$ROVER_MAIN/veins/subprojects/veins_inet/src/veins_inet")
CMD_ARR+=("$OPP_INI")
echo "workingdir: $PWD"
echo "running command:"
echo "$ROVER_MAIN/scripts/omnetpp ${CMD_ARR[@]}"
echo ""
$ROVER_MAIN/scripts/omnetpp "${CMD_ARR[@]}"

if [[ -f ".cmdenv-log" ]];then
	mv -v ".cmdenv-log" "$RESULT_DIR"
fi
popd > /dev/null
