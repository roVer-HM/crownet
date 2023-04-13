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
    
def plt_node(ax, filter, color, alpha):
    x = [d[0] for d in get_x(cur, filter)]
    y = [d[0] for d in get_y(cur, filter)]
    
    u_x = []
    u_y = []
    
    for i in range(len(x)):
        coords = utm.from_latlon(y[i] / 10**6, x[i] / 10**6, 32, "U")
        
        u_x.append(coords[0] - 689432)
        u_y.append(400 - (coords[1] - 5336004))
    
    ax.plot(u_y, u_x, color=color, alpha=alpha)        
     
if __name__ == "__main__":
    with sqlite3.connect(get_vec_files()[0]) as db:
        cur = db.cursor()
        
        fig, ax = plt.subplots(figsize=(7, 2.8))
        fig.subplots_adjust(bottom=0.15, left=0.1, top=1, right=0.95, hspace=0)
        
        for i in range(30):
            print(i)
            plt_node(ax, f"%pNode[{i}]%", "orange", 0.5)
        
        for i in range(5):
            plt_node(ax, f"%vNode[{i}]%", "red", 0.7)
            
        ax.plot([], [], color="red", label="vNode trajectory")   
        ax.plot([], [], color="orange", label="pNode trajectory")
        ax.plot([], [], color="royalblue", label="vNode obstacle", linewidth=3)
        ax.plot([], [], color="gray", label="Road", linewidth=3)
        
        im = plt.imread("simple.png")
        ax.imshow(im, extent=(0, 400, 0, 400), origin='upper', alpha=0.35)
        
        ax.set_xlim(0, 400)
        ax.set_ylim(75, 230)
        
        plt.xlabel("y [m]")
        plt.ylabel("x [m]")
        
        ax.legend(ncol=2, loc="upper center")
        
        fig.savefig(f"fig/simple_traj.png")    
        fig.savefig(f"fig/simple_traj.svg")
        fig.savefig(f"fig/simple_traj.pdf")
        
        plt.show()
        
    
    
    
    