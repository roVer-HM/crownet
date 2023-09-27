#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = [
    "study_simple_velocity_difference_exceed_standard_g_ind_fix",
    "study_simple_velocity_difference_exceed_standard_g_sma_fix",
    "study_simple_velocity_difference_exceed_standard_fix",
    "study_simple_velocity_difference_exceed_standard_g_gro_fix",
]
LABELS = [
    "Individual",
    "Small groups",
    "Default groups",
    "Large groups",
    "Demonstration"
]
COLORS = ["black", "blue", "purple", "orange", "red"]

PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "\nMedian number of\nmessages received by\nvehicle nodes\n+/- std. deviation"
VAM_TYPES = {
    1: "Individual VAM",
    11: "Cluster VAM\n(cardinality = 1)",
    2: "Cluster VAM\n(cardinality > 1)",
    3: "Individual VAM\njoin notification",
    4: "Individual VAM\nleave notification",
    5: "Individual VAM\ncluster breakup",
    10: "Individual VAM\nleader lost"
}
VAM_TYPE_COLORS = {
    1: "blue",
    11: "orange",
    2: "orange",
    3: "green",
    4: "green",
    5: "brown",
    10: "brown"
}
TEMP_FILE = [f"{s}_temp.json" for s in STUDY_NAME]


def count_vams(cur, module, type):
    return cur.execute(f"""
        select count(vectorData.value), moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'vam_type'
        and moduleName LIKE '{module}'
        and vectorData.value = {type}
        group by moduleName
        """).fetchall()
        
def count_all_vams(cur, module):
    return cur.execute(f"""
        select count(vectorData.value), moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'vam_type'
        and moduleName LIKE '{module}'
        group by moduleName
        """).fetchall()

def get_parameter(cursor, parameter):
    cursor.execute(f"SELECT configValue FROM runConfig WHERE configKey = '{parameter}'")
    return cursor.fetchone()[0]

def get_vec_files(study):
    files = []
    for result in os.listdir(f"{RESULTS}/{study}"):
        files.extend(glob.glob(f"{RESULTS}/{study}/{result}/*.vec"))
    return files
            
  
def analyze(study, temp):
    # {parameter: {vam_type: [counts]}}
    data = {}
    for vector_file in get_vec_files(study):
        with sqlite3.connect(vector_file) as db:
            cur = db.cursor()
            parameter_value = get_parameter(cur, PARAMETER)
            
            if parameter_value not in data:
                data[parameter_value] = {}
                
            if "all" not in data[parameter_value]:
                data[parameter_value]["all"] = []
                
            data[parameter_value]["all"].extend([v[0] for v in count_all_vams(cur, RECEIVER)])
                            
    with open(temp, "w") as f:
        json.dump(data, f)
    

def visualize():
    plt.rcParams.update({'font.size': 14})   
    plt.rc('axes', titlesize=16)   
    
    fig = plt.figure(figsize=(10, 4))
    ax = plt.gca()
    
    ax.grid(True, axis="both", linestyle="--")
    ax.set_ylim(3500, 6000)

    for i, t in enumerate(TEMP_FILE):
        with open(t, "r") as f:
            data = json.load(f)
            
            by_param = {}
            
            for param in data.keys():
                by_param[param] = data[param]["all"]

            parameters = sorted(by_param.keys())
                        
            ax.errorbar(
                parameters,
                [np.median(by_param[p]) for p in parameters],
                [np.std(by_param[p]) for p in parameters],
                ecolor="gray",
                # linestyle='None',
                color=COLORS[i],
                label=LABELS[i],
                capsize=5,
            )
        
    for tick in ax.get_xticklabels():
        tick.set_rotation(45)
    
    plt.ylabel(YLABEL, size=18)
    plt.xlabel(f"{PARAMETER_NAME} parameter", size=18)
    plt.legend()
    fig.tight_layout()
    
    fig.savefig(f"fig/{PARAMETER_NAME}_mult.png")    
    fig.savefig(f"fig/{PARAMETER_NAME}_mult.svg")
    fig.savefig(f"fig/{PARAMETER_NAME}_mult.pdf")
    plt.show()
        
                
if __name__ == "__main__":
    for i in range(len(TEMP_FILE)):
        if not os.path.isfile(TEMP_FILE[i]):
            analyze(STUDY_NAME[i], TEMP_FILE[i])

    visualize()
    
    
    
    