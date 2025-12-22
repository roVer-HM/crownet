#!/bin/bash

declare -a configs=("upto-14-ue-ramp-WLAN" "upto-14-ue-WLAN" "upto-14-ue-burst-WLAN" "upto-14-ue-NR" "upto-14-ue-ramp-NR" "upto-14-ue-burst-NR" "noarc-upto-14-ue-ramp-WLAN" "noarc-upto-14-ue-WLAN" "noarc-upto-14-ue-burst-WLAN")
# declare -a configs=("upto-14-ue-ramp-WLAN")
# declare -a configs=("upto-14-ue-WLAN" "upto-14-ue-ramp-WLAN" "upto-14-ue-burst-WLAN")

set -e
pushd .
cd $CROWNET_HOME/crownet/simulations/arcdsa_lab

for config in "${configs[@]}"
do
    echo "Processing config: $config"
    if [ -d "./results/$config" ]; then
        echo "Results folder for config $config already exists - skipping this config."
        continue
    fi
    echo "Now running config $config."
    omnetpp exec opp_runall -j`nproc` ../../src/run_crownet -u Cmdenv -c $config
done

popd