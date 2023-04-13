#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_simple_num_create_cluster"
PARAMETER = "*.pNode[*].middleware.VaService.numCreateCluster"
PARAMETER_NAME = "numCreateCluster"
RECEIVER = "World.vNode[%].middleware.VaService"

TEMP_FILE = f"{STUDY_NAME}_csize_temp.json"
        
def cluster_size(cur, module):
    return cur.execute(f"""
        select vectorData.value, moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'vam_cluster'
        and moduleName LIKE '{module}'
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
                data[parameter_value] = []
                
            data[parameter_value].extend([d[0] for d in cluster_size(cur, RECEIVER)])
            
            print(f"{parameter_value}")
                
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        keys = sorted(data.keys())
        
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=16) 
        
        fig, ax = plt.subplots(1, 1, dpi=150, figsize=(10, 5))
        fig.subplots_adjust(bottom=0.25, left=0.09, top=0.95, right=0.99, hspace=0)
        
        
        ax.errorbar(
            keys,
            [np.mean([d for d in data[p] if d > 0]) for p in keys],
            [np.std([d for d in data[p] if d > 0]) for p in keys],
            color="black",
            ecolor="gray",
            # linestyle='None',
            marker='o',
            capsize=5,
        )
        
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)
            
        ax.grid(True, linestyle="--")
        
        fig.supylabel("\nMean VRU cluster size in meters\n+/- std. deviation", size=18, ha="center")
        fig.supxlabel(f"{PARAMETER_NAME} parameter", size=18)
        
        fig.savefig(f"fig/{PARAMETER_NAME}_csize.png")    
        fig.savefig(f"fig/{PARAMETER_NAME}_csize.svg")
        fig.savefig(f"fig/{PARAMETER_NAME}_csize.pdf")

        plt.show()

                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    