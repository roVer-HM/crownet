#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
import collections

ST_CONV_FACTOR = 10**12
RESULTS = "../results"
STUDY_NAME = "RunStudyVeloDifferenceVadereGroups"

def get_va_service_modules(cur):
    modules = cur.execute(f"""
        select moduleName, startSimtimeRaw, endSimtimeRaw, vectorCount  from vector
        where moduleName LIKE 'World.pNode[%].middleware.VaService'
        and vectorName = 'vam_sent_type'
        """).fetchall()
    return [(m[0], m[1] / ST_CONV_FACTOR, m[2] / ST_CONV_FACTOR, m[3]) for m in modules if m[0] and m[1] and m[2] and m[3]]

def get_vec_files():
    files = []
    for result in os.listdir(f"{RESULTS}/{STUDY_NAME}"):
        files.extend(glob.glob(f"{RESULTS}/{STUDY_NAME}/{result}/*.vec"))
    return files

def get_va_service_modules(cur):
    modules = cur.execute(f"""
        select moduleName, startSimtimeRaw, endSimtimeRaw, vectorCount  from vector
        where moduleName LIKE 'World.pNode[%].middleware.VaService'
        and vectorName = 'vam_sent_type'
        """).fetchall()
    return [(m[0], m[1] / ST_CONV_FACTOR, m[2] / ST_CONV_FACTOR, m[3]) for m in modules if m[0] and m[1] and m[2] and m[3]]


def analyze_vector(f):
    active_vrus = np.full(300, 0)
    with sqlite3.connect(f) as db:
        cur = db.cursor()
        modules = get_va_service_modules(cur)

        for module in modules:
            start_time = module[1]
            end_time = module[2]

            for i in range(int(start_time), int(end_time)):
                active_vrus[i] += 1

    return active_vrus

if __name__ == '__main__':
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, get_vec_files())

    avg_vrus = np.array(list(results)).mean(axis=0)

    fig = plt.figure(figsize=(10, 4))
    ax = plt.gca()

    ax.grid(True, axis="both", linestyle="--")
    ax.plot(np.arange(0, 300), avg_vrus)

    plt.show()

    fig.savefig(f"fig/vru_over_time.png")    
    fig.savefig(f"fig/vru_over_time.svg")
    fig.savefig(f"fig/vru_over_time.pdf")

