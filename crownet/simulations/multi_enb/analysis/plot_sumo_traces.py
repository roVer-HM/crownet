"""Create mobility statistics for sumo simulation

Paper: Adaptive Transmission Rate Control for Decentralized Sensing Applications
"""
from __future__ import annotations
import glob
import os
from argparse import ArgumentParser, Namespace
from typing import List, Protocol
from crownetutils.utils.json import NumpyEncoder
from crownetutils.utils.plot import PlotUtil
from crownetutils.utils.styles import load_matplotlib_style
import matplotlib.pyplot as plt

from crownetutils.sumo.bonnmotion_reader import frame_from_bm
from matplotlib.ticker import MultipleLocator, ScalarFormatter
import numpy as np
import pandas as pd
import json

load_matplotlib_style(os.path.join(os.path.dirname(__file__), "paper_tex.mplstyle"))


class MyMultipleLocator(MultipleLocator):
    def __init__(self, base: float = ...) -> None:
        super().__init__(base)

    def set_params(self, base: float):
        return super().set_params(base)

    def __call__(self):
        return super().__call__()

    def tick_values(self, vmin: float, vmax: float):
        return super().tick_values(vmin, vmax)


class BonnmotionFileFilter:
    def __call__(self, file_paths: List[str]) -> List[str]:
        pass


def all_files(file_paths: List[str]) -> List[str]:
    return file_paths


def get_seeds(path, filter: BonnmotionFileFilter):
    bm_files = glob.glob(f"{path}/**/*.bonnmotion.gz", recursive=True)
    bm_files = filter(bm_files)
    bm = {}
    for b in bm_files:
        bid = int(os.path.basename(b)[0:3])
        if bid not in bm and bid < 10:
            bm[bid] = b
    return bm


def parse_bonnmotion(study_dir, filter: BonnmotionFileFilter):
    bm_files = get_seeds(study_dir, filter)
    data = []

    for bid, b in bm_files.items():
        d = frame_from_bm(b)
        d = d[d["time"] <= 1000.0].copy()
        d["seed"] = bid
        data.append(d)

    data = pd.concat(data, axis=0)
    return data


def get_number_of_nodes(study_dir, cache_path, filter: BonnmotionFileFilter):
    if os.path.exists(cache_path):
        return pd.read_csv(cache_path, index_col=["seed", "time"])

    data = parse_bonnmotion(study_dir, filter)
    node_count: pd.DataFrame = data.groupby(["seed", "time"])["id"].count()
    node_count.to_csv(cache_path)
    stats = node_count[node_count.index.get_level_values("time") > 120].describe()
    stats.to_csv(os.path.join(os.path.dirname(cache_path), "num_nodes_after_spawn.csv"))
    return node_count


def plot_number_of_nodes(node_count: pd.DataFrame):
    colors = plt.get_cmap("tab10")(range(10))

    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    seeds = node_count.index.get_level_values("seed").unique().to_list()
    for seed in seeds:
        _d = node_count.loc[seed]
        ax.plot(_d.index, _d, label=f"Seed {seed}", color=colors[seed])
    ax.grid(visible=True, which="major", axis="both")
    ax.set_ylabel("number of pedestrians")
    ax.set_xlabel("Simulation time in seconds")
    ax.set_xlim(0, 1010)
    ax.set_ylim(0, 820)
    ax.vlines(
        120, 0, 820, colors="darkred", linestyles="dashed", label="No new pedestrians"
    )
    ax.legend(loc="lower right")
    PlotUtil.auto_major_minor_locator(ax)
    ax.text(
        120,
        -25,
        "120",
        transform=ax.transData,
        horizontalalignment="center",
        fontdict=dict(color="darkred"),
    )

    return fig, ax


def get_speed_hist_data(study_dir, cache_path, filter: BonnmotionFileFilter):
    if os.path.exists(cache_path):
        with open(cache_path, "r", encoding="utf-8") as f:
            return json.load(fp=f)

    bm = parse_bonnmotion(study_dir, filter)
    bm = bm[bm["time"] > 120].copy()

    qs = np.percentile(bm["speed"], [25, 75])
    bins = 2 * (qs[1] - qs[0]) * bm.shape[0] ** (1 / 3)
    counts, bins = np.histogram(bm["speed"], bins=int(bins) + 1)

    hist_data = {}
    hist_data["bins"] = bins
    hist_data["total_count"] = counts
    hist_data["describe"] = bm["speed"].describe().to_dict()
    hist_data["faster_than_in_percent"] = {}
    hist_data["seed"] = {}

    for i in [2, 3, 4, 5, 6]:
        hist_data["faster_than_in_percent"][i] = (
            bm[bm["speed"] > i].shape[0] / bm.shape[0] * 100
        )

    for seed, _d in bm.groupby("seed"):
        c, _ = np.histogram(_d["speed"], bins=bins)
        hist_data["seed"][seed] = list(c)

    with open(cache_path, "w", encoding="utf-8") as f:
        json.dump(hist_data, f, cls=NumpyEncoder)

    return get_speed_hist_data(study_dir, cache_path, filter)


