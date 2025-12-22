#!/bin/bash

# omnetpp configurations to be considered
declare -a configs=("upto-14-ue-WLAN" "upto-14-ue-ramp-WLAN" "upto-14-ue-burst-WLAN" "upto-14-ue-NR" "upto-14-ue-ramp-NR" "upto-14-ue-burst-NR" "noarc-upto-14-ue-WLAN" "noarc-upto-14-ue-ramp-WLAN" "noarc-upto-14-ue-burst-WLAN")
# declare -a configs=("upto-14-ue-ramp-WLAN")
# declare -a configs=("upto-14-ue-WLAN" "upto-14-ue-ramp-WLAN" "upto-14-ue-burst-WLAN")
# declare -a configs=("two-ue-NR")
# declare -a configs=("upto-14-ue-ramp-WLAN")

# analysis scripts to be run (assumes that all results data is available in the ./results folder)
declare -a ANALYSIS=("analysis.py")

ENV_ACTIVATE="out/crownet_user/bin/activate"

if [[ -z "${CROWNET_HOME}" ]]; then
    echo "Environment variable CROWNET_HOME is not set - please set it to the folder in which you cloned the CrowNet repository."
    exit 1
fi

if [[ ! -f "${CROWNET_HOME}/${ENV_ACTIVATE}" ]]; then
    echo "Missing environment activation script: $CROWNET_HOME/$ENV_ACTIVATE"
    echo "Please run \"make analysis-all\" in $CROWNET_HOME to create the environments."
    exit 1
fi

source $CROWNET_HOME/$ENV_ACTIVATE

#install all required packages

pip3 install ipython jupyter
pip3 install numpy pandas matplotlib
pip3 install scipy pivottablejs

pushd .

# convert OMNeT++ scalar and vector files to CSV format using the scavetool
cd ${CROWNET_HOME}/crownet/simulations/arcdsa_lab/results
scavetool="omnetpp exec opp_scavetool x "

# Find all subdirectories in the current directory and iterate over them
subdirs=()

while IFS= read -r -d '' subdir; do
    subdirs+=("$subdir")
done < <(find . -mindepth 1 -maxdepth 2 -type d -print0)


for subdir in "${subdirs[@]}"; do
    pushd .
    echo "Entering directory: $subdir"
    cd "$subdir"
    # Run the scavetool
    for config in "${configs[@]}"; do
        if [[ ! -e "${config}_sca.csv" ]] && [[ -e "${config}.sca" ]]; then
            $scavetool ${config}.sca -o ${config}_sca.csv
        fi
        if [[ ! -e "${config}_vec.csv" ]] && [[ -e "${config}.vec" ]]; then
            $scavetool ${config}.vec -o ${config}_vec.csv
        fi
    done
    popd
done

# run analysis
cd ${CROWNET_HOME}/crownet/simulations/arcdsa_lab/study
for analysis in "${ANALYSIS[@]}"
do
    python3 $analysis
done
popd