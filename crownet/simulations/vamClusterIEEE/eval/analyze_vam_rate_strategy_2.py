#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, count_all_vams
from common.plot_utils import save_plot

RESULTS = "../results"
STUDY_NAME = "RunStudyVeloDifferenceVadereGroupsDistrib4"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
TEMP_FILE = f"{STUDY_NAME}_temp_vamsps.json"
YLABEL2 = "VAMs per second\n+- std. deviation"
YLABEL = "VAMs per second per VRU\n+- std. deviation"
NC_TX = 1.39
NC_RX = 70.55

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 250
END_TIME = 500
NUM_VRUS = 67
NUM_VEH = 8

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()
        parameter_value = get_parameter_value(cur, PARAMETER)

        results_sent = []
        results_received = []
        results_sent_aggr = []

        vams_sent = count_all_vams(
            cur,
            "vam_sent_type",
            "World.pNode[%].middleware.VaService",
            ST_CONV_FACTOR * START_TIME,
            ST_CONV_FACTOR * END_TIME
        )

        vams_received = count_all_vams(
            cur,
            "vam_type",
            "World.vNode[%].middleware.VaService",
            ST_CONV_FACTOR * START_TIME,
            ST_CONV_FACTOR * END_TIME
        )

        if vams_sent:
            results_sent_aggr.append(vams_sent / (END_TIME - START_TIME))
            vams_sent = vams_sent / (END_TIME - START_TIME) # VAMs / Second
            vams_sent = vams_sent / NUM_VRUS # Vams / VRU
            print("---------")
            print(f"TX: {vams_sent}")
            results_sent.append(vams_sent)

        if vams_received:
            vams_received = vams_received / (END_TIME - START_TIME) # VAMs / Second
            vams_received = vams_received / NUM_VEH # Vams / VRU
            print(f"RX: {vams_received}")
            results_received.append(vams_received)

        return (parameter_value, results_sent, results_received, results_sent_aggr)

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = {}

    for result in results:
        if result[0] not in data:
            data[result[0]] = ([], [], [])

        data[result[0]][0].extend(result[1])
        data[result[0]][1].extend(result[2])
        data[result[0]][2].extend(result[3])

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def setup_fig(height=6, yl=YLABEL):
    fig = plt.figure(figsize=(13, height))
    plt.rc('font', size=22)  

    ax = plt.gca()

    for tick in ax.get_xticklabels():
        tick.set_rotation(45)
    
    plt.ylabel(yl)
    plt.xlabel(f"{PARAMETER_NAME} parameter")
    fig.tight_layout()

    ax.grid(True, axis="both", linestyle="--")

    return (plt, fig, ax)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        parameters = sorted([k for k in data.keys() if len(data[k])])

        plt, fig, ax = setup_fig()

        ax.errorbar(
            parameters,
            [np.mean(data[p][0]) for p in parameters],
            [np.std(data[p][0], ddof=1) for p in parameters],
            color="blue",
            marker='o',
            capsize=3,
            linewidth=1,
            label="Transmitted VAMs"
        )
        ax.axhline(
            NC_TX,
            color="purple",
            label="Transmitted VAMs w/o clustering"
        )
        ax.legend()
        save_plot(fig, "vam_rate_tx")

        plt, fig, ax = setup_fig(6, YLABEL2)
        ax.errorbar(
            parameters,
            [np.mean(data[p][1]) for p in parameters],
            [np.std(data[p][1], ddof=1) for p in parameters],
            color="blue",
            marker='o',
            capsize=3,
            linewidth=1,
            label="Received VAMs"
        )
        ax.plot(
            parameters,
            [np.mean(data[p][2]) for p in parameters],
            color="orange",
            label="Transmitted VAMs(aggregated)"
        )
        ax.set_ylim(50, 110)

        ax.axhline(
            NC_RX,
            color="purple",
            label="Received VAMs w/o clustering"
        )
        ax.legend(ncol=2)
        save_plot(fig, "vam_rate_rx")


if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
