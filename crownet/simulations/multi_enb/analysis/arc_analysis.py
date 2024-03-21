from abc import ABC
from dataclasses import dataclass
from itertools import product, repeat
from functools import partial
from logging import DEBUG
import os
import shutil
from typing import Any, List
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, DpmmCfgBuilder
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.helper import save_box_plot_to_hdf
from crownetutils.analysis.hdf_providers.map_error_data import MapCountError
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.utils.parallel import ExecutionItem, run_items
from crownetutils.utils.styles import load_matplotlib_style
from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.offsetbox import AnnotationBbox, TextArea
from matplotlib.patches import BoxStyle
import numpy as np
import pandas as pd
from crownetutils.analysis.common import (
    CacheLoader,
    IndexedSimulation,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)

from crownetutils.utils.plot import (
    GridPlot,
    GridPlotIter,
    MulticolorPatch,
    MulticolorPatchHandler,
    PlotUtil,
    calc_box_stats,
    combine_images,
)
from crownetutils.utils.logging import logger, timing

from matplotlib.ticker import (
    FixedFormatter,
    FixedLocator,
    MultipleLocator,
    ScalarFormatter,
)

load_matplotlib_style(os.path.join(os.path.dirname(__file__), "paper_tex.mplstyle"))

STUDY_DIR = "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/"
OUT_DIR = "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/paper_plots_merged"


class FilteredSeedGroupFactory:

    """Only select specific seeds from the provided SqucStudy """

    def __init__(self, seeds: List[int], sim_per_group=20, offset=0) -> None:
        self.seed = seeds
        self.sim_per_group = sim_per_group
        self.offset = offset

    def __call__(
        self, sim: Simulation, data: List[Simulation], **kwds: Any
    ) -> SimulationGroup:
        attr = kwds.get("attr", {})

        data_filtered: List[Simulation] = list(np.array(data)[self.seed])
        sim: Simulation = data_filtered[0]
        entropy_cfg = sim.run_context.ini_get("*.misc[*].app[2].app.mapCfg")
        ttl = int(entropy_cfg.as_dict["cellAgeTTL"].replace(".0s", "").replace("s", ""))
        order = entropy_cfg.as_dict["idStreamType"]
        sg = SimulationGroup(
            group_name=f"{order}_ttl{ttl}", data=data_filtered, attr=attr
        )
        return sg


@dataclass
class SeedLocation:

    seed_list: List[int]  # list of seeds to use for that study
    study_dir: str  # path to suqc directory
    offset: int = 0  # offset applied if multiple studies are combined
    group_size: int = 20  # number of seeds


def build_run_map(output_path, seed_location_list: List[SeedLocation]):

    r = RunMap(output_path)
    for seed_location in seed_location_list:
        f = FilteredSeedGroupFactory(seed_location.seed_list)
        study = SuqcStudy(base_path=seed_location.study_dir)
        study.update_run_map(
            run_map=r,
            sim_per_group=seed_location.group_size,
            id_offset=seed_location.offset,
            sim_group_factory=f,
        )

    return r


def get_or_create_run_map():

    seed_location_list = [
        SeedLocation([5, 6, 8, 9], STUDY_DIR),
    ]
    run_map = RunMap.load_or_create(
        create_f=partial(build_run_map, seed_location_list=seed_location_list),
        output_path=OUT_DIR,
    )
    return run_map


def get_enb_colors(r: RunMap, with_zero: bool = False):
    sim = r[0][0]
    pos = NodePositionWithRsdHdf(sim, hdf_path=sim.dpmm_cfg.position.path)
    colors = pos.enb_colors(with_zero=with_zero)
    return colors


def save_old(fig, path):
    """Save figure but keep old version"""

    os.makedirs(os.path.dirname(path), exist_ok=True)
    try:
        shutil.copyfile(path, f"{path}.old")
    except:
        pass
    fig.savefig(path)


def get_box_props(rsd_color):
    flierprops = dict(
        marker="2",
        markerfacecolor=rsd_color,
        markeredgecolor=rsd_color,
        markersize=2,
        linestyle="none",
    )
    boxprops = dict()
    medianprops = dict(linestyle="-", linewidth=1.0, color="darkred")
    meanprops = dict(linestyle="dotted", linewidth=1, color="black")
    return dict(
        flierprops=flierprops,
        medianprops=medianprops,
        meanprops=meanprops,
        boxprops=boxprops,
    )


class ThroughputData(CacheLoader):
    @staticmethod
    @timing
    def _load(isim: IndexedSimulation, rsd_list, bin_size):
        sim = isim.sim
        ntx: NodeTxData = NodeTxData(sim.dpmm_cfg.node_tx.path)

        tp_target_rate = ntx.get_target_rates(1 / 8)  # B/s
        tp_target_rate = {"e_map": tp_target_rate["e_map"]}  # only one app
        df: List[pd.DataFrame] = []
        for rsd in rsd_list:
            tp = ntx.get_tx_throughput_diff_by_app(
                target_rates=tp_target_rate,
                bin_size=bin_size,
                throughput_unit=1000,
                serving_enb=rsd,
            )
            tp["rsd"] = rsd
            df.append(tp)
        df = pd.concat(df, axis=0)
        df["run_id"] = isim.global_id
        df["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        return df

    def __init__(self, run_map: RunMap) -> None:
        super().__init__(run_map, run_map.path("insertion_order_throughput_by_ttl.h5"))

    def load(self):
        items: List[ExecutionItem] = []
        rsd_list = list(range(1, 16))
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(
                    ExecutionItem(
                        fn=self._load,
                        kwargs=dict(isim=idx_sim, rsd_list=rsd_list, bin_size=10.0),
                    )
                )
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df: pd.DataFrame = pd.concat(ret, axis=0)

        bplot = df.groupby(["ttl", "rsd"])[["e_map", "diff_e_map"]].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )
        save_box_plot_to_hdf(bbox=bplot["e_map"], hdf=self.hdf, base_name="throughput_")
        save_box_plot_to_hdf(
            bbox=bplot["diff_e_map"], hdf=self.hdf, base_name="throughput_diff_"
        )


def throughput_by_enb_for_all_ttls(run_map: RunMap):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=75)
    fig, ax = plt.subplots(1, 1, figsize=fig_size)

    box_df = ThroughputData(run_map).get()
    boxes = box_df.frame("throughput_boxes")
    fliers = box_df.frame("throughput_fliers").dropna()
    width = 2.5
    fixed_lbl = [0]  # dummy value

    # sort by rsd and then by ttl (as index)
    boxes = boxes.reorder_levels((1, 0)).sort_index()
    # get rsd values sorted by ascending number of nodes
    rsd_order = (
        NumGlobalCount(r)
        .get()
        .frame()["mean"]
        .reset_index()
        .sort_values("mean")["rsd_id"]
        .values
    )
    # reorder boxes to match that oder
    boxes = boxes.loc[rsd_order]

    for i, (idx, box) in enumerate(boxes.iterrows()):
        b = box.to_dict()
        if idx[1] == 15:
            positions = -width + int(i / 3) * 10
        elif idx[1] == 60:
            positions = int(i / 3) * 10
            fixed_lbl.append(idx[0])
        else:
            positions = width + int(i / 3) * 10
        try:
            f = fliers.loc[pd.IndexSlice[idx[1], idx[0]]].values[0]
        except KeyError:
            f = []
        b["fliers"] = f
        box_artist = ax.bxp(
            [b],
            positions=[positions],
            widths=width,
            showmeans=True,
            meanline=True,
            patch_artist=True,
            showfliers=True,
            **get_box_props(colors[idx[0]]),
        )
        for patch in box_artist["boxes"]:
            patch.set_facecolor(colors[idx[0]])

    ax.hlines(
        125,
        xmin=-6,
        xmax=146,
        color="darkred",
        linestyles="dashed",
        label="Shared max rate",
        zorder=0,
    )

    ax.set_xlim(-6, 146)
    ax.xaxis.set_major_locator(MultipleLocator(10))
    ax.xaxis.set_major_formatter(FixedFormatter(fixed_lbl))
    ax.set_xlabel("eNB")
    ax.set_ylabel("Throughput in KB/s")
    ax.set_ylim(-5, 170)
    ax.yaxis.set_major_locator(MultipleLocator(50))
    ax.yaxis.set_minor_locator(MultipleLocator(10))

    ax.annotate(
        text="ttl15\nttl60\nttl3600",
        xy=(1, 3),
        xytext=(7.25, -50.55),
        xycoords="data",
        fontsize="xx-small",
        ha="right",
        rotation=80,
    )

    ax.yaxis.set_label_coords(-0.08, 0.5, transform=ax.transAxes)

    h = [
        Line2D([0], [0], color="darkred"),
        Line2D([0], [0], color="black", linestyle="dotted"),
    ]
    l = ["Median", "Average"]

    fig.legend(
        h,
        l,
        loc="lower right",
        handler_map={},
        bbox_to_anchor=(1.015, 0.00),
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
        columnspacing=1.0,
        handlelength=1.0,
    )

    h = [
        Line2D(
            [0],
            [0],
            label="fliers",
            marker="2",
            markersize=4,
            markeredgecolor="black",
            markerfacecolor="black",
            linestyle="",
        ),
        Line2D([0], [0], color="darkred", linestyle="dashed"),
    ]
    l = ["Flieres", "Shared bandwidth"]

    fig.legend(
        h,
        l,
        loc="lower left",
        handler_map={},
        bbox_to_anchor=(0.2, 0.00),
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
        columnspacing=1.0,
        handlelength=1.0,
    )

    fig.tight_layout()

    fig.subplots_adjust(
        left=0.12, bottom=0.22, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )

    save_old(fig, run_map.path("insertion_order_throughput.pdf"))


class NumGlobalCount(CacheLoader):
    def __init__(self, run_map: RunMap, root_group: str = "root") -> None:
        super().__init__(
            run_map,
            run_map.path("insertion_order_glb_mean_number_nodes_per_enb.h5"),
            root_group,
        )

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df: pd.DataFrame = pd.concat(ret, axis=0)
        df = df.groupby(["ttl", "rsd_id"])["map_glb_count"].mean()
        df = df.groupby("rsd_id").agg(["mean", "std"])
        self.hdf.write_frame(self.hdf.group, frame=df)

    @staticmethod
    def _load(isim: IndexedSimulation):
        map_err = MapCountError(hdf_path=isim.sim.dpmm_cfg.map_count_error.path)
        df = map_err.hdf_map_measure_rsd_local.select(columns=["map_glb_count"])
        df["run_id"] = isim.global_id
        df["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        return df


class EnbRBData(CacheLoader):
    def __init__(self, run_map: RunMap) -> None:
        super().__init__(run_map, run_map.path("insertion_order_rb_usage_by_ttl.h5"))
        self.hdf = BaseHdfProvider(self.cache_path)

    def cache_exists(self) -> bool:
        return super().cache_exists() and all(
            [g in ["boxes", "fliers", "hist"] for g in self.hdf.get_groups()]
        )

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df: pd.DataFrame = pd.concat(ret, axis=0)

        bin_count = 26  # 25 RBs pluse zero RB scheduled.
        bin_edges = np.arange(bin_count + 1)  # one edge more than bins
        hist: List[pd.DataFrame] = []
        for key, _df in df.groupby(["ttl", "enb_idx"]):
            bins, _edge = np.histogram(_df["value"], bins=bin_edges)
            idx = (
                np.concatenate([np.repeat(key, bin_count), np.arange(bin_count)])
                .reshape((-1, bin_count))
                .T
            )
            idx = pd.MultiIndex.from_frame(
                pd.DataFrame(idx), names=["ttl", "enb_idx", "bin"]
            )
            hist.append(pd.Series(bins, name="bin_count", index=idx))

        hist = pd.concat(hist, axis=0)
        self.hdf.write_frame(group="hist", frame=hist)
        bplot = df.groupby(["ttl", "enb_idx"])["value"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )

        save_box_plot_to_hdf(bplot, self.hdf)

    @timing
    @staticmethod
    def _load(isim: IndexedSimulation):
        cfg: DpmmCfg = isim.sim.dpmm_cfg
        df = BaseHdfProvider(cfg.enb_rb.path, group=cfg.enb_rb.group)
        if df.hdf_file_exists:
            df = df.frame()
        else:
            raise FileNotFoundError(df._hdf_path)
        df["run_id"] = isim.global_id
        df["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        return df


class NumberOfCellsInMap(CacheLoader):
    @classmethod
    def insertion_order(cls, r: RunMap):
        return cls(run_map=r, cache_path=r.path("insertion_order_num_cells_in_map"))

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df: pd.DataFrame = pd.concat(ret, axis=0)
        bplot = df.groupby(["ttl", "enb"])["numberOfCells"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )

        print("df")

    @staticmethod
    def _load(isim: IndexedSimulation):
        cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        df = pd.read_csv(cfg.map_size.path, index_col=None)
        pos = NodePositionWithRsdHdf(isim.sim, cfg.position.path)
        df["run_id"] = isim.global_id
        df["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        df = pos.merge_rsd_id_on_host_time_interval(
            df,
            time_col="simtime",
            host_id_col="hostId",
            append_interval=False,
            rsd_col_name="enb",
        )
        return df


def rb_utilization_by_enb_for_all_ttls_histogram(run_map: RunMap):

    colors = get_enb_colors(run_map)

    fig_size = PlotUtil.fig_size_mm(width=90, height=25)
    box_df: BaseHdfProvider = EnbRBData(run_map).get()
    fig, axes = plt.subplots(
        nrows=3, ncols=15, figsize=fig_size, sharex=True, sharey=True
    )
    ttl_map = {0: 15, 1: 60, 2: 3600}
    ax_order = axes[::-1, 0].T
    for enb in range(ax_order.shape[0]):
        c = colors[enb + 1]
        for ttl in range(ax_order.shape[1]):
            ax = ax_order[enb, ttl]
            df = box_df.select(
                key="hist",
                where="enb_idx={enb} and ttl={ttl}".format(enb=enb, ttl=ttl_map[ttl]),
            )
            ax.hist(
                x=np.arange(26),
                bins=np.arange(27),
                weights=df.values,
                density=True,
                color=c,
            )
            # ax.set_axis_off()
            ax.set_xticks([])
            ax.set_yticks([])
            for side in ["top", "bottom", "left", "right"]:
                ax.spines[side].set(lw=0.2)

    fig.subplots_adjust(
        left=0.0, bottom=0.0, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )

    save_old(fig, run_map.path("hist.png"))
    print("hi")


# Base station utilization over all seeds.
# x base station id
# y box plot of combined data
def rb_utilization_by_enb_for_all_ttls_bar_chart(run_map: RunMap):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=55)
    fig, ax = plt.subplots(1, 1, figsize=fig_size)

    box_df = EnbRBData(run_map).get()
    boxes = (
        box_df.frame("boxes")
        .reset_index()
        .sort_values(["enb_idx", "ttl"])
        .to_dict("records")
    )
    width = 2.5
    fixed_lbl = [0]

    enb_order = NumGlobalCount(r).get().frame()["mean"]
    # sort ascending by num nodes and ttl
    boxes.sort(key=lambda x: (enb_order.loc[x["enb_idx"] + 1], x["ttl"]))
    ax.grid(visible=True, which="major", axis="y", zorder=1)
    for idx, b in enumerate(boxes):
        if b["ttl"] == 15:
            positions = -width + int(idx / 3) * 10
        elif b["ttl"] == 60:
            positions = int(idx / 3) * 10
            fixed_lbl.append(b["enb_idx"] + 1)
        else:
            positions = width + int(idx / 3) * 10

        ax.bar(
            positions,
            b["mean"],
            width=width,
            color=colors[b["enb_idx"] + 1],
            edgecolor="black",
            linewidth=0.5,
        )
        ax.hlines(
            y=b["med"],
            xmin=positions - width / 2,
            xmax=positions + width / 2,
            colors="darkred",
        )

    ax.set_xlim(-6, 146)
    ax.xaxis.set_major_locator(MultipleLocator(10))
    # ax.xaxis.set_major_formatter(FixedFormatter(range(0, 16 , 1)))
    ax.xaxis.set_major_formatter(FixedFormatter(fixed_lbl))
    ax.set_xlabel("eNB")
    ax.set_ylabel("RBs scheduled")
    ax.set_ylim(0, 15)
    ax.yaxis.set_major_locator(MultipleLocator(5))
    ax.yaxis.set_minor_locator(MultipleLocator(1))

    ax.annotate(
        text="ttl15\nttl60\nttl3600",
        xy=(1, 3),
        xytext=(7.25, -6.0),
        xycoords="data",
        fontsize="xx-small",
        ha="right",
        rotation=80,
    )

    PlotUtil.move_last_y_ticklabel_down(ax, "15")

    h = [MulticolorPatch(list(colors.values())), Line2D([0], [0], color="darkred")]
    l = ["Average num RBs", "Median num  RBs"]

    fig.legend(
        h,
        l,
        loc="lower right",
        handler_map={MulticolorPatch: MulticolorPatchHandler()},
        bbox_to_anchor=(1.015, 0.00),
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
    )

    fig.tight_layout()

    fig.subplots_adjust(
        left=0.12, bottom=0.3, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )

    save_old(fig, run_map.path("insertion_order_rb_utilization.pdf"))


if __name__ == "__main__":
    logger.setLevel(DEBUG)
    r = get_or_create_run_map()
    # rb_utilization_by_enb_for_all_ttls_histogram(r)

    # rb_utilization_by_enb_for_all_ttls_bar_chart(r)
    # throughput_by_enb_for_all_ttls(r)
    NumberOfCellsInMap.insertion_order(r).get()
    print("hi")
