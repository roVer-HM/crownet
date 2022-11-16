import os 
import time
import sqlite3
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

from datetime import datetime
from _datetime import timedelta
from builtins import set

# Start time of the simulation - taken from the simulations omnetpp.ini
time_start = "2020-06-01 12:35:00"

# Epoch start for genDeltaTime calculation - taken from the simulations omnetpp.ini
epoch_start = "2004-01-01 00:00:00"

# Variable used for genDeltaTime modulo calculation
gen_delta_mod = 65536

# Offset for the genDeltaTime calculation in milliseconds: The simulation starts at time_start and the start of the epoch is epoch_start
ts_offset = (datetime.fromisoformat(time_start) - datetime.fromisoformat(epoch_start))/timedelta(milliseconds=1)


def calc_latency(genDelta, simTime):
    ts = (ts_offset + simTime) % gen_delta_mod
    lat = ts - genDelta
    
    if lat < 0:
        lat = (gen_delta_mod + ts) - genDelta
        
    return lat
    

""" 
Calculates the average latency of all received VAMs in milliseconds.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            A df containing two columns: simtime & latency (in ms)
"""
def latency(db_path, scenario, config):    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'reception:vector(vamGenerationDeltaTime)'")
    v_ids = cur.fetchall()
    
    df = pd.read_sql_query("SELECT vd.value as genDelta, vd.simtimeRaw/1e9 as simtime FROM vectorData as vd WHERE vd.vectorId IN (" + ",".join(map(str, v_ids)) + ")", con)
    """
    generationDeltaTime = TimestampIts mod 65 536 
    TimestampIts represents an integer value in milliseconds since 2004-01-01T00:00:00:000Z as defined in ETSI TS 102 894-2
    """
    df["latency"] = df.apply(lambda row : calc_latency(row["genDelta"], row["simtime"]), axis=1)
    df["simtime"] = df.apply(lambda row: row[1]/1000, axis=1)
    df = df.sort_values(by="simtime", ascending=True)
    cur.close()
    
    avg_latency = round(df["latency"].mean(),3)
    
    plt.close()
    
    plt.figure(figsize=(12, 6), dpi=300)
    plt.scatter(df.simtime, df.latency, s=20, alpha=0.7, linewidths=0)
    plt.xlabel("Simtime in s", fontsize=14)
    plt.ylabel("Latency in ms", fontsize=14)
    plt.xticks(fontsize=12)
    plt.savefig("plots/" + scenario + "/"  + scenario + "_" + config +"_lat_scatter.png")
    
    plt.close()
    
    plt.figure(figsize=(20, 6), dpi=300)
    plt.plot(df.simtime, df.latency, label="Latency", linewidth=1)
    plt.legend(loc="upper right")
    plt.xlabel("Simtime in s")
    plt.ylabel("Latency in ms")
    plt.savefig("plots/" + scenario + "/"  + scenario + "_" + config +"_lat_line.png")
    
    # Clear plot window between consecutive plots
    plt.close()
    
    plt.hist(x=df["latency"], bins=60)
    plt.xlabel("Latency in ms")
    plt.ylabel("Count")
    plt.savefig("plots/" + scenario + "/"  + scenario +  "_" + config + "_lat_hist.png")
    
    print ("# Latency: " + str(df.shape[0]))
    print("Average Latency for " + scenario + " : " + str(avg_latency))
    
    return df

"""
Calculates the average datavolume across all VAMs sent in bytes.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            A dataframe containing two cols: simtime & bytes that were sent. 
                    A dataframe containing two cols: simtime in 1s interval & bytes that were sent in each interval.
"""
def datavolume(db_path, scenario, config):
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'packetSent:vector(packetBytes)'")
    v_ids = cur.fetchall()
    
    # simtime in ms
    df = pd.read_sql_query("SELECT vd.value as bytes, vd.simtimeRaw/1e9 as simtime FROM vectorData as vd WHERE vd.vectorId IN (" + ",".join(map(str, v_ids)) + ")", con)
    df["simtime"] = df.apply(lambda row: row[1]/1000, axis=1)
    df = df.sort_values(by="simtime", ascending=True)
    
    total_datavolume = df["bytes"].sum()
    
    cur.close()
    
    df_bytes_grouped = df.groupby(df["simtime"] // 1 ).bytes.sum().reset_index()
    avg_bytes_per_sec = df_bytes_grouped["bytes"].mean()
    
    plt.clf()
    
    lp = df_bytes_grouped.plot.line(x="simtime", y="bytes")
    lp.set_xlabel("Simtime in s")
    lp.set_ylabel("Data sent (Bytes)")
    lp.figure.savefig("plots/" + scenario + "/"  + scenario + "_" + config + "_vol_line.png")
    
    plt.clf()
    
    print("Total datavolume for " + scenario + " :" + str(total_datavolume))
    print("Average bytes per sec for " + scenario + " : " + str(avg_bytes_per_sec))
    
    return df, df_bytes_grouped

"""
Calculates the average deviation of a VRUs known position from its actual position in meters.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            A dataframe containg two cols: simtime & posdiff (in meters)
"""
def pos_diff(db_path, scenario, config): 
    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'posdiff:vector'")
    v_ids = cur.fetchall()
    
    #Posdiff in meters
    df = pd.read_sql_query("SELECT vd.value as posdiff, vd.simtimeRaw/1e9 as simtime FROM vectorData as vd WHERE vd.vectorId IN (" + ",".join(map(str, v_ids)) + ")", con)
    df["simtime"] = df.apply(lambda row: row[1]/1000, axis=1)
    # Remove stray values
    #df.drop(df[df["posdiff"] >= 1.0].index, inplace=True)
    df = df.sort_values(by="simtime", ascending=True)
    
    df_stats = pd.read_sql_query("SELECT v.vectorCount as count, v.vectorSum as posdiff FROM vector as v where v.vectorName = 'posdiff:vector'", con)
    df_sum = df_stats.apply(np.sum, axis=0)
    
    cur.close()   
    
    plt.close()
    
    plt.figure(figsize=(12, 6), dpi=300)
    plt.scatter(df.simtime, df.posdiff, s=20, alpha=0.7, linewidths=0)
    plt.xlabel("Simtime in s", fontsize=14)
    plt.ylabel("posDiff in m", fontsize=14)
    plt.xticks(fontsize=12)
    plt.savefig("plots/" + scenario + "/"  + scenario + "_" + config +"_pos_scatter.png")
    
    plt.close()  
    
    hp = df["posdiff"].plot(kind="hist", bins=60)
    hp.set_xlabel("Position difference in m")
    hp.figure.savefig("plots/" + scenario + "/" + scenario + "_" + config + "_pos_hist.png")
    
    # Clear plot window between consecutive plots
    plt.close()
    
    plt.figure(figsize=(20, 6), dpi=300)
    plt.plot(df.simtime, df.posdiff, linewidth=1)
    plt.xlabel("Simtime in s")
    plt.ylabel("posDiff in m")
    plt.savefig("plots/" + scenario + "/"  + scenario + "_" + config + "_pos_line.png")
    
    plt.close()
    
    
    avg_pos_diff = df_sum[1]/df_sum[0]
    print("Average posDiff for " + scenario + " : " + str(avg_pos_diff))
    
    return df


"""
Plots the results of latency of all scenearios with the same parameter configuration in one graph.

<id> -> par_var_id + prev_seeds_run e.g. seed = 2 first config in par var [0,1] second config = [2,3]...
<run_id> -> always 0
Pattern -> study_<name>/simulation_runs/outputs/Sample_<id>_<run_id>/<name>_out/vars_rep_0.vec
study_osm_small/simulation_runs/outputs/Sample_0_0/osm_small_out/vars_rep_0.vec
study_osm_med/simulation_runs/outputs/Sample_0_0/osm_med_out/vars_rep_0.vec
study_osm_large/simulation_runs/outputs/Sample_0_0/osm_large_out/vars_rep_0.vec

plot two dfs in same plot
ax = df1.plot()
df2.plot(ax=ax)

:param:      Scenario

:return:     True if the generation of the graphs succeeded else false.
"""
def compare_latency(scenario):
    df1 = latency("results/study_" + scenario + "/simulation_runs/outputs/Sample_0_0/" + scenario + "_out/vars_rep_0.vec", scenario,"low")
    df2 = latency("results/study_" + scenario + "/simulation_runs/outputs/Sample_1_0/" + scenario + "_out/vars_rep_0.vec", scenario, "med")
    df3 = latency("results/study_" + scenario + "/simulation_runs/outputs/Sample_2_0/" + scenario + "_out/vars_rep_0.vec", scenario, "high")
    
    
    plt.close()
    
    plt.figure(figsize=(12, 6), dpi=300)
    plt.scatter(df1.simtime, df1.latency, s=20, alpha=0.1, linewidths=0, label="Latency - Low Traffic")
    plt.scatter(df2.simtime, df2.latency, s=20, alpha=0.1, linewidths=0, edgecolors="orange", label="Latency - Medium Traffic")
    plt.scatter(df3.simtime, df3.latency, s=20, alpha=0.1, linewidths=0, edgecolors="blue", label="Latency - Medium Traffic")
    leg = plt.legend(loc="best")
    
    for lh in leg.legendHandles:
        lh.set_alpha(1)
    
    plt.xlabel("Simtime in s")
    plt.ylabel("Latency in ms")
    plt.savefig("plots/" + scenario + "/"  + scenario +"_lat_scatter_combined.png")
    
    plt.close()
    
    plt.figure(figsize=(20, 6), dpi=300)
    plt.plot(df1.simtime, df1.latency, alpha=0.5, label="Latency - Low Traffic", linewidth=1)
    plt.plot(df2.simtime, df2.latency, alpha=0.5, label="Latency - Medium Traffic", linewidth=1)
    plt.plot(df3.simtime, df3.latency, alpha=0.5, label="Latency - High Traffic", linewidth=1)
    plt.legend(loc="upper right")
    plt.xlabel("Simtime in s")
    plt.ylabel("Latency in ms")
    plt.savefig("plots/" + scenario + "/" + scenario + "_lat_line_combined.png")
    
    # Clear plot window between consecutive plots
    plt.close()
    
    plt.hist(df1.latency, bins=70, alpha=0.5, label="Latency - Low Traffic")
    plt.hist(df2.latency, bins=70, alpha=0.5, label="Latency - Medium Traffic")
    plt.hist(df3.latency, bins=70, alpha=0.5, label="Latency - High Traffic")
    plt.legend(loc="upper right")
    plt.xlabel("Latency in ms")
    plt.savefig("plots/" + scenario + "/" + scenario + "_lat_hist_combined.png")
    
    return True

def compare_datavolume(scenario):
    df1, df1_bps = datavolume("results/study_" + scenario + "/simulation_runs/outputs/Sample_0_0/" + scenario + "_out/vars_rep_0.vec", scenario, "low")
    df2, df2_bps = datavolume("results/study_" + scenario + "/simulation_runs/outputs/Sample_1_0/" + scenario + "_out/vars_rep_0.vec", scenario, "med")
    df3, df3_bps = datavolume("results/study_" + scenario + "/simulation_runs/outputs/Sample_2_0/"+ scenario + "_out/vars_rep_0.vec", scenario, "high")
    
    plt.clf()
    
    plt.plot(df1_bps.simtime, df1_bps.bytes, alpha=0.5, label="Bytes/s - Low Traffic")
    plt.plot(df2_bps.simtime, df2_bps.bytes, alpha=0.5, label="Bytes/s - Medium Traffic")
    plt.plot(df3_bps.simtime, df3_bps.bytes, alpha=0.5, label="Bytes/s - High Traffic")
    plt.legend(loc="upper right")
    plt.xlabel("Simtime in s")
    plt.ylabel("Data sent in bytes")
    plt.savefig("plots/" + scenario + "/" + scenario + "_vol_line_combined.png")
    
    return True

def compare_posdiff(scenario):
    df1 = pos_diff("results/study_" + scenario + "/simulation_runs/outputs/Sample_0_0/" + scenario + "_out/vars_rep_0.vec", scenario, "low")
    df2 = pos_diff("results/study_" + scenario + "/simulation_runs/outputs/Sample_1_0/" + scenario + "_out/vars_rep_0.vec", scenario, "med")
    df3 = pos_diff("results/study_" + scenario + "/simulation_runs/outputs/Sample_2_0/"+ scenario + "_out/vars_rep_0.vec", scenario, "high")
    
    plt.clf()
    
    plt.figure(figsize=(20, 6), dpi=300)
    plt.plot(df1.simtime, df1.posdiff, alpha=0.5, label="posDiff - Low Traffic", linewidth=1)
    plt.plot(df2.simtime, df2.posdiff, alpha=0.5, label="posDiff - Medium Traffic", linewidth=1)
    plt.plot(df3.simtime, df3.posdiff, alpha=0.5, label="posDiff - High Traffic", linewidth=1)
    plt.legend(loc="upper right")
    plt.xlabel("Simtime in s")
    plt.ylabel("posDiff in m")
    plt.savefig("plots/" + scenario + "/"  + scenario + "_pos_line_combined.png")
    
    # Clear plot window between consecutive plots
    plt.close()
    
    plt.hist(df1.posdiff, bins=70, alpha=0.5, label="posDiff - Low Traffic")
    plt.hist(df2.posdiff, bins=70, alpha=0.5, label="posDiff - Medium Traffic")
    plt.hist(df3.posdiff, bins=70, alpha=0.5, label="posDiff - High Traffic")
    plt.legend(loc="upper right")
    plt.xlabel("posDiff in m")
    plt.savefig("plots/" + scenario + "/" + scenario + "_pos_hist_combined.png")
    
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sc", type=str, required=True, help="Sceneario for cp_vol, cp_pos, cp_lat. Either osm_small, osm_medium or osm_large.")
    
    subparsers = parser.add_subparsers()
    parser_latency = subparsers.add_parser("lat", help="Print average VAM latency across all sent VAM, plot scatter plot + histogramm & return df with latency data.")
    parser_latency.set_defaults(func=latency)
    parser_datavolume = subparsers.add_parser("vol", help="Print average of data volume of VAM related messages, plot lineplot & return df with throughput data.")
    parser_datavolume.set_defaults(func=datavolume)
    parser_pos_diff = subparsers.add_parser("pos", help="Print average of deviation of a VRUs known position from its actual position, plot scatterplot + histogram & return df with position diff data.")
    parser_pos_diff.set_defaults(func=pos_diff)
    parser_compare_lat = subparsers.add_parser("cp_lat", help="Generate graphs of M1 for all scenarios (study_osm_small, study_osm_medium, study_osm_large).")
    parser_compare_lat.set_defaults(func=compare_latency)
    parser_compare_datavolume = subparsers.add_parser("cp_vol", help="Generate graphs of M2 for all scenarios (study_osm_small, study_osm_medium, study_osm_large).")
    parser_compare_datavolume.set_defaults(func=compare_datavolume)
    parser_compare_posdiff = subparsers.add_parser("cp_pos", help="Generate graphs of M3 for all scenarios (study_osm_small, study_osm_medium, study_osm_large).")
    parser_compare_posdiff.set_defaults(func=compare_posdiff)
    
    # calc_latency("results/study_osm_small/simulation_runs/outputs/Sample_0_0/osm_small_out/vars_rep_0.vec")
    options = parser.parse_args()
    options.func(options.sc)

if __name__ == "__main__":
    main()
    
