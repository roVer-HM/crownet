#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_resource_blocks
from common.plot_utils import save_plot
from common.opp_utils import s_to_st

RESULTS = "../results"
STUDY_NAME = "RunStudyVeloDifferenceVadereGroupsDistrib4"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
TEMP_FILE = f"{STUDY_NAME}_temp_rbs.json"
YLABEL = "Mean Uplink\nResource Block Usage"

START_TIME = 200
END_TIME = 500

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()
        parameter_value = get_parameter_value(cur, PARAMETER)

        rbs = get_resource_blocks(cur, "World.gNodeB1.cellularNic.mac", s_to_st(START_TIME), s_to_st(END_TIME))

        return (parameter_value, [np.mean([r[0] for r in rbs])])

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = {}

    for result in results:
        if result[0] not in data:
            data[result[0]] = []

        data[result[0]].extend(result[1])

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def setup_fig():
    fig = plt.figure(figsize=(13, 6))

    plt.rc('font', size=22)  

    ax = plt.gca()

    for tick in ax.get_xticklabels():
        tick.set_rotation(45)
    
    plt.ylabel(YLABEL)
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
            [np.mean(data[p]) for p in parameters],
            [np.std(data[p]) for p in parameters],
            label="RB Usage with Clustering",
            color="black",
            marker='o',
            capsize=3,
            linewidth=1
        )

        ax.axhline(0.5943, color="purple", label="RB Usage w/o Clustering")
        ax.legend()


        save_plot(fig, "rb")

if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
