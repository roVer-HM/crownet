#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import matplotlib.pylab as pl
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_mf_velocity_difference_separated_fix"
PARAMETER1 = "*.pNode[*].middleware.VaService.maxClusterSpeedDifference"
PARAMETER2 = "*.pNode[*].middleware.VaService.maxClusterAngleDifference"
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "\nMedian number of messages\nreceived by vehicle nodes\n+/- std. deviation"
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
TEMP_FILE = f"{STUDY_NAME}_temp.json"


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
            print(f"{parameter_value}")
            
                
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        by_param = {}
        
        for param in sorted(data.keys()):
            for type in data[param].keys():
                if type == "all":
                    param1 = param.split("_")[0]
                    param2 = param.split("_")[1]

                    if not param2 in by_param:
                        by_param[param2] = {}

                    by_param[param2][param1] = data[param][type]

                    
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=16)   


        fig = plt.figure(figsize=(10, 6))
        ax = plt.gca()
        
        ax.grid(True, axis="both", linestyle="--")

        colors = pl.cm.jet(np.linspace(0,1,10))

        for i, param in enumerate(by_param):
            parameters = sorted(by_param[param].keys())

            ax.errorbar(
                parameters,
                [np.median(by_param[param][p]) for p in parameters],
                [np.std(by_param[param][p]) for p in parameters],
                ecolor=colors[i],
                # linestyle='None',
                marker='o',
                capsize=5,
                linewidth=1.5,
                color=colors[i],
                label=f"{param}"
            )
        
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)

        box = ax.get_position()
        ax.set_position([box.x0, 0, box.width, 0.28])
        ax.set_ylim(3000, 6500)

        ax.legend(title="Maximum heading difference", ncol=5, bbox_to_anchor=(0.5, -0.4), loc="upper center")
        
        plt.ylabel(YLABEL, size=18)
        plt.xlabel(f"Maximum speed difference", size=18)
        fig.tight_layout()
        
        fig.savefig(f"fig/sep_all.png")    
        fig.savefig(f"fig/sep_all.svg")
        fig.savefig(f"fig/sep_all.pdf")
        plt.show()
        
                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    