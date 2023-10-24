#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_va_service_modules, aggregate_vam_recv
from common.plot_utils import save_plot

RESULTS = "../results"
STUDY_NAME = "RunStudyVeloDifferenceVadereGroupsDistrib4"
TEMP_FILE = f"{STUDY_NAME}_temp_perrcs.json"
YLABEL = "VAM Position Error\nin Meters"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 250
END_TIME = 500

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        modules = get_va_service_modules(cur, "vam_x", "World.vNode[%].middleware.VaService")

        result = []

        for mod in modules:
            print(mod)

            agg = aggregate_vam_recv(
                cur,
                mod[0]
            )

            for v in agg.values():

                if "pos_error_c" in v:

                    result.append(
                        (v["vam_cluster"], v["pos_error_c"])
                    )
                

        return result

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = []

    for result in results:
        data.extend(result)

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def setup_fig(height=6, yl=YLABEL):
    fig = plt.figure(figsize=(8, 4))
    plt.rc('font', size=22)  

    ax = plt.gca()
    
    plt.ylabel(yl)
    fig.tight_layout()

    ax.grid(True, axis="both", linestyle="--")

    return (plt, fig, ax)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        plt, fig, ax = setup_fig()

        ax.boxplot(
            [data["all"], data["cluster"]],
            showfliers=False,
            labels=[f"All VAMs", "Cluster VAMs"],
            medianprops=dict(color="black", linewidth=2)
        )

        save_plot(fig, "pos_error_ec")


if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
