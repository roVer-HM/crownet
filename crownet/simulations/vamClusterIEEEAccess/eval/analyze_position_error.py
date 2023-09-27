#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_position_error
from common.plot_utils import save_plot

RESULTS = "../results"
STUDY_NAME = "RunStudyClusterDistance"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterDistance"
PARAMETER_NAME = "Maximum Distance Between Cluster Leader and Members\nin Meters"
TEMP_FILE = f"{STUDY_NAME}_temp_perr.json"
YLABEL = "Position Error of Cluster VAMs\nin Meters"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 200
END_TIME = 500

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()
        parameter_value = get_parameter_value(cur, PARAMETER)

        errors = get_position_error(
            cur,
            "World.pNode[%].middleware.VaService",
            ST_CONV_FACTOR * START_TIME,
            ST_CONV_FACTOR * END_TIME
        )


        return (parameter_value, [e[0] for e in errors])

def analyze():
    pool = ThreadPoolExecutor(max_workers = 70)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = {}

    for result in results:
        if result[0] not in data:
            data[result[0]] = {
                "values": [],
                "hist_per_rep": []
            }

        bins = np.arange(0, 5, 0.05)

        hist = np.histogram(
            result[1],
            bins=len(bins),
            range=(bins.min(), bins.max())
        )[0]

        data[result[0]]["values"].extend(result[1])
        data[result[0]]["hist_per_rep"].append([int(h) for h in hist])


    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def setup_fig(height=6, yl=YLABEL):
    fig = plt.figure(figsize=(13, height))
    plt.rc('font', size=22)  

    ax = plt.gca()

    for tick in ax.get_xticklabels():
        tick.set_rotation(45)
    
    plt.ylabel(yl)
    plt.xlabel(f"{PARAMETER_NAME}")
    fig.tight_layout()

    ax.grid(True, axis="both", linestyle="--")

    return (plt, fig, ax)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        parameters = sorted([k for k in data.keys() if len(data[k]) and float(k) <= 4])

        plt, fig, ax = setup_fig()

        ax.boxplot(
            [data[p]["values"] for p in parameters],
            showfliers=False,
            labels=parameters,
            medianprops=dict(color="black", linewidth=2)
        )

        save_plot(fig, "pos_error")

        plt, fig, ax = setup_fig()

        for d in ['1', '1.5', '2', '2.5', '3', '3.5', '4']:
            dat = np.array(data[d]["hist_per_rep"])
            avg_hist = np.mean(dat, axis=0)
            std = dat.std(axis=0)

            ax.errorbar(
                np.arange(0, 5, 0.05), 
                avg_hist,
                std,
                label=f"{d} m"
            )

        plt.subplots_adjust(left=0.13, right=0.98, top=0.95, bottom=0.15)
        ax.legend(ncols=2, title="maxClusterDistance")
        plt.ylabel("Number of analyzed nodes")
        plt.xlabel(f"Position error in meters")

        save_plot(fig, "pos_error_hist")


if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
