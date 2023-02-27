#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_simple_velocity_difference_exceed_standard"
PARAMETER = "*.pNode[*].middleware.VaService.maxClusterVelocityDifference"
PARAMETER_NAME = "maxClusterVelocityDifference"
RECEIVER = "World.vNode[%].middleware.VaService"

TEMP_FILE = f"{STUDY_NAME}_poserr_temp.json"


def position_error_cluster(cur, module):
    return cur.execute(f"""
        select vectorData.value, moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error_c'
        and moduleName LIKE '{RECEIVER}'
        """).fetchall()
        
def position_error(cur, module):
    return cur.execute(f"""
        select vectorData.value, moduleName from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error'
        and moduleName LIKE '{RECEIVER}'
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
                data[parameter_value] = {"norm": [], "clust": []}
                
            perrc = [p[0] for p in position_error_cluster(cur, RECEIVER)]
            data[parameter_value]["clust"].extend(perrc)
            
            perr = [p[0] for p in position_error(cur, RECEIVER)]
            data[parameter_value]["norm"].extend(perr)
            
            print(f"{parameter_value} {len(perr)} {len(perrc)}")
                
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        keys = sorted(data.keys())
        
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=16) 
        
        fig, axs = plt.subplots(1, 2, dpi=150, figsize=(15, 5))
        fig.subplots_adjust(bottom=0.25, left=0.1, top=0.8, right=0.99, hspace=0)
        
        axs[0].boxplot(
            [data[k]["clust"] for k in keys],
            showfliers=False,
            labels=keys,
            medianprops=dict(color="black", linewidth=2)
        )
        
        axs[1].boxplot(
            [data[k]["clust"] + data[k]["norm"] for k in keys],
            showfliers=False,
            labels=keys,
            medianprops=dict(color="black", linewidth=2)
        )
        
        # plt.setp(axs[1].get_yticklabels(), visible=False)
        axs[0].set_ylim(0, 6)
        axs[1].set_ylim(0, 6)
        axs[0].grid(True, linestyle="--")
        axs[1].grid(True, linestyle="--")
        
        axs[0].set_title("Cluster VAMs")
        axs[1].set_title("All VAMs")
        
        for tick in axs[0].get_xticklabels():
            tick.set_rotation(45)
        
        for tick in axs[1].get_xticklabels():
            tick.set_rotation(45)
        
        fig.supylabel("\nPosition error of received VAMs\nin meters", size=18, ha="center")
        fig.supxlabel(f"{PARAMETER_NAME} parameter", size=18)
        
        fig.savefig(f"fig/{PARAMETER_NAME}_perr.png")    
        fig.savefig(f"fig/{PARAMETER_NAME}_perr.svg")
        fig.savefig(f"fig/{PARAMETER_NAME}_perr.pdf")

        plt.show()

                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    