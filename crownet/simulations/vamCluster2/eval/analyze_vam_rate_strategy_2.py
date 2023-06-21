#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
import collections

RESULTS = "../results"
STUDY_NAME = "RunStudyVeloDifferenceVadereGroups"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
TEMP_FILE = f"{STUDY_NAME}_temp_vamsps.json"
YLABEL = "VAMs / second"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

START_TIME = 70
END_TIME = 100
NUM_VRUS = 70

def get_parameter(cursor, parameter):
    cursor.execute(f"SELECT configValue FROM runConfig WHERE configKey = '{parameter}'")
    return cursor.fetchone()[0]

def get_vec_files():
    files = []
    for result in os.listdir(f"{RESULTS}/{STUDY_NAME}"):
        files.extend(glob.glob(f"{RESULTS}/{STUDY_NAME}/{result}/*.vec"))
    return files

def count_all_vams(cur, start, end):
    data = cur.execute(f"""
        select count(vectorData.value) from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'vam_sent_type'
        and moduleName LIKE 'World.pNode[%].middleware.VaService'
        and vectorData.simtimeRaw > {start}
        and vectorData.simtimeRaw < {end}
        """).fetchall()

    if len(data):
        return data[0][0]
    else:
        return None

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()
        parameter_value = get_parameter(cur, PARAMETER)

        results = []

        vams = count_all_vams(
            cur,
            ST_CONV_FACTOR * START_TIME,
            ST_CONV_FACTOR * END_TIME
        )

        if vams:
            vams = vams / (END_TIME - START_TIME) # VAMs / Second
            vams = vams / NUM_VRUS # Vams / VRU
            print(vams)
            results.append(vams)

        return (parameter_value, results)

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files())

    data = {}

    for result in results:
        if result[0] not in data:
            data[result[0]] = []

        data[result[0]].extend(result[1])

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        fig = plt.figure(figsize=(10, 4))
        ax = plt.gca()

        parameters = sorted([k for k in data.keys() if len(data[k])])
        ax.grid(True, axis="both", linestyle="--")
        
        ax.errorbar(
            parameters,
            [np.mean(data[p]) for p in parameters],
            [np.std(data[p], ddof=1) for p in parameters],
            color="black",
            ecolor="gray",
            # linestyle='None',
            marker='o',
            capsize=5,
            label="Clustering"
        )

        ax.legend()
        
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)
        
        plt.ylabel(YLABEL, size=18)
        plt.xlabel(f"{PARAMETER_NAME} parameter", size=18)
        fig.tight_layout()
        
        fig.savefig(f"fig/{PARAMETER_NAME}_all.png")    
        fig.savefig(f"fig/{PARAMETER_NAME}_all.svg")
        fig.savefig(f"fig/{PARAMETER_NAME}_all.pdf")
        plt.show()


if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
