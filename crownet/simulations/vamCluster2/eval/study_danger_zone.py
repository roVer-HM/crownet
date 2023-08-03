#!/usr/bin/env python3
import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_vru_pos, get_va_service_modules, aggregate_vam_recv
from common.plot_utils import save_plot
from common.opp_utils import s_to_st

RESULTS = "../results"
STUDY_NAME = "RunStudyEffectiveClustering"
TEMP_FILE = f"{STUDY_NAME}_dz.json"

# Seconds to opp simtime
ST_CONV_FACTOR = 10**12

# START_TIME = 200
# END_TIME = 500
START_TIME = 0
END_TIME = 550

DANGER_ZONE = (
    48154100,
    48154300
)

YLABEL = "Number of VRUs in danger zone"


def analyze_vector(vector_file):
    print(vector_file)

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        #
        # Analyze data received by vehicle nodes
        #
        modules = get_va_service_modules(cur, "vam_x", "World.vNode[%].middleware.VaService")

        rx_danger_zone = {}

        for mod in modules:
            node = mod[0]
            print(f"Analyzing {node}")

            rx_danger_zone[node] = {}

            agg_data = aggregate_vam_recv(cur, node)

            currently_in_dz = set()
            last_data = 0

            cluster_size = {}
            last_received = {}

            for t in range(START_TIME, END_TIME, 3):
                keys = [k for k in agg_data.keys() if k > s_to_st(t) and k < s_to_st(t + 3)]
                rx_danger_zone[node][t] = []
                
                

                for k in keys:
                    data = agg_data[k]

                    last_received[data["vam_id"]] = t
                    last_data = t

                    if (data["vam_y"] > DANGER_ZONE[0] * 10 and data["vam_y"]< DANGER_ZONE[1] * 10):
                        currently_in_dz.add(data["vam_id"])
                        
                        print(data["vam_cluster"])

                        if data["vam_cluster"] > 0:
                            if data["vam_id"] not in cluster_size:
                                cluster_size[data["vam_id"]] = []

                            cluster_size[data["vam_id"]].append(data["vam_cluster"])
                            
                        else:
                            cluster_size[data["vam_id"]] = [1]
                    else:
                        currently_in_dz.discard(data["vam_id"])

                # No data for 5s -> Clear all VRUs in danger zone (aging)
                if t - last_data > 3:
                    currently_in_dz.clear()

                in_dz = 0

                for c in currently_in_dz:
                    if t - last_received[c] < 2:
                        in_dz += int(np.median(cluster_size[c]))
                    

                rx_danger_zone[node][t] = in_dz

        #
        # Analyze real number of VRUs in danger zone
        #
        modules = get_va_service_modules(cur, "self_x", "World.pNode[%].middleware.VaService")

        # { mod1: [t1, t2, ...], mod: 2: [ ... ]  }
        modules_times_in_dz = {}
        
        # for mod in modules:
        #     node = mod[0]
        #     print(f"Analyzing {node}")

        #     d = aggregate_vam_recv(cur, node)

        #     modules_times_in_dz[node] = []

        #     pos_x = get_vru_pos(cur, node, "x")
        #     pos_y = get_vru_pos(cur, node, "y")

        #     for t in range(START_TIME, END_TIME):
        #         py = [p[0] for p in pos_y if p[1] > s_to_st(t) and p[1] < s_to_st(t + 1)]

        #         if len(py):
                    
        #             if (py[0] > DANGER_ZONE[0] and py[0] < DANGER_ZONE[1]):
        #                 # In Danger zone
        #                 modules_times_in_dz[node].append(t)
                    
        return (modules_times_in_dz, rx_danger_zone)

        

def analyze():
    pool = ThreadPoolExecutor(max_workers = 15)
    results = pool.map(analyze_vector, [get_vec_files(RESULTS, STUDY_NAME)[0]])

    data = {"actual": [], "rx": []}

    for r in results:
        data["actual"].append(r[0])
        data["rx"].append(r[1])

    with open(TEMP_FILE, "w") as f:
       json.dump(data, f)

def setup_fig(height=6, yl=YLABEL):
    fig = plt.figure(figsize=(15, 7))
    plt.rc('font', size=22)  

    ax = plt.gca()
    
    plt.ylabel(yl)
    plt.xlabel("Simulation Time (s)")
    fig.tight_layout()

    ax.grid(True, axis="both", linestyle="--")

    return (plt, fig, ax)

def plot_rx(ax, rx_data, label):
    received_distrib = {}

    for d in rx_data:
        # Iterate modules
        for m in d.keys():
            # Iterate timestamps
            for t in d[m].keys():
                t_val = int(t)

                number_of_vams = d[m][t]
                if number_of_vams > 0:
                    if t_val not in received_distrib:
                        received_distrib[t_val] = []
                    
                    
                    received_distrib[t_val].append(number_of_vams)

    received_distrib_x = list(received_distrib.keys())
    received_distrib_x.sort()

    received_distrib_y = [np.mean(received_distrib[k]) for k in received_distrib_x]

    ax.plot(received_distrib_x, received_distrib_y, label=label)

def visualize():
    data2 = {}

    
    with open("RunStudyEffectiveClustering_dz.json", "r") as f:
        data2 = json.load(f)


    with open(TEMP_FILE, "r") as f:
        data = json.load(f)
        
        actual_distrib = {}

        plt, fig, ax = setup_fig()

        for d in data["actual"]:
            # Iterate modules
            for m in d.keys():
                # Iterate timestamps
                for t in d[m]:
                    if t not in actual_distrib:
                        actual_distrib[t] = 0
                    actual_distrib[t] += 1

        actual_distrib_x = list(actual_distrib.keys())
        actual_distrib_x.sort()

        actual_distrib_y = [actual_distrib[k] for k in actual_distrib_x]

        ax.plot(actual_distrib_x, actual_distrib_y, label="Actual number")


        # received_distrib = {}

        plot_rx(ax, data["rx"], "No Clustering - Mean density received by vNodes")
        plot_rx(ax, data2["rx"], "Clustering - Mean density received by vNodes")

        # for d in data["rx"]:
        #     # Iterate modules
        #     for m in d.keys():
        #         # Iterate timestamps
        #         for t in d[m].keys():
        #             t_val = int(t)

        #             number_of_vams = d[m][t]
        #             if number_of_vams > 0:
        #                 if t_val not in received_distrib:
        #                     received_distrib[t_val] = []
                        
                        
        #                 received_distrib[t_val].append(number_of_vams)

        # received_distrib_x = list(received_distrib.keys())
        # received_distrib_x.sort()
        
        # received_distrib_y = [np.mean(received_distrib[k]) for k in received_distrib_x]

        # ax.plot(received_distrib_x, received_distrib_y, label="Mean density received by vNodes")
        ax.set_ylim(0, 50)
        ax.legend(loc="upper left")

        save_plot(plt, "danger_zone")



if __name__ == '__main__':
    if not os.path.isfile(TEMP_FILE):
        analyze()
    visualize()
    
