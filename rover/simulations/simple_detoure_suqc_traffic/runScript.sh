#!/bin/bash

if [[ -z $ROVER_MAIN ]];then
  echo "Variable ROVER_MAIN not set."
  exit -1
fi

SIM_DIR="$(dirname $(realpath $0))"
OPP_INI="omnetpp.ini"

if [[ -z "$1" ]];then
    CONFIG="final"
else
    CONFIG="$1"
fi


EXPERIMENT="out"
RESULT_DIR="$(realpath "results")"
mkdir -p "$RESULT_DIR"

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
echo "$ROVER_MAIN/scripts/omnetpp_rnd ${CMD_ARR[@]}"
echo ""
$ROVER_MAIN/scripts/omnetpp_rnd "${CMD_ARR[@]}"


if [[ -f ".cmdenv-log" ]];then
	mv -v ".cmdenv-log" "${RESULT_DIR}/${CONFIG}_${EXPERIMENT}"
fi
popd > /dev/null
echo "done."
#if [ "${RESULT_DIR}/${CONFIG}_${EXPERIMENT}" ];then
#    pushd ${RESULT_DIR}/${CONFIG}_${EXPERIMENT} > /dev/null
#    for x in ./* ;do
#        tmp="${x//,/_}"
#        mv -- "$x" "$tmp"
#        tmp2="${tmp//=/}"
#        mv -- "$tmp" "$tmp2"
#        tmp3="${tmp2//-/_}"
#        mv -- "$tmp2" "$tmp3"
#    done
#    popd > /dev/null
#else
#    echo "result dir not found for renaming..."
#fi