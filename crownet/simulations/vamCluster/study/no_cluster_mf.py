#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np

RESULTS = "../results"
STUDY_NAME = "study_mf_velocity_no_cluster"
RECEIVER = "World.vNode[%].middleware.VaService"
YLABEL = "\nMedian number of messages\nreceived by vehicle nodes\n+/- std. deviation"
TEMP_FILE = f"{STUDY_NAME}_temp.json"

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

def get_vec_files():
    files = []
    for result in os.listdir(f"{RESULTS}/{STUDY_NAME}"):
        files.extend(glob.glob(f"{RESULTS}/{STUDY_NAME}/{result}/*.vec"))
    return files
            
  
def analyze():
    # {parameter: {vam_type: [counts]}}
    data = {"num": [], "poserr": []}
    for vector_file in get_vec_files():
        with sqlite3.connect(vector_file) as db:
            cur = db.cursor()
            print(count_all_vams(cur, RECEIVER))
            data["num"].extend([v[0] for v in count_all_vams(cur, RECEIVER)])
            data["poserr"].extend([v[0] for v in position_error(cur, RECEIVER)])
            
    with open(TEMP_FILE, "w") as f:
        json.dump(data, f)
    

def visualize():
    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        plt.rcParams.update({'font.size': 14})   
        plt.rc('axes', titlesize=14) 
        
        # Number
        fig, ax = plt.subplots(1, 1, dpi=150, figsize=(5, 5))
        fig.subplots_adjust(bottom=0.25, left=0.3, top=0.8, right=0.99, hspace=0)
        
        ax.boxplot(
            data["num"],
            showfliers=False,
            labels=["Individual VAM"],
            medianprops=dict(color="black", linewidth=2)
        )
        
        ax.set_title(f"Median: {np.median(data['num'])}, Mean: {np.mean(data['num'])}\nStd. deviation: {np.std(data['num']):.{2}f}")
        
        ax.grid(True, linestyle="--")
        ax.set_ylim(0, 12000)

        fig.supylabel("\n\nNumber of messages\nreceived by vehicle nodes", size=18, ha="center")
        
        fig.savefig(f"fig/{STUDY_NAME}_all.png")    
        fig.savefig(f"fig/{STUDY_NAME}_all.svg")
        fig.savefig(f"fig/{STUDY_NAME}_all.pdf")

        plt.show()
        
        # Position error
        fig, ax = plt.subplots(1, 1, dpi=150, figsize=(5, 5))
        fig.subplots_adjust(bottom=0.25, left=0.3, top=0.8, right=0.99, hspace=0)
        
        ax.boxplot(
            data["poserr"],
            showfliers=False,
            labels=["Individual VAM"],
            medianprops=dict(color="black", linewidth=2)
        )
        
        ax.set_title(f"Median: {np.median(data['poserr']):.{3}f}, Mean: {np.mean(data['poserr']):.{3}f}\nStd. deviation: {np.std(data['poserr']):.{3}f}")
        
        ax.grid(True, linestyle="--")

        fig.supylabel("\n\nPosition error of\nreceived VAM in meters", size=18, ha="center")
        
        fig.savefig(f"fig/{STUDY_NAME}_poserr.png")    
        fig.savefig(f"fig/{STUDY_NAME}_poserr.svg")
        fig.savefig(f"fig/{STUDY_NAME}_poserr.pdf")

        plt.show()

        
                
if __name__ == "__main__":
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
    
    
    