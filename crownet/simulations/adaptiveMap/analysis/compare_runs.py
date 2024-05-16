import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

from crownetutils.analysis.common import RunMap
from crownetutils.analysis.omnetpp import OppAnalysis


def main():

    new_run = RunMap.load_from_json(
        "/mnt/data1tb/results/arc-dsa_single_cell/study_out/s0/compare_out/run_map.json"
    )
    old_run = RunMap.load_from_json(
        "/mnt/data1tb/results/_dyn_tx/s1_corridor_member_estimate_cmp/compare_out/run_map.json"
    )

    r1 = OppAnalysis.run_collect_vec_info(
        new_run,
        pool_size=40,
        module_name="World.misc[%].app[1].app",
        vector_name="packetSent:vector(packetBytes)",
    )
    r1 = (
        r1[["vec_idx", "vectorCount", "vectorSum", "group", "run_id"]]
        .set_index(["group", "run_id", "vec_idx"])
        .sort_index()
    )
    r1 = r1.set_axis(["pkt_count_1", "byte_sum_1"], axis=1)
    r2 = OppAnalysis.run_collect_vec_info(
        old_run,
        module_name="World.misc[%].app[1].app",
        vector_name="packetSent:vector(packetBytes)",
    )
    r2 = (
        r2[["vec_idx", "vectorCount", "vectorSum", "group", "run_id"]]
        .set_index(["group", "run_id", "vec_idx"])
        .sort_index()
    )
    r2 = r2.set_axis(["pkt_count_2", "byte_sum_2"], axis=1)
    r = pd.concat([r1, r2], axis=1, ignore_index=False)
    r["diff_pkt_count"] = r["pkt_count_1"] - r["pkt_count_2"]
    r["diff_byte_sum"] = r["byte_sum_1"] - r["byte_sum_2"]

    r["x"] = np.tile(np.arange(8000), 3)

    fig, axes = plt.subplots(3, 1, sharex=True)
    for i, (idx, df) in enumerate(r.groupby("group")):
        ax: plt.Axes = axes[i]
        ax.scatter(df["x"], df["diff_byte_sum"], color="black", label="packet diff")
        m = df["diff_byte_sum"].mean()
        ax.hlines(m, 0, 8000, colors="red", label="mean diff")

    x = r.loc[pd.IndexSlice["ConstantRate-", 40]].agg(["sum", "count"])

    print("hi")


if __name__ == "__main__":
    main()
