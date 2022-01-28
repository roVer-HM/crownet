import os 
import time
import sqlite3
import argparse

from datetime import datetime
from _datetime import timedelta

# Start time of the simulation
time_start = "2020-06-01 12:35:00"

# Epoch start for genDeltaTime calculation
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
def calc_latency_ms(db_path):
    total_lat = 0
    count = 0
    
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    cur.row_factory = lambda cursor, row: row[0]
    
    cur.execute("SELECT v.vectorId FROM vector as v WHERE v.vectorName = 'reception:vector(vamGenerationDeltaTime)'")
    v_ids = cur.fetchall()
    
    for v_id in v_ids:
        cur.row_factory = None
        cur.execute("SELECT vd.simtimeRaw/1000000000 as simTimeMs, vd.value FROM vectorData as vd WHERE vd.vectorId = " + str(v_id))
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

def calc_datavolume():
    return true

def get_pos_diff():
    return true

def main():
    calc_latency_ms("results/simple_cam_den_20220120-17:27:23/vars_rep_0.vec")

if __name__ == "__main__":
    main()
    
