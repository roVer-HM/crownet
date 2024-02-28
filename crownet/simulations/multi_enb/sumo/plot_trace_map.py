

import os
from crownetutils.sumo.bonnmotion_reader import frame_from_bm
from crownetutils.sumo.plot_sumo_traces import plot_trace
from matplotlib import pyplot as plt
import pandas as pd


def main(trace_file, enb_file):
    traces = frame_from_bm(trace_file)
    enb = pd.read_csv(enb_file, comment="#")
    fig, ax = plt.subplots(1,1, figsize=(16,9))
    plot_trace(ax, traces, enb, inner_r=650)
    fig.tight_layout()
    fig.savefig("/tmp/foo.png")

if __name__ == "__main__":
    trace_file = os.path.abspath(
        os.path.join(
            "munich/muc_cleaned/output/muc_steady_state_60min_cleaned",
            "bonnmotion",
            "000___muc.bonnmotion.gz"
        )
    )
    enb_file = os.path.abspath(
        "../enb_position_hex_muc_clean.csv"
    )
    main(trace_file, enb_file)