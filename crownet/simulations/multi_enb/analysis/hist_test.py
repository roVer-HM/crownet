from crownetutils.analysis.common import Simulation
from crownetutils.analysis.dpmm import MapType
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg
from crownetutils.omnetpp.scave import CrownetSql

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
import numpy as np


def get_density_cfg(base_dir):
    density_cfg = DpmmCfg(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        global_map_csv_name="global.csv",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def main():
    root_d = "/mnt/data1tb/results/multi_enb/final_multi_enb_dev_20230928-09:40:19/"
    sim = Simulation.from_dpmm_cfg(get_density_cfg(root_d))

    sql: CrownetSql = sim.sql
    df = sql.hist_data(
        "World.pNode[%].cellularNic.channelModel[0]", "measuredSinrUl:histogram"
    )

    inf_bin_sum_count = df[df["inf_bin"]]["binValue"].sum()
    if inf_bin_sum_count > 0:
        print("found fliers in inf bins (-inf, inf)")
    df = df[~df["inf_bin"]]
    fig, ax = plt.subplots(1, 1)
    for i, _df in df.groupby(["statId"]):
        if i[0] == 1758:
            continue
        print(i)
        ax.hist(
            _df["lowerEdge"][:-1],
            bins=_df["lowerEdge"],
            weights=_df["binValue"][:-1],
            histtype="step",
        )

    ax.xaxis.set_major_locator(MultipleLocator(5))
    ax.xaxis.set_minor_locator(MultipleLocator(2.5))
    ax.yaxis.set_major_locator(MultipleLocator(100))
    ax.yaxis.set_minor_locator(MultipleLocator(50))
    ax.set_xlim((df["lowerEdge"].min() - 10, df["lowerEdge"].max() + 10))
    ax.set_xlabel("Measured Sinr UL")
    print("hi")