def get_trace_length_hist_data(study_dir, cache_path, filter: BonnmotionFileFilter):
    if os.path.exists(cache_path):
        with open(cache_path, "r", encoding="utf-8") as f:
            return json.load(fp=f)

    bm = parse_bonnmotion(study_dir, filter)
    bm = bm.groupby(["seed", "id"])["dist"].sum().to_frame()

    qs = np.percentile(bm["dist"], [25, 75])
    bins = 30
    counts, bins = np.histogram(bm["dist"], bins=int(bins) + 1)

    hist_data = {}
    hist_data["bins"] = bins
    hist_data["total_count"] = counts
    hist_data["describe"] = bm["dist"].describe().to_dict()
    hist_data["faster_than_in_percent"] = {}
    hist_data["seed"] = {}

    for seed, _d in bm.groupby("seed"):
        c, _ = np.histogram(_d["dist"], bins=bins)
        hist_data["seed"][seed] = list(c)

    with open(cache_path, "w", encoding="utf-8") as f:
        json.dump(hist_data, f, cls=NumpyEncoder)

    return get_speed_hist_data(study_dir, cache_path, filter)


class Labeloffset:
    def __init__(self, ax, pos, axis="y"):
        self.ax: plt.Axes = ax
        self.axis = {"y": ax.yaxis, "x": ax.xaxis}[axis]
        self.pos = pos
        ax.callbacks.connect(axis + "lim_changed", self.update)
        ax.figure.canvas.draw()
        self.update(None)

    def update(self, lim):
        fmt = self.axis.get_major_formatter()
        o = fmt.get_offset()
        print(f"offset {o} {self.pos}")
        self.ax.text(
            *self.pos, o, transform=self.ax.transData, fontdict={"size": "small"}
        )


def main(ns: Namespace):
    study_dir = ns.input[0]
    analysis_dir = ns.output

    def file_filter(bm_files: List[str]) -> List[str]:
        seeds = [2, 3, 5, 6, 8, 9]
        ret = []
        for f in bm_files:
            for s in seeds:
                if f"00{s}___muc.bonnmotion.gz" in f:
                    ret.append(f)
        return ret

    node_count = get_number_of_nodes(
        study_dir, os.path.join(analysis_dir, "num_nodes.csv"), file_filter
    )
    hist_data_speed = get_speed_hist_data(
        study_dir, os.path.join(analysis_dir, "speed_hist.json"), file_filter
    )
    hist_data_trace = get_trace_length_hist_data(
        study_dir, os.path.join(analysis_dir, "trace_length_hist.json"), file_filter
    )

    col_pt = 242.67355 * 1.3
    col_cm = col_pt / 2.835 / 10
    col_inch = col_cm / 2.54
    fig = plt.figure(constrained_layout=False, figsize=(col_inch, 9 / 2.54))
    gs1 = fig.add_gridspec(
        nrows=4, ncols=4, top=1.0, right=1.0, bottom=0.31, wspace=0.3, hspace=2.1
    )
    ax_count_ts = fig.add_subplot(gs1[:2, :])
    ax_speed_hist = fig.add_subplot(gs1[2:, 0:2])
    ax_dist_hist = fig.add_subplot(gs1[2:, 2:])

    colors = plt.get_cmap("tab10")(range(10))

    # 1
    seeds = node_count.index.get_level_values("seed").unique().to_list()
    for seed in seeds:
        _d = node_count.loc[seed]
        ax_count_ts.plot(_d.index, _d, label=f"Seed {seed}", color=colors[seed])
    ax_count_ts.grid(visible=True, which="major", axis="both")
    ax_count_ts.set_ylabel("Num nodes")
    ax_count_ts.set_xlabel("Simulation time in seconds")
    ax_count_ts.xaxis.set_label_coords(0.5, -0.26)
    ax_count_ts.set_xlim(0, 1010)
    ax_count_ts.set_ylim(0, 1000)
    ax_count_ts.vlines(
        120, 0, 1000, colors="darkred", linestyles="dashed", label="Stop spawn"
    )
    ax_count_ts.yaxis.set_major_locator(MultipleLocator(400))
    ax_count_ts.xaxis.set_major_locator(MultipleLocator(200))
    ax_count_ts.xaxis.set_minor_locator(MultipleLocator(50))
    ax_count_ts.text(
        120,
        -220,
        "120",
        transform=ax_count_ts.transData,
        horizontalalignment="center",
        fontdict=dict(color="darkred", size="small"),
    )
    ax_count_ts.figure.canvas.draw()  # populate ticks
    for l in ax_count_ts.xaxis.get_majorticklabels():
        if "1000" in l.get_text():
            l.set_horizontalalignment("right")

    # 2
    bins = hist_data_speed["bins"]
    ax_speed_hist.hist(
        bins[:-1],
        bins=bins,
        weights=hist_data_speed["total_count"],
        histtype="stepfilled",
        density=True,
        color="gray",
        alpha=0.30,
        label="All seeds",
    )
    for seed, counts in hist_data_speed["seed"].items():
        ax_speed_hist.hist(
            bins[:-1],
            bins=bins,
            weights=counts,
            histtype="step",
            density=True,
            color=colors[int(seed)],
            label=f"Seed {seed}",
        )
    ax_speed_hist.set_xlim(0, 4.0)
    ax_speed_hist.set_ylabel("Density")
    ax_speed_hist.set_ylim(0, 1.6)
    ax_speed_hist.xaxis.set_major_locator(MultipleLocator(1))
    ax_speed_hist.xaxis.set_minor_locator(MultipleLocator(0.2))
    ax_speed_hist.yaxis.set_major_locator(MultipleLocator(0.5))
    ax_speed_hist.yaxis.set_minor_locator(MultipleLocator(0.1))

    ax_speed_hist.set_xlabel("Node speed in m/s")
    ax_speed_hist.xaxis.set_label_coords(0.5, -0.24)
    view = 100.0 - hist_data_speed["faster_than_in_percent"]["4"]
    ax_dist_hist.text(
        3.95,
        1.5,
        f"Clipped view:\n {view:.2f}\% of data \n $\leq$ 4.0 m/s",
        transform=ax_speed_hist.transData,
        fontdict={"size": "small"},
        verticalalignment="top",
        horizontalalignment="right",
    )

    # 3
    bins = hist_data_trace["bins"]
    ax_dist_hist.hist(
        bins[:-1],
        bins=bins,
        weights=hist_data_trace["total_count"],
        histtype="stepfilled",
        density=True,
        color="gray",
        alpha=0.30,
        label="All seeds",
    )
    for seed, counts in hist_data_trace["seed"].items():
        ax_dist_hist.hist(
            bins[:-1],
            bins=bins,
            weights=counts,
            histtype="step",
            density=True,
            color=colors[int(seed)],
            label=f"Seed {seed}",
        )

    s = ScalarFormatter()
    s.set_scientific(True)
    s.set_powerlimits((-3, 1))
    ax_dist_hist.yaxis.set_major_formatter(s)
    ax_dist_hist.yaxis.offsetText.set_visible(False)
    lo = Labeloffset(ax_dist_hist, pos=(5, 0.0019))
    ax_dist_hist.xaxis.set_major_locator(MultipleLocator(500))
    ax_dist_hist.xaxis.set_minor_locator(MultipleLocator(100))
    ax_dist_hist.yaxis.set_major_locator(MultipleLocator(1e-3))
    ax_dist_hist.yaxis.set_minor_locator(MultipleLocator(0.2e-3))
    ax_dist_hist.set_xlabel("Trace length in m")
    ax_dist_hist.xaxis.set_label_coords(0.5, -0.24)

    h, l = ax_dist_hist.get_legend_handles_labels()
    l = ["All seeds", "Seed 1", "Seed 2", "Seed 3", "Seed 4", "Seed 5", "Seed 6"]
    h = np.array(h)[[0, 4, 1, 5, 2, 6, 3]].tolist()
    l = np.array(l)[[0, 4, 1, 5, 2, 6, 3]].tolist()
    _h, _l = ax_count_ts.get_legend_handles_labels()
    h.append(_h[-1])
    l.append(_l[-1])
    fig.legend(
        h,
        l,
        ncol=4,
        loc="lower center",
        fontsize="small",
        columnspacing=1,
        frameon=False,
        borderpad=0.1,
        labelspacing=0.3,
        handletextpad=0.2,
    )
    fig.savefig(os.path.join(analysis_dir, "sumo_trace_plot.pdf"))


if __name__ == "__main__":
    p = ArgumentParser(prog="Create Sumo Trace Plots")
    p.add_argument(
        "-o", "--output-dir", dest="output", help="Directory to place generated files"
    )
    p.add_argument("input", nargs=1, help="Directory containing simulation runs")
    ns = p.parse_args()
    main(ns)
