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

START_TIME = 0
END_TIME = 550
RESULTS = "../results"
STUDY_NAME = "RunStudyNoClusteringDiscrete"
TEMP_FILE = f"{STUDY_NAME}_dzd.json"

DZ_EAST = 689920
DZ_NORTH = 5336590
DZ_RAD = 20

"""
Checks if an UTM coordinate lays within the defined danger zone
"""
def is_utm_in_danger_zone(utm_coord):
    return math.sqrt((utm_coord[0] - DZ_EAST) ** 2 + (utm_coord[1] - DZ_NORTH) ** 2) < DZ_RAD


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
Returns a set of times where a vehicle receives VAMs of nodes in danger zone
"""
def get_received_danger_zone_times(cur):
    danger_times = set()
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

                if is_utm_in_danger_zone(utm_pos):
                    danger_times.add(t)

    return danger_times




def analyze_vector(vector_file):
    print(f"Analyze {vector_file}")

    with sqlite3.connect(vector_file) as db:
        cur = db.cursor()

        actual_danger_times = times_with_nodes_in_danger_zone(get_actual_node_positions(cur))
        received_danger_times = get_received_danger_zone_times(cur)

        return {
            "actual": list(actual_danger_times),
            "received": list(received_danger_times)
        }


"""
Performs statistical analysis on the results and visualizes them
"""
def visualize(data):
    for repetition in data:
        actual = set(repetition["actual"])
        received = set(repetition["received"])

        print(len(actual))
        print(len(received))

        print(received - actual)
        print(actual - received)
        


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