import os
import sqlite3
import json
import matplotlib.pyplot as plt
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from common.db_utils import get_vec_files, get_parameter_value, get_vru_pos, get_va_service_modules, aggregate_vam_recv
from common.plot_utils import save_plot
from common.opp_utils import s_to_st
import utm
import math
import matplotlib.ticker as mtick

IGNORE_RADIUS = True
EXPORT_FALSE_POSITIVES = False
EXPORT_FALSE_NEGATIVES = True

START_TIME = 0
END_TIME = 550
RESULTS = "../results"
STUDY_NAME = "RunStudyEffectiveDiscrete"
TEMP_FILE = f"{STUDY_NAME}_dzd.json"

DZ_EAST = 689920
DZ_NORTH = 5336590
DZ_RAD = 20

"""
Checks if an UTM coordinate lays within the defined danger zone
"""
def is_utm_in_danger_zone(utm_coord, cluster_radius = 0):
    return math.sqrt((utm_coord[0] - DZ_EAST) ** 2 + (utm_coord[1] - DZ_NORTH) ** 2) < (DZ_RAD + cluster_radius)


"""
Get positions of all nodes by time in seconds.
Converts lat/lon to UTM
node_path -> t -> (x, y)
"""
def get_actual_node_positions(cur):
        # Get all pNode modules
        modules = get_va_service_modules(cur, "self_x", "World.pNode[%].middleware.VaService")

        aggregated_by_node_path = {}

        for mod in modules:
            # Module path
            node = mod[0]

            # Get actual node positions from mobility simulation
            pos_x = get_vru_pos(cur, node, "x")
            pos_y = get_vru_pos(cur, node, "y")

            # Aggregate by seconds to t -> (x, y)
            node_pos_at = {}

            for t in range(START_TIME, END_TIME):
                node_x_pos = [p[0] for p in pos_x if p[1] > s_to_st(t) and p[1] < s_to_st(t + 1)]
                node_y_pos = [p[0] for p in pos_y if p[1] > s_to_st(t) and p[1] < s_to_st(t + 1)]

                if len(node_x_pos) and len(node_y_pos):
                    node_pos_at[t] = utm.from_latlon(node_y_pos[0] / 10 ** 6, node_x_pos[0] / 10 ** 6)

            aggregated_by_node_path[node] = node_pos_at

        return aggregated_by_node_path


"""
Returns all time periods in seconds where pNodes are inside the danger zone.

Parameters:
    positions: Node positions in the format node_path -> t -> (x, y)
               As produced by get_actual_node_positions

Returns a set of times where nodes are in the danger zone
"""
def times_with_nodes_in_danger_zone(positions):
    danger_times = set()

    for t in range(START_TIME, END_TIME):
        for mod in positions.keys():

            if t in positions[mod]:
                if is_utm_in_danger_zone(positions[mod][t]):
                    danger_times.add(t)

    return danger_times


"""
Extract snapshot (list of all VRUs / Clusters at a certain timestamp)
t = Current timestamp
data = Array of nodes (utm, bbox)
"""
def extract_snapshot(t, data):
    return (t, [(utm.from_latlon(d["vam_y"] / 10 ** 7, d["vam_x"]  / 10 ** 7), d["vam_bbox_size"] / 10) for d in data])
    

"""
Returns a set of times where a vehicle receives VAMs of nodes in danger zone
Also creates snapshots for all timestamps with danger zone events
"""
def get_received_danger_zone_times(cur):
    danger_times = set()
    snapshots = []

    modules = get_va_service_modules(cur, "vam_x", "World.vNode[%].middleware.VaService")

    for mod in modules:
        node = mod[0]

        agg_data = aggregate_vam_recv(cur, node)

        for t in range(START_TIME, END_TIME):
            # Find all timestamps within the current time interval
            keys = [k for k in agg_data.keys() if k > s_to_st(t) and k < s_to_st(t + 1)]
            for k in keys:
                data = agg_data[k]

                utm_pos = utm.from_latlon(data["vam_y"] / 10 ** 7, data["vam_x"]  / 10 ** 7)

                # Cluster VAM bbox radius is in decimeters!
                if is_utm_in_danger_zone(utm_pos, (0 if IGNORE_RADIUS else data["vam_bbox_size"] / 10)):
                    danger_times.add(t)

            snapshots.append(extract_snapshot(t, [agg_data[k] for k in keys]))

    return (danger_times, snapshots)


"""
node_path -> t -> (x, y) ==> t -> [(x, y)]
"""
def aggregate_actual_positions_by_time(positions):
    aggregated = {}

    for t in range(START_TIME, END_TIME):
        if t not in aggregated:
            aggregated[t] = []

        for mod in positions.keys():
            if t in positions[mod]:
                aggregated[t].append(positions[mod][t])

    return aggregated



