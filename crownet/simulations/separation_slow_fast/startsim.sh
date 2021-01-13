#!/bin/bash

if [[ -z $CROWNET_HOME ]];then
  echo "Variable CROWNET_HOME not set."
  exit -1
fi

echo "$PWD"

SIM_DIR="$PWD"
OPP_INI="omnetpp.ini"
CONFIG="vadere-01"


EXPERIMENT="experiment-$(date +"%Y-%m-%d-%s")"
RESULT_DIR="$(realpath "output")"
if [[ -d "$RESULT_DIR" ]];then
	rm -rf "$RESULT_DIR"
fi
mkdir "$RESULT_DIR"

# run
pushd $SIM_DIR > /dev/null
source $CROWNET_HOME/scripts/nedpath > /dev/null

CMD_ARR=(exec opp_runall -j $(nproc) opp_run)
CMD_ARR+=(-u Cmdenv)
CMD_ARR+=(-c "$CONFIG")
CMD_ARR+=("--experiment-label=$EXPERIMENT")
CMD_ARR+=("--result-dir=$RESULT_DIR")
CMD_ARR+=(-l "$CROWNET_HOME/inet4/src/INET")
CMD_ARR+=(-l "$CROWNET_HOME/crownet/src/CROWNET")
CMD_ARR+=(-l "$CROWNET_HOME/simulte/src/lte")
CMD_ARR+=(-l "$CROWNET_HOME/veins/src/veins")
CMD_ARR+=(-l "$CROWNET_HOME/veins/subprojects/veins_inet/src/veins_inet")
CMD_ARR+=("$OPP_INI")
echo "workingdir: $PWD"
echo "running command:"
echo "$CROWNET_HOME/scripts/omnetpp ${CMD_ARR[@]}"
echo ""
$CROWNET_HOME/scripts/omnetpp "${CMD_ARR[@]}"

if [[ -f ".cmdenv-log" ]];then
	mv -v ".cmdenv-log" "$RESULT_DIR"
fi
popd > /dev/null
