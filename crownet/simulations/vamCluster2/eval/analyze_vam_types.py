import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_vru_pos, get_va_service_modules, count_vams
from common.plot_utils import save_plot
from common.opp_utils import s_to_st
import utm
import math
import matplotlib.ticker as mtick

START_TIME = 200
END_TIME = 500
RESULTS = "../results"
STUDY_NAME = "RunStudyEffectiveClustering"
TEMP_FILE = f"{STUDY_NAME}_type.json"

VAMS = {
    1 : "Individual",
    2 : "Cluster",
    3 : "Join",
    5 : "Breakup",
    4 : "Leave",
    11 : "Cluster\nw/o Members",
}

NUM_VRUS = 71

def analyze_vector(vector_file):
    print(vector_file)

    result = {}

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        for vam_type in VAMS.keys():
            """
            Count number of transmitted VAMs for all pedestrian nodes
            """
            number = count_vams(
                cur,
                vam_type,
                "World.pNode[%].middleware.VaService",
                s_to_st(START_TIME),
                s_to_st(END_TIME)
            )

            number = number  / NUM_VRUS
            number = number / (END_TIME - START_TIME)

            result[vam_type] = number

    return result


def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    with open(TEMP_FILE, "w") as f:
        json.dump(list(results), f)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        results = {}

        for vam_type in VAMS.keys():
            results[vam_type] = [d[str(vam_type)] for d in data]

        fig = plt.figure(figsize=(13, 5))
        plt.rc('font', size=22)  

        ax = plt.gca()
        fig.tight_layout()

        ax.grid(True, axis="both", linestyle="--")

        plt.subplots_adjust(left=0.1, right=0.97, top=0.95, bottom=0.25)
        ax.set_ylabel("VAMs per Second per VRU")
        ax.set_xlabel("VAM Type")

        ax.boxplot(
            [v for v in results.values()],
            showfliers=False,
            labels=[v for v in VAMS.values()],
            medianprops=dict(color="black", linewidth=2)
        )

        save_plot(plt, "vam_types_effective")

if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
