#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, count_all_vams
from common.plot_utils import save_plot

YLABEL = "VAMs per second"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 220
END_TIME = 500
NUM_VRUS = 139
NUM_VEH = 8

def visualize():

    no_clustering = []
    effective_clustering = []

    with open("RunStudyNoClustering_MF_temp_vamsps.json", "r") as f:
        no_clustering = json.load(f)

    with open("RunStudyEffectiveClustering_MF_temp_vamsps.json", "r") as f:
        effective_clustering = json.load(f)

    fig = plt.figure()
    plt.rc('font', size=20)  

    fig, ax = plt.subplots(1, 1, figsize=(6, 4), gridspec_kw=dict(left=0.2, right=0.9,bottom=0.2, top=0.95))

    ax.grid(True, axis="both", linestyle="--")

    fig.text(0.06, 0.5,YLABEL, ha='center', va='center', rotation='vertical')

    mean_no_c = np.mean(no_clustering[0])
    mean_c = np.mean(effective_clustering[0])

    bp1 = ax.boxplot(
        [no_clustering[0], effective_clustering[0]], 
        medianprops=dict(linestyle='-', linewidth=2, color='black'),
        labels=[f"Transmitted\nno clustering\nmean = {mean_no_c:.2f}", f"Transmitted\nclustering\nmean = {mean_c:.2f}"]
    )

    ax.set_ylim(1.0, 1.5)

    save_plot(fig, "cmp_clustering_MF")


if __name__ == '__main__':
    visualize()
    
