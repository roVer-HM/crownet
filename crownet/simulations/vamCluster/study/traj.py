#!/usr/bin/env python3
import os
import glob
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
import utm

RESULTS = "../results"
STUDY_NAME = "study_simple_no_cluster"


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
        
        u_x.append(coords[0] - 689432)
        u_y.append(coords[1] - 5336004)
    
    plt.plot(u_x, u_y, color=color, alpha=alpha)        
     
if __name__ == "__main__":
    with sqlite3.connect(get_vec_files()[0]) as db:
        cur = db.cursor()
        
        plt.rcParams["figure.figsize"] = (3,5)
        
        for i in range(30):
            print(i)
            plt_node(f"%pNode[{i}]%", "orange", 0.5)
        
        for i in range(5):
            plt_node(f"%vNode[{i}]%", "red", 0.5)
            
        plt.plot([], [], color="red", label="vNode")   
        plt.plot([], [], color="orange", label="pNode")  
        
        im = plt.imread("simple.png")
        plt.imshow(im, extent=(0, 400, 0, 400), origin='upper', alpha=0.5)
        
        plt.xlim((50, 250))
        plt.ylim((0, 400))
        
        plt.xlabel("x [m]")
        plt.ylabel("y [m]")
        
        plt.legend()
        
        plt.savefig(f"fig/simple_traj.png")    
        plt.savefig(f"fig/simple_traj.svg")
        plt.savefig(f"fig/simple_traj.pdf")
        
        plt.show()
        
    
    
    
    