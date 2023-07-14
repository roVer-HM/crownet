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
STUDY_NAME = "RunStudyNoClustering"
TEMP_FILE = f"{STUDY_NAME}_temp_rbs.json"

START_TIME = 200
END_TIME = 500

def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        rbs = get_resource_blocks(cur, "World.gNodeB1.cellularNic.mac", s_to_st(START_TIME), s_to_st(END_TIME))

        return [r[0] for r in rbs]

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

    data = []

    for result in results:
        data.extend(result)

    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)


def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)

        print(np.mean(data))

if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
