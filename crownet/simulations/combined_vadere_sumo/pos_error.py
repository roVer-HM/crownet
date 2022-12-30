import sqlite3
import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import threading

# RESULT_VECTOR = "/media/ramdisk/mfCombined_20221209-09:11:09/vars_rep_0.vec"
RESULT_VECTOR = "/home/rupp/crownet_paper/crownet/simulations/combined_vadere_sumo/results/mfCombined_20221216-10:16:43/vars_rep_0.vec"
# RESULT_VECTOR = "/home/rupp//crownet_paper/crownet/simulations/combined_vadere_sumo/results/mfSumo_20221209-13:40:45/vars_rep_0.vec"
WORLD = "MultimobilityWorld"
# WORLD = "World"

P_PROVIDER="(Vadere)"
V_PROVIDER="(SUMO)"

S = 10**12
TIME_FROM = 20 * S
TIME_TO = 300 * S

def eukl(x1, y1, x2, y2):
    return math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)

class VecDataReader:
    def __init__(self, cur):
        self.cur = cur
        node_mob_update_rows = self.cur.execute(f"""
        SELECT vd.simtimeRaw, v.moduleName, v.vectorName, vd.value
        FROM vector v
        INNER JOIN vectorData vd
            ON v.vectorId = vd.vectorId
            AND v.moduleName LIKE '{WORLD}.%Node[%]'
            AND (
                v.vectorName = 'posX:vector' OR
                v.vectorName = 'posY:vector'
            )
            AND vd.simtimeRaw > {TIME_FROM}
            AND vd.simtimeRaw < {TIME_TO}
        ORDER BY vd.simtimeRaw ASC""").fetchall()

        self.node_mob = {}
        self.node_names = set()
        for r in node_mob_update_rows:
            if r[0] not in self.node_mob:
                self.node_mob[r[0]] = {}
            if r[1] not in self.node_mob[r[0]]:
                self.node_mob[r[0]][r[1]] = {}
                self.node_names.add(r[1])
            if r[2] == "posX:vector":
                self.node_mob[r[0]][r[1]]["x"] = r[3]
            if r[2] == "posY:vector":
                self.node_mob[r[0]][r[1]]["y"] = r[3]

        node_recv_data_rows = self.cur.execute(f"""
        SELECT vd.simtimeRaw, v.moduleName, v.vectorName, vd.value
        FROM vector v
        INNER JOIN vectorData vd
            ON v.vectorId = vd.vectorId
            AND v.moduleName LIKE '{WORLD}.%Node[%].app[0].app'
            AND (
                v.vectorName = 'receivedX'  OR
                v.vectorName = 'receivedY'  OR
                v.vectorName = 'receivedId' OR
                v.vectorName = 'selfId'
            )
            AND vd.simtimeRaw > {TIME_FROM}
            AND vd.simtimeRaw < {TIME_TO}
        ORDER BY vd.simtimeRaw ASC""").fetchall()

        self.node_recv = {}
        self.node_id_map = {}
        self.last_data = {}
        self.first_data = {}
        for r in node_recv_data_rows:
            node_name = r[1].split(".app")[0]
            if node_name not in self.first_data:
                self.first_data[node_name] = r[0]

            self.last_data[node_name] = r[0]
            key = f"{r[0]}.{r[1]}"
            if key not in self.node_recv:

                mob_data = {}
                for ts, v in self.node_mob.items():
                    if node_name in v and ts < r[0]:
                        mob_data = v[node_name]
                    elif ts > r[0]:
                        break

                self.node_recv[key] = {
                    "ts" : r[0],
                    "receiver" : r[1],
                    "receiver_node": node_name,
                    "mobility": mob_data
                }

            if r[2] == "receivedId":
                self.node_recv[key]["id"] = r[3]
            if r[2] == "receivedX":
                self.node_recv[key]["x"] = r[3]
            if r[2] == "receivedY":
                self.node_recv[key]["y"] = r[3]
            if r[2] == "selfId":
                self.node_id_map[r[1]] = r[3]

    def get_all_nodes_of_type(self, type):
        return [n.split(".")[1] for n in list(self.node_names) if type in n]

    def analyze_single_node_position_error(self, n, recv):
        p_err_data = []
        n_path = f"{WORLD}.{n}"
        n_id = self.node_id_map[f"{n_path}.app[0].app"]

        for k, v in self.node_mob.items():
            if n_path in v:
                ts = k
                x = v[n_path]["x"]
                y = v[n_path]["y"]

                # Search pNodes that received vNode data and are still active
                pNodes_received_vNode = [
                    v for v in self.node_recv.values()
                    if (recv in v["receiver"])
                       and (v["id"] == n_id)
                       and (v["ts"] < ts)
                       and self.last_data[v["receiver_node"]] > ts
                       and "mobility" in v
                       and "x" in v["mobility"]
                       and "y" in v["mobility"]
                ]

                selected_pNodes = {}

                # Get latest receive event for each pNode
                for p in pNodes_received_vNode:
                    if p["receiver_node"] not in selected_pNodes:
                        selected_pNodes[p["receiver_node"]] = p
                    if selected_pNodes[p["receiver_node"]]["ts"] < p["ts"]:
                        selected_pNodes[p["receiver_node"]] = p

                for p in selected_pNodes.values():
                    # Calculate position error
                    # Filter nodes that are far away
                    dist = eukl(x, y, p["mobility"]["x"], p["mobility"]["y"])
                    if dist < 20:
                        err = eukl(p["x"], p["y"], x, y)
                        
                        if err > 0.0:
                            p_err_data.append(err)
                        
        self.results[n] = p_err_data

    def analyze_node_position_error(self, nodes, recv):
        self.results = {}
        for n in nodes:
            print(f"--- {n}")
            self.analyze_single_node_position_error(n, recv)

        return sum(self.results.values(), [])

