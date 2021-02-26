#!/bin/bash

if [[ -z $CROWNET_HOME ]];then
  echo "Variable CROWNET_HOME not set."
  exit -1
fi

SIM_DIR="$(dirname $(realpath $0))"
OPP_INI="omnetpp.ini"

if [[ -z "$1" ]];then
    CONFIG="vadere00"
else
    CONFIG="$1"
fi


EXPERIMENT="$(date +"%Y-%m-%d-%s")"
RESULT_DIR="$(realpath "results")"
mkdir -p "$RESULT_DIR"

# run
pushd $SIM_DIR > /dev/null
source $CROWNET_HOME/scripts/nedpath > /dev/null

CMD_ARR=(exec gdb --args opp_run_dbg)
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

