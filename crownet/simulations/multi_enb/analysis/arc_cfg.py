import importlib
from crownetutils.analysis.dpmm import MapType
import matplotlib.pyplot as plt
import os
from datetime import datetime, timedelta

import pandas as pd
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfgDb, DpmmCfgBuilder


def main():
    builder = DpmmCfgBuilder()
    base = "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/simulation_runs/outputs/"
    out = [
        "./Sample_123_0/final_multi_enb_out",
        "./Sample_41_0/final_multi_enb_out",
        "./Sample_3_0/final_multi_enb_out",
        "./Sample_2_0/final_multi_enb_out",
        "./Sample_122_0/final_multi_enb_out",
        "./Sample_121_0/final_multi_enb_out",
        "./Sample_101_0/final_multi_enb_out",
        "./Sample_61_0/final_multi_enb_out",
        "./Sample_161_0/final_multi_enb_out",
        "./Sample_1_0/final_multi_enb_out",
        "./Sample_81_0/final_multi_enb_out",
        "./Sample_21_0/final_multi_enb_out",
        "./Sample_141_0/final_multi_enb_out",
    ]

    for o in out:
        o = os.path.join(base, o)
        print(f"process: {o}")

        c1 = get_density_cfg(o)
        c2 = get_entropy_cfg(o)

        builder.save_in_root(c1)
        builder.save_in_root(c2)


# todo:
def get_density_cfg(base_dir):
    density_cfg = DpmmCfgDb(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        map_db_name="global_densityMap.db",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def get_entropy_cfg(base_dir):
    density_cfg = DpmmCfgDb(
        base_dir=base_dir,
        hdf_file="entropy_data.h5",
        map_type=MapType.ENTROPY,
        map_db_name="global_entropyMap.db",
        beacon_app_path=None,
        map_app_path="app[2]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def print_entropy_build_performance():
    csv_p = "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/simulation_runs/outputs/Sample_123_0/final_multi_enb_out/time_log.csv"
    data = pd.read_csv(csv_p, delimiter=" ")
    data.columns = ["date", "time", "processed", "all", "percent_done"]
    t = (data["date"] + " " + data["time"]).to_list()
    times = times = [datetime.strptime(l, "%Y-%m-%d %H:%M:%S,%f") for l in t]
    data["t"] = times
    data["delta_t"] = (
        data["t"].diff(1).fillna(timedelta(0)).apply(lambda x: x.components.minutes)
    )

    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    ax.plot(data["processed"], data["delta_t"], label="Time to process 8 nodes")
    ax.set_xlabel("Commulative number of processed nodes")
    ax.set_label("Time in minutes needed.")
    ax.set_title(
        "Procssing time to read from 600GB DB to create Map-HDF file with error values"
    )

    fig.tight_layout()
    p = "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/simulation_runs/outputs/Sample_123_0/final_multi_enb_out/build_performance.png"
    fig.savefig(p)


if __name__ == "__main__":
    # main()
    print_entropy_build_performance()
