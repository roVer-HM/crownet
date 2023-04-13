#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
import utm

RESULTS = "../results"
STUDY_NAME = "study_mf_velocity_difference"


def get_x(cur, module):
    return cur.execute(f"""
        select vectorData.value from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'self_x'
        and moduleName LIKE '{module}'
        order by simtimeRaw asc
        """).fetchall()
        
def get_y(cur, module):
    return cur.execute(f"""
        select vectorData.value from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'self_y'
        and moduleName LIKE '{module}'
        order by simtimeRaw asc
        """).fetchall()


def get_vec_files():
    files = []
    for result in os.listdir(f"{RESULTS}/{STUDY_NAME}"):
        files.extend(glob.glob(f"{RESULTS}/{STUDY_NAME}/{result}/*.vec"))
    return files
    
def plt_node(filter, color, alpha):
    x = [d[0] for d in get_x(cur, filter)]
    y = [d[0] for d in get_y(cur, filter)]
    
    u_x = []
    u_y = []
    
    for i in range(len(x)):
        coords = utm.from_latlon(y[i] / 10**6, x[i] / 10**6, 32, "U")
        
        u_x.append(coords[0] - 691444)
        u_y.append(coords[1] - 5335504)
        
    
    print(u_x[0], u_y[0])
    plt.plot(u_x, u_y, color=color, alpha=alpha)        
     
if __name__ == "__main__":
    with sqlite3.connect(get_vec_files()[0]) as db:
        cur = db.cursor()
        
        plt.rcParams["figure.figsize"] = (5,5)
        
        for i in range(40):
            print(i)
            plt_node(f"%pNode[{i}]%", "orange", 0.5)
        
        for i in range(30):
            print(i)
            plt_node(f"%vNode[{i}]%", "red", 0.5)
            
        plt.plot([], [], color="red", label="vNode")   
        plt.plot([], [], color="orange", label="pNode")  
        
        im = plt.imread("mf.png")
        plt.imshow(im, extent=(0, 493, 0, 662), origin='upper', alpha=0.5)
        
        plt.xlim((0, 493))
        plt.ylim((0, 662))
        
        plt.xlabel("x [m]")
        plt.ylabel("y [m]")
        
        plt.legend()
        
        plt.savefig(f"fig/mf_traj.png")    
        plt.savefig(f"fig/mf_traj.svg")
        plt.savefig(f"fig/mf_traj.pdf")
        
        plt.show()
        
    
    
    
    