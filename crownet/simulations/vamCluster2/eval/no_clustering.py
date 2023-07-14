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
STUDY_NAME = "RunStudyNoClustering"
TEMP_FILE = f"{STUDY_NAME}_temp_vamsps.json"
YLABEL = "VAMs per Second"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 200
END_TIME = 500
NUM_VRUS = 71
NUM_VEH = 8

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        results_sent = []
        results_received = []

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

        return (results_sent, results_received)

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = [[], []]

    for result in results:
        data[0].extend(result[0])
        data[1].extend(result[1])

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)


def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        fig = plt.figure()
        plt.rc('font', size=22)  

        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6), gridspec_kw=dict(left=0.15, right=0.9,bottom=0.2, top=0.9))

        ax1.grid(True, axis="both", linestyle="--")
        ax2.grid(True, axis="both", linestyle="--")

        fig.text(0.06, 0.5,YLABEL, ha='center', va='center', rotation='vertical')

        bp1 = ax1.boxplot(
            [data[0]], 
            medianprops=dict(linestyle='-', linewidth=2, color='black'),
            labels=[f"Sent by pNode\n(mean = {np.mean(data[0]):.2f})"]
        )

        ax2.boxplot(
            data[1], 
            medianprops=dict(linestyle='-', linewidth=2, color='black'),
            labels=[f"Received by vNode\n(mean = {np.mean(data[1]):.2f})"]
        )

        ax2.set_ylim(0, 85)
        ax1.set_ylim(0, 2)

        save_plot(fig, "no_clustering")


if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