def main():
    
    if False:
        print("init db")
    
        con = sqlite3.connect(RESULT_VECTOR)
        cur = con.cursor()
    
        print("read data")
    
        v_reader = VecDataReader(cur)
    
        print("analyze p to v")
    
        p_nodes = v_reader.get_all_nodes_of_type("pNode")
        data_p_v = v_reader.analyze_node_position_error(p_nodes, "vNode")
    
        print("analyze v to p")
    
        v_nodes = v_reader.get_all_nodes_of_type("vNode")
        data_v_p = v_reader.analyze_node_position_error(v_nodes, "pNode")
    
        print("finish")
    
        df = pd.DataFrame()
        df["pv"] = data_p_v
        df.to_csv(f"./tmp_pv_{WORLD}.csv")
    
        df = pd.DataFrame()
        df["vp"] = data_v_p
        df.to_csv(f"./tmp_vp_{WORLD}.csv")
        
    data_p_v = pd.read_csv(f"./tmp_pv_{WORLD}.csv")["pv"]
    data_v_p = pd.read_csv(f"./tmp_vp_{WORLD}.csv")["vp"]
    
    fig = plt.figure(dpi=250)
    fig.set_size_inches(9, 11.5)
    
    plt.rc('font', size=26)

    plt.ylim(-0.1, 2.4)
    plt.boxplot([data_p_v, data_v_p], labels=[f"Person -> Vehicle\nMedian: {str(np.median(data_p_v))[:4]}", f"Vehicle -> Person\nMedian: {str(np.median(data_v_p))[:4]}"], showfliers=False, widths=0.5, boxprops=dict(linewidth=2, markersize=10))
    plt.xlabel("Communication direction")
    plt.ylabel("Position error in meters")
    
    plt.savefig(f"/home/rupp/pos_error_{WORLD}.png")
    plt.savefig(f"/home/rupp/pos_error_{WORLD}.svg")
    plt.savefig(f"/home/rupp/pos_error_{WORLD}.pdf")
    plt.show()


if __name__ == "__main__":
    main()