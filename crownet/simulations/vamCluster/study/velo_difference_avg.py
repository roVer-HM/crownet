#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_simple_velocity_difference_avg"
PARAMETER1 = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER2 = "*.pNode[*].middleware.VaService.averageHeadingBufferSize"
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "\nMedian number of \n messages received by \nvehicle nodes\n+/- std. deviation"
VAM_TYPES = {
    1: "Individual VAM",
    11: "Cluster VAM (cardinality = 1)",
    2: "Cluster VAM (cardinality > 1)",
    3: "Individual VAM join notification",
    4: "Individual VAM leave notification",
    5: "Individual VAM cluster breakup",
    10: "Individual VAM leader lost"
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
TEMP_FILE = f"{STUDY_NAME}_temp.json"
LABELS = {
    "10": "Moving average (n=10)",
    "20": "Moving average (n=20)"
}
COLORS = {
    "10": "purple",
    "20": "orange"
}

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

def get_vec_files():
    files = []
    for result in os.listdir(f"{RESULTS}/{STUDY_NAME}"):
        files.extend(glob.glob(f"{RESULTS}/{STUDY_NAME}/{result}/*.vec"))
    return files
            
  
def analyze():
    # {parameter: {vam_type: [counts]}}
    data = {}
    for vector_file in get_vec_files():
        with sqlite3.connect(vector_file) as db:
            cur = db.cursor()
            parameter_value1 = get_parameter(cur, PARAMETER1)
            parameter_value2 = get_parameter(cur, PARAMETER2)

            parameter_value = f"{parameter_value1}_{parameter_value2}"
            
            if parameter_value not in data:
                data[parameter_value] = {}
                
            if "all" not in data[parameter_value]:
                data[parameter_value]["all"] = []
                
            data[parameter_value]["all"].extend([v[0] for v in count_all_vams(cur, RECEIVER)])
                
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        by_param = {}
        
        for param in data.keys():
            for type in data[param].keys():
                if type == "all":
                    param1 = param.split("_")[0]
                    param2 = param.split("_")[1]

                    if not param2 in by_param:
                        by_param[param2] = {}

                    by_param[param2][param1] = data[param][type]


                    
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=16)   
        
        
        fig = plt.figure(figsize=(10, 4))
        ax = plt.gca()
        
        ax.grid(True, axis="both", linestyle="--")

        ax.axhline(5680, linestyle="dashed", label="No clustering", linewidth=2, color="blue")
        ax.axhline(5680 + 432, color="blue", linestyle="dotted")
        ax.axhline(5680 - 432, color="blue", linestyle="dotted")

        for param in list(by_param)[0:1]:
            parameters = sorted(by_param[param].keys())

            ax.errorbar(
                parameters,
                [np.median(by_param[param][p]) for p in parameters],
                [np.std(by_param[param][p]) for p in parameters],
                ecolor="gray",
                # linestyle='None',
                marker='o',
                capsize=5,
                label=LABELS[param],
                color=COLORS[param]
            )
        
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)

        # Plot no avg
        with open("study_simple_velocity_difference_exceed_standard_fix_temp.json", "r") as f2:
            data2 = json.load(f2)
            
            by_param2 = {}

            for param in data2.keys():
                for type in data2[param].keys():
                    if type == "all":
                        by_param2[param] = data2[param][type]

            parameters2 = sorted(by_param2.keys())

            ax.errorbar(
                parameters2,
                [np.median(by_param2[p]) for p in parameters2],
                [np.std(by_param2[p]) for p in parameters2],
                color="black",
                ecolor="gray",
                # linestyle='None',
                marker='o',
                capsize=5,
                label="Raw value"
            )

        ax.legend()

        ax.set_ylim(2000, 6500)
        
        plt.ylabel(YLABEL, size=18)
        plt.xlabel(f"maxClusterVelocityDifference parameter", size=18)
        fig.tight_layout()
        
        fig.savefig(f"fig/avg_all.png")    
        fig.savefig(f"fig/avg_all.svg")
        fig.savefig(f"fig/avg_all.pdf")
        plt.show()
        
                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    