#!/bin/bash


function checkOK() {
    if [ "$?" -ne 0 ]; then
        echo "error in command"
        exit -1
    fi
}

NUM=5
SCENARIO="vadere/scenarios/mf_m_dyn_const_4e20s_15x8_120.scenario"
OUT="misc_pos2.ini"


num=(10 26 50 76 100 126 150 176 200)

# for n in ${num[@]}; do
#     echo "Create with number of runs $n"
#     rnd_stationary_positions \
#         --vadere $SCENARIO \
#         --number-of-runs $NUM \
#         --number-of-positions ${n} \
#         --seed-type order \
        # --append \
#         --output $OUT
#       checkOK
# done

density=(0.000611583389395144 0.0007828267384257843 0.0009296067518806189 \
         0.0010763867653354536 0.001223166778790288)

run=0

for n in ${density[@]}; do
    echo "Create with density ${n} on full area $n"
    rnd_stationary_positions \
        --vadere $SCENARIO \
        --number-of-runs $NUM \
        --const-density ${n} \
        --seed-type order \
        --config-name "full_${run}" \
        --append \
        --output $OUT
    checkOK

    echo "Create with density ${n} on half area $n"
    rnd_stationary_positions \
        --vadere $SCENARIO \
        --number-of-runs $NUM \
        --const-density ${n} \
        --seed-type order \
        --offset 0.25 0.25 0.5 0.5 \
        --config-name "quater_${run}" \
        --append \
        --output $OUT
    checkOK
    run=$(($run + 1))
done