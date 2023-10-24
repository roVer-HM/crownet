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
STUDY_NAME_1 = "RunStudyNoClustering_MF"
STUDY_NAME_2 = "RunStudyEffectiveClustering_MF"

PARAMETER_NAME = "maxClusterVelocityDifference"
TEMP_FILE = f"mf_temp_rbs.json"
YLABEL = "Mean uplink\nresource block usage"

START_TIME = 230
END_TIME = 500

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()
        rbs = get_resource_blocks(cur, "World.gNodeB1.cellularNic.mac", s_to_st(START_TIME), s_to_st(END_TIME))
        return [np.mean([r[0] for r in rbs])]

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)

    results_nc = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME_1))
    results_c = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME_2))

    data = {
        "no_cluster": [],
        "cluster": []
    }

    for result in results_nc:
        data["no_cluster"].append(result)


    for result in results_c:
        data["cluster"].append(result)

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def setup_fig():
    fig = plt.figure(figsize=(6, 5))

    plt.rc('font', size=22)  

    ax = plt.gca()

    plt.ylabel(YLABEL)
    fig.tight_layout()

    ax.grid(True, axis="both", linestyle="--")

    return (plt, fig, ax)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)


        plt, fig, ax = setup_fig()

        wo_c = [d[0] for d in data["no_cluster"] if d[0]]
        w_c = [d[0] for d in data["cluster"] if d[0]]

        ax.boxplot(
            [wo_c, w_c],
            labels=["No\nclustering", "Clustering"],
            medianprops=dict(linestyle='-', linewidth=2, color='black'),
        )

        print(f"median without clustering: {np.median(wo_c)}")
        print(f"median with clustering: {np.median(w_c)}")

        save_plot(fig, "rb_mf")

if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
