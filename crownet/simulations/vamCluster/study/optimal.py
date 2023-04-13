#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDIES = [
    "study_simple_no_cluster",
    "simple_optimal_ind_fix",
    "simple_optimal_sma_fix",
    "simple_optimal_fix",
    "simple_optimal_gro_fix"
]
LABELS = [
    "No clustering",
    "Individual",
    "Small groups",
    "Default groups",
    "Large groups"
]
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "\nMedian number of messages\nreceived by vehicle nodes\n+/- std. deviation"

def count_all_vams(cur, module):
    return cur.execute(f"""
        select count(vectorData.value), moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'vam_type'
        and moduleName LIKE '{module}'
        group by moduleName
        """).fetchall()

def position_error(cur, module):
    return cur.execute(f"""
        select vectorData.value, moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error'
        and moduleName LIKE '{module}'
        """).fetchall()

def position_error_cluster(cur, module):
    return cur.execute(f"""
        select vectorData.value, moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error_c'
        and moduleName LIKE '{module}'
        """).fetchall()

def get_vec_files(study):
    files = []
    for result in os.listdir(f"{RESULTS}/{study}"):
        files.extend(glob.glob(f"{RESULTS}/{study}/{result}/*.vec"))
    return files
            
  
def analyze(study):
    # {parameter: {vam_type: [counts]}}
    data = {"num": [], "poserr": [], "poserr_c": []}
    for vector_file in get_vec_files(study):
        with sqlite3.connect(vector_file) as db:
            cur = db.cursor()
            data["num"].extend([v[0] for v in count_all_vams(cur, RECEIVER)])
            data["poserr"].extend([v[0] for v in position_error(cur, RECEIVER)])
            data["poserr_c"].extend([v[0] for v in position_error_cluster(cur, RECEIVER)])
            
    with open(f"{study}_temp.json", "w") as f:
        json.dump(data, f)
    

def visualize():
    data = {}
    for study in STUDIES:
        with open(f"{study}_temp.json", "r") as f:
            data[study] = json.load(f)
        
    plt.rcParams.update({'font.size': 14})   
    plt.rc('axes', titlesize=14) 
    
    # Number
    fig, ax = plt.subplots(1, 1, dpi=150, figsize=(13, 5))
    fig.subplots_adjust(bottom=0.1, left=0.15, top=0.95, right=0.99, hspace=0)

    ax.axhline(5680, linestyle="dashed", label="No clustering", linewidth=1, color="black")
    
    ax.boxplot(
        [d["num"] for d in data.values()],
        showfliers=False,
        labels=LABELS,
        medianprops=dict(color="black", linewidth=2),
        widths=[0.2, 0.2, 0.2, 0.2, 0.2]
    )

    props = dict(boxstyle='round', facecolor='white', alpha=0.5)

    for i in range(5):
        median = np.median(list(data.values())[i]['num'])

        if i ==0:
            ax.text(i + 1, median - 1500, f"Median: {median}", fontsize=14,
                    horizontalalignment="center",
                    verticalalignment='top', bbox=props)
        else:
            ax.text(i + 1, median - 1500, f"Median: {median}\nReduction: {(1 - median / 5680) * 100:.{2}f} %", fontsize=14,
                    horizontalalignment="center",
                    verticalalignment='top', bbox=props)
    
    ax.grid(True, linestyle="--")
    ax.set_ylim(0, 7000)

    fig.supylabel("\n\nNumber of messages\nreceived by vehicle nodes", size=18, ha="center")
    
    fig.savefig(f"fig/optimal_all.png")    
    fig.savefig(f"fig/optimal_all.svg")
    fig.savefig(f"fig/optimal_all.pdf")

    plt.show()
    
    # Position error
    fig, ax = plt.subplots(1, 1, dpi=150, figsize=(9, 4))
    fig.subplots_adjust(bottom=0.1, left=0.15, top=0.95, right=0.99, hspace=0)

    ax.boxplot(
        [d["poserr_c"] for d in list(data.values())[1:]],
        showfliers=False,
        labels=LABELS[1:],
        medianprops=dict(color="black", linewidth=2),
        widths=[0.2, 0.2, 0.2, 0.2]
    )

    ax.grid(True, linestyle="--")

    fig.supylabel("\n\nPosition error of\nreceived cluster VAMs\nin meters", size=18, ha="center")
    
    fig.savefig(f"fig/optimal_poserr_c.png")    
    fig.savefig(f"fig/optimal_poserr_c.svg")
    fig.savefig(f"fig/optimal_poserr_c.pdf")

    plt.show()

    # # Position error cluster VAMs
    # fig, ax = plt.subplots(1, 1, dpi=150, figsize=(5, 5))
    # fig.subplots_adjust(bottom=0.25, left=0.2, top=0.8, right=0.99, hspace=0)
    
    # ax.boxplot(
    #     data["poserr_c"],
    #     showfliers=False,
    #     labels=["Cluster VAMs"],
    #     medianprops=dict(color="black", linewidth=2)
    # )
    
    # ax.set_title(f"Median: {np.median(data['poserr']):.{3}f}, Mean: {np.mean(data['poserr']):.{3}f}\nStd. deviation: {np.std(data['poserr']):.{3}f}")
    
    # ax.grid(True, linestyle="--")

    # fig.supylabel("\n\nPosition error of\nreceived VAM in meters", size=18, ha="center")
    
    # fig.savefig(f"fig/{STUDY_NAME}_poserr_c.png")    
    # fig.savefig(f"fig/{STUDY_NAME}_poserr_c.svg")
    # fig.savefig(f"fig/{STUDY_NAME}_poserr_c.pdf")

    # plt.show()

        
                
if __name__ == "__main__":
    for study in STUDIES:
        if not os.path.isfile(f"{study}_temp.json"):
            analyze(study)
    visualize()
    
    
    
    