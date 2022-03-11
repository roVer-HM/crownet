import os 
import time
import sqlite3
import argparse

from datetime import datetime
from _datetime import timedelta

# Start time of the simulation - taken from the simulations omnetpp.ini
time_start = "2020-06-01 12:35:00"

# Epoch start for genDeltaTime calculation - taken from the simulations omnetpp.ini
epoch_start = "2004-01-01 00:00:00"

# Variable used for genDeltaTime modulo calculation
gen_delta_mod = 65536

# Offset for the genDeltaTime calculation in milliseconds: The simulation starts at time_start and the start of the epoch is epoch_start
ts_offset = (datetime.fromisoformat(time_start) - datetime.fromisoformat(epoch_start))/timedelta(milliseconds=1)


""" 
Calculates the average latency of all received VAMs in milliseconds.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            The average latency of all received VAMS in milliseconds.
"""
def calc_latency(db_path):
    total_lat = 0
    count = 0
    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'reception:vector(vamGenerationDeltaTime)'")
    v_ids = cur.fetchall()
    
    for v_id in v_ids:
        cur.row_factory = None
        cur.execute("SELECT vd.simtimeRaw/1e9 as simTimeMs, vd.value FROM vectorData as vd WHERE vd.vectorId = " + str(v_id))
        ts_tuples = cur.fetchall()
        
        for ts_tuple in ts_tuples:
            gen_delta_prev = ts_tuple[1]
            """
            generationDeltaTime = TimestampIts mod 65 536 
            TimestampIts represents an integer value in milliseconds since 2004-01-01T00:00:00:000Z as defined in ETSI TS 102 894-2
            """
            gen_delta_curr = (ts_offset + ts_tuple[0]) % gen_delta_mod
            lat = gen_delta_curr - gen_delta_prev
            total_lat += lat
            count += 1 
    
    cur.close()
    avg_latency = round(total_lat/count, 3)
    print(avg_latency)
    
    return avg_latency

"""
Calculates the average datavolume across all VAMs sent in bytes.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            The average datavolume across all VAMs sent in bytes.
"""
def calc_datavolume(db_path):
    total_datavolume = 0
    count = 0
    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'packetSent:vector(packetBytes)'")
    v_ids = cur.fetchall()
    
    for v_id in v_ids:
        cur.execute("SELECT vd.value FROM vectorData as vd WHERE vd.vectorId = " + str(v_id))
        received_packets = cur.fetchall()
        total_datavolume += sum(received_packets)
        count += len(received_packets)
    
    cur.close()
    
    avg_datavolume = total_datavolume/count
    print(avg_datavolume)
    
    return avg_datavolume

"""
Calculates the average deviation of a VRUs known position from its actual position in meters.

:param: db_path:    Path to the database which stores the results of the simulation.
    
:return:            The average deviation of a VRUs known position from its actual position in meters.
"""
def calc_pos_diff(db_path): 
    total_pos_diff = 0
    count = 0
    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    
    cur.execute("SELECT v.vectorCount as count, v.vectorSum as sum FROm vector as v where v.vectorName = 'posdiff:vector'")
    pd_tuples = cur.fetchall()
    
    for pd_tuple in pd_tuples:
        count += pd_tuple[0]
        total_pos_diff += pd_tuple[1]
    
    cur.close()     
    
    avg_pos_diff = total_pos_diff/count
    print(avg_pos_diff)
    
    return avg_pos_diff


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-path", type=str, required=True, help="Path to the databse that contains the simulation results.")
    
    subparsers = parser.add_subparsers()
    parser_latency = subparsers.add_parser("lat", help="Average VAM latency across all sent VAMs.")
    parser_latency.set_defaults(func=calc_latency)
    parser_datavolume = subparsers.add_parser("vol", help="Average data volume of VAM related messages.")
    parser_datavolume.set_defaults(func=calc_datavolume)
    parser_pos_diff = subparsers.add_parser("pos", help="Average deviation of a VRUs known position from its actual position.")
    parser_pos_diff.set_defaults(func=calc_pos_diff)
    
    # calc_latency("results/simple_cam_den_20220215-10:44:58/vars_rep_0.vec")
    options = parser.parse_args()
    options.func(options.path)

if __name__ == "__main__":
    main()
    
