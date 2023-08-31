#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_va_service_modules
from common.opp_utils import st_to_s

ST_CONV_FACTOR = 10**12
RESULTS = "../results"
STUDY_NAME = "RunStudyNoClustering_MF"

INLCUDE_VEH = False

T_END = 600
STABLE_START = 230
STABLE_END = 500

VRU_COLOR="blue"
VEH_COLOR="orange"

def analyze_vector(f):
    active_vrus = np.full(T_END, 0)
    active_vehs = np.full(T_END, 0)

    with sqlite3.connect(f) as db:
        cur = db.cursor()
        p_modules = get_va_service_modules(cur, 'vam_sent_type', 'World.pNode[%].middleware.VaService')

        for module in p_modules:
            start_time = st_to_s(module[1])
            end_time = st_to_s(module[2])

            for i in range(int(start_time), int(end_time)):
                active_vrus[i] += 1

        v_modules = get_va_service_modules(cur, 'vam_type', 'World.vNode[%].middleware.VaService')

        for module in v_modules:
            start_time = st_to_s(module[1])
            end_time = st_to_s(module[2])

            for i in range(int(start_time), int(end_time)):
                active_vehs[i] += 1

    return (active_vrus, active_vehs)

if __name__ == '__main__':
    pool = ThreadPoolExecutor(max_workers = 15)
    results = list(pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME)))

    avg_vrus = np.array([r[0] for r in results]).mean(axis=0)
    std_vrus = np.array([r[0] for r in results]).std(axis=0)

    avg_vehs = np.array([r[1] for r in results]).mean(axis=0)

    font = {'size': 22}
    plt.rc('font', **font)   

    fig = plt.figure(figsize=(16, 7))
    ax = plt.gca()

    ax.set_ylim(0,  300)
    ax.set_xlim(0, T_END)
    ax.grid(True, axis="both", linestyle="--")
    ax.plot(np.arange(0, T_END), avg_vrus, label="Pedestrian", color=VRU_COLOR)
    ax.fill_between(np.arange(0, T_END), avg_vrus - std_vrus, avg_vrus + std_vrus, color=VRU_COLOR, alpha=0.3)

    if INLCUDE_VEH:
        ax.plot(np.arange(0, T_END), avg_vehs, label="Vehicle", color=VEH_COLOR)

    plt.axvspan(STABLE_START, STABLE_END, color='gray', alpha=0.2, label=f"Steady state ({STABLE_END - STABLE_START}s)")

    vru_mean = np.mean(avg_vrus[STABLE_START:STABLE_END])
    vru_std = np.std(avg_vrus[STABLE_START:STABLE_END])
    ax.axhline(vru_mean, linestyle="dashed", color=VRU_COLOR, label=f"Pedestrian (mean) = {vru_mean:.2f}, std. dev = {vru_std:.2f}")

    veh_mean = np.mean(avg_vehs[STABLE_START:STABLE_END])
    veh_std = np.std(avg_vehs[STABLE_START:STABLE_END])

    if INLCUDE_VEH:
        ax.axhline(veh_mean, linestyle="dashed", color=VEH_COLOR, label=f"Vehicle (mean) = {veh_mean:.2f}, std. dev = {veh_std:.2f}")

    plt.axvspan(0, STABLE_START, facecolor="none", hatch="//", edgecolor="lightgray", label="Build-up / Fade-out")
    plt.axvspan(STABLE_END, T_END, facecolor="none", hatch="//", edgecolor="lightgray")

    legend = ax.legend(ncol=2, loc="upper left")
    legend.get_frame().set_alpha(None)

    plt.ylabel("Number of Nodes in Simulation", size=24)
    plt.xlabel(f"Simulation Time (s)", size=24)
    fig.tight_layout()

    plt.show()

    fig.savefig(f"fig/vru_over_time_mf.png")    
    fig.savefig(f"fig/vru_over_time_mf.svg")
    fig.savefig(f"fig/vru_over_time_mf.pdf")