def analyze_vector(vector_file):
    print(f"Analyze {vector_file}")

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        actual_positions = get_actual_node_positions(cur)
        actual_danger_times = times_with_nodes_in_danger_zone(actual_positions)
        received_danger_times, snapshots = get_received_danger_zone_times(cur)
        actual_positions = aggregate_actual_positions_by_time(actual_positions)

        return {
            "actual_pos": actual_positions,
            "actual": list(actual_danger_times),
            "received": list(received_danger_times),
            "snapshots": list(snapshots)
        }



def save_snapshot(snapshots, name, t, actual):
    fig = plt.figure(figsize=(5, 5))
    plt.rc('font', size=22)
    ax = plt.gca()

    plt.subplots_adjust(left=0.1, right=0.95, top=0.9, bottom=0.1)

    # Plot actual node positions
    ax.scatter(
        [a[0] for a in actual],
        [a[1] for a in actual],
        s=7,
        label="VRU (actual)",
        color="black"
    )

    # Just for legend
    ax.scatter([], [], label="VRU (received)", color="blue", marker="x")

    for snapshot in snapshots:
        # Plot received node positions
        ax.scatter(
            [n[0][0] for n in snapshot],
            [n[0][1] for n in snapshot],
            color="blue",
            marker="x"
        )

        for n in snapshot:
            # Plot cluster radiuses
            ax.add_patch(plt.Circle((n[0][0], n[0][1]), n[1], color="blue", fill=False))

    # Danger Zone
    ax.scatter(
        [DZ_EAST],
        [DZ_NORTH],
        color="red",
        marker="x",
        label="Danger Zone"
    )

    plt.xticks([])      
    plt.yticks([]) 

    ax.add_patch(plt.Circle((DZ_EAST, DZ_NORTH), DZ_RAD, color="red", fill=False))

    ax.set_xlim(DZ_EAST - 35, DZ_EAST + 35)
    ax.set_ylim(DZ_NORTH - 35, DZ_NORTH + 35)

    ax.set_xlabel("UTM Easting")
    ax.set_ylabel("UTM Northing")

    plt.title(f"t = {t}s")

    ax.legend(loc="lower right")

    save_plot(plt, f"snaps/{name}")

    plt.close()

"""
Performs statistical analysis on the results and visualizes them
"""
def visualize(data):
    false_positives = []
    false_negatives = []

    for ri, repetition in enumerate(data):
        actual = set(repetition["actual"])
        received = set(repetition["received"])
        actual_positions = repetition["actual_pos"]

        false_positive = len(received - actual)
        false_positives.append(false_positive / (END_TIME - START_TIME) * 100)

        false_negative = len(actual - received)
        false_negatives.append(false_negative / (END_TIME - START_TIME) * 100)

        # Save snapshots for false-positive events
        snapshots = repetition["snapshots"]

        if EXPORT_FALSE_POSITIVES:
            for fp in (received - actual):
                snapshots_filtered = [s[1] for s in snapshots if s[0] == fp]
                save_snapshot(snapshots_filtered, f"{STUDY_NAME}_false_positive_{fp}_{ri}", fp, actual_positions[f"{fp}"])

        if EXPORT_FALSE_NEGATIVES:
            for fn in (actual - received):
                snapshots_filtered = [s[1] for s in snapshots if s[0] == fn]
                save_snapshot(snapshots_filtered, f"{STUDY_NAME}_false_negative{fn}_{ri}", fn, actual_positions[f"{fn}"])



    fig = plt.figure(figsize=(6, 6))
    plt.rc('font', size=22)  

    ax = plt.gca()
    fig.tight_layout()

    plt.subplots_adjust(left=0.2, right=0.9, top=0.95, bottom=0.2)

    ax.yaxis.set_major_formatter(mtick.PercentFormatter())
        
    ax.bar(
        ["False\nNegative", "False\nPositive"],
        [np.mean(false_negatives), np.mean(false_positives)],
        yerr=[np.std(false_negatives), np.std(false_positives)],
        color=["coral", "deepskyblue"],
        width=0.7,
        edgecolor="black",
        linewidth=2,
        capsize=5
    )

    ax.set_ylim(0, 2)

    save_plot(plt, f"danger_zone_{STUDY_NAME}")

        


def main():
    if not os.path.isfile(TEMP_FILE):

        pool = ThreadPoolExecutor(max_workers = 15)
        results = pool.map(analyze_vector, get_vec_files(RESULTS, STUDY_NAME))

        with open(TEMP_FILE, "w") as f:
            json.dump(list(results), f)

    with open(TEMP_FILE, "r") as f:
        visualize(json.load(f))


if __name__ == "__main__":
    main()