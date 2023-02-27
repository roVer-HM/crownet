#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_simple_velocity_difference"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "Number of messages received\nby vehicle nodes"
VAM_TYPES = {
    1: "Individual VAM",
    11: "Cluster VAM\n(cardinality = 1)",
    2: "Cluster VAM\n(cardinality > 1)",
    3: "Individual VAM\njoin notification",
    4: "Individual VAM\nleave notification",
    5: "Individual VAM\nCluster breakup",
    10: "Individual VAM\nleader lost"
}
VAM_TYPE_COLORS = ["blue", "orange", "orange", "green", "green", "brown", "brown"]
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
            parameter_value = get_parameter(cur, PARAMETER)
            
            if parameter_value not in data:
                data[parameter_value] = {}
                
            if "all" not in data[parameter_value]:
                data[parameter_value]["all"] = []
                
            data[parameter_value]["all"].extend([v[0] for v in count_all_vams(cur, RECEIVER)])
            
            for type in VAM_TYPES.keys():
                print(f"{parameter_value}, {type}")
                
                vams = [v[0] for v in count_vams(cur, RECEIVER, type)]
                
                if type not in data[parameter_value]:
                    data[parameter_value][type] = []
                    
                data[parameter_value][type].extend(vams)
                
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    
    
def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        by_vam_type = {}
        by_param = {}
        
        for param in data.keys():
            for type in data[param].keys():
                if type != "all":
                    if type not in by_vam_type:
                        by_vam_type[type] = {}
                    if param not in by_vam_type[type]:
                        by_vam_type[type][param] = []
                    by_vam_type[type][param] = data[param][type]
                else:
                    by_param[param] = data[param][type]
                    
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=16)   
          
        
        fig, axs = plt.subplots(1, 7, dpi=100, figsize=(16, 4))
        fig.subplots_adjust(bottom=0.25, left=0.08, top=0.8, right=0.99, hspace=0)
        

        for i, type in enumerate(VAM_TYPES.keys()):
            axs[i].set_title(f"{VAM_TYPES[type]}")
            
            parameters = sorted(by_vam_type[f"{type}"].keys())
            
            axs[i].errorbar(
                parameters,
                [np.median(by_vam_type[f"{type}"][p]) for p in parameters],
                [np.std(by_vam_type[f"{type}"][p]) for p in parameters],
                color=VAM_TYPE_COLORS[i],
                linestyle='None',
                marker='o',
                capsize=5,
                markersize=5
            )
            
            axs[i].grid(True, axis="both", linestyle="--")
            axs[i].set_ylim(0, 3000)
            for tick in axs[i].get_xticklabels():
                tick.set_rotation(45)
            
            if i > 0:
                plt.setp(axs[i].get_yticklabels(), visible=False)
            
         
        fig.supylabel(YLABEL, size=18, ha="center")
        fig.supxlabel(f"{PARAMETER_NAME}", size=18)
        
        fig.savefig(f"fig/{PARAMETER_NAME}.png")    
        fig.savefig(f"fig/{PARAMETER_NAME}.svg")
        fig.savefig(f"fig/{PARAMETER_NAME}.pdf")
        plt.show()
        
        ax = plt.gca()

        parameters = sorted(by_param.keys())
        
        ax.set_ylim(0, 7000)
        ax.grid(True, axis="both", linestyle="--")
        
        ax.errorbar(
            parameters,
            [np.median(by_param[p]) for p in parameters],
            [np.std(by_param[p]) for p in parameters],
            color="black",
            ecolor="gray",
            # linestyle='None',
            marker='o',
            capsize=5,
        )
        
        plt.ylabel(YLABEL, size=18)
        plt.xlabel(f"{PARAMETER_NAME}", size=18)
        plt.tight_layout()
        
        plt.savefig(f"fig/{PARAMETER_NAME}_all.png")    
        plt.savefig(f"fig/{PARAMETER_NAME}_all.svg")
        plt.savefig(f"fig/{PARAMETER_NAME}_all.pdf")
        plt.show()
        
                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    