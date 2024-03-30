from __future__ import annotations

from abc import ABC
from dataclasses import dataclass, field
from itertools import product, repeat
from functools import partial
from logging import DEBUG
import os
import shutil
from typing import Any, List
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, DpmmCfgBuilder
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.helper import save_box_plot_to_hdf
from crownetutils.analysis.hdf_providers.map_age_over_distance import (
    MapMeasurementsAgeOverDistance,
    MapSizeAndAgeOverDistance,
)
from crownetutils.analysis.hdf_providers.map_error_data import MapCountError
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.utils.parallel import ExecutionItem, run_items
from crownetutils.utils.styles import load_matplotlib_style
from matplotlib import pyplot as plt
import matplotlib
from matplotlib.gridspec import GridSpec
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
    print(f"save {path}")
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


@dataclass
class NumGlobalCount(CacheLoader):
    """Number of nodes per RSD (Average over Seeds)"""

    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        obj = cls(
            run_map,
            cache_path=run_map.path("insertion_order_glb_mean_number_nodes_per_enb.h5"),
        )
        obj.check()
        return obj

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


@dataclass
class EnbRBData(CacheLoader):
    @classmethod
    def insertion_order(cls, run_map: RunMap) -> EnbRBData:
        obj = cls(run_map, run_map.path("insertion_order_rb_usage_by_ttl.h5"))
        obj.check()
        return obj

    hdf_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_hist: BaseHdfProvider = CacheLoader.shared_hdf_field()

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
        for key, _df in df.groupby(["ttl", "rsd_id"]):
            bins, _edge = np.histogram(_df["value"], bins=bin_edges)
            idx = (
                np.concatenate([np.repeat(key, bin_count), np.arange(bin_count)])
                .reshape((-1, bin_count))
                .T
            )
            idx = pd.MultiIndex.from_frame(
                pd.DataFrame(idx), names=["ttl", "rsd_id", "bin"]
            )
            hist.append(pd.Series(bins, name="bin_count", index=idx))

        hist = pd.concat(hist, axis=0)
        self.hdf.write_frame(group="hist", frame=hist)
        bplot = df.groupby(["ttl", "rsd_id"])["value"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )

        save_box_plot_to_hdf(bplot, self.hdf)

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
        df = df.reset_index().rename(columns={"enb_idx": "rsd_id"})
        df["rsd_id"] += 1
        df = df.set_index(["rsd_id", "time"])
        return df


@dataclass
class NumberOfCellsInMap(CacheLoader):
    @classmethod
    def insertion_order(cls, r: RunMap):
        c = cls(run_map=r, cache_path=r.path("insertion_order_num_cells_in_map.h5"))
        c.check()
        return c

    # helpers for ease access to specific group
    hdf_glb_all_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_all_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_desc_all: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_desc_all: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_all_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_all_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_desc_after_120: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_after_120_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_after_120_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df_nodes = [i[0] for i in ret]
        df_nodes: pd.DataFrame = pd.concat(df_nodes, axis=0)
        after_120 = df_nodes["simtime"] > 120
        # describe
        self.save_hdf(
            self.hdf_nodes_desc_all,
            df_nodes.groupby(["ttl", "enb"])["numberOfCells"].describe(),
        )
        self.save_hdf(
            self.hdf_nodes_desc_after_120,
            df_nodes[after_120].groupby(["ttl", "enb"])["numberOfCells"].describe(),
        )

        # box plots
        df_glb_bplot = df_nodes.groupby(["ttl", "enb"])["numberOfCells"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )
        save_box_plot_to_hdf(df_glb_bplot, hdf=self.hdf, base_name="nodes_all_")

        df_glb_bplot = (
            df_nodes[after_120]
            .groupby(["ttl", "enb"])["numberOfCells"]
            .agg(calc_box_stats(use_mpl_name=True, include_mean=True))
        )
        save_box_plot_to_hdf(df_glb_bplot, hdf=self.hdf, base_name="nodes_after_120_")

        df_glb = [i[1] for i in ret]
        df_glb: pd.DataFrame = pd.concat(df_glb, axis=0)

        after_120 = df_glb["simtime"] > 120
        # describe
        self.save_hdf(
            self.hdf_glb_desc_all, df_glb.groupby("ttl")["numberOfCells"].describe()
        )
        self.save_hdf(
            self.hdf_glb_desc_after_120,
            df_glb[after_120].groupby("ttl")["numberOfCells"].describe(),
        )

        # box plots
        df_glb_bplot = df_glb.groupby(["ttl"])["numberOfCells"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )
        save_box_plot_to_hdf(df_glb_bplot, hdf=self.hdf, base_name="glb_all_")

        df_glb_bplot = (
            df_glb[after_120]
            .groupby(["ttl"])["numberOfCells"]
            .agg(calc_box_stats(use_mpl_name=True, include_mean=True))
        )
        save_box_plot_to_hdf(df_glb_bplot, hdf=self.hdf, base_name="glb_after_120_")

    @staticmethod
    def _load(isim: IndexedSimulation):
        cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        df_nodes = pd.read_csv(cfg.map_size.path, index_col=None)
        pos = NodePositionWithRsdHdf(isim.sim, cfg.position.path)
        df_nodes["run_id"] = isim.global_id
        df_nodes["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        df_nodes = pos.merge_rsd_id_on_host_time_interval(
            df_nodes,
            time_col="simtime",
            host_id_col="hostId",
            append_interval=False,
            rsd_col_name="enb",
        )
        df_glb = pd.read_csv(cfg.map_size_global.path, index_col=None)
        df_glb["run_id"] = isim.global_id
        df_glb["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))
        return df_nodes, df_glb


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


@dataclass
class MapAgeOverDistance(CacheLoader):

    hdf_age_over_dist_rsd_mean_of_means: BaseHdfProvider = (
        CacheLoader.shared_hdf_field()
    )
    hdf_age_over_dist_rsd_mean_of_mins: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_age_over_dist_rsd_min_of_mins: BaseHdfProvider = CacheLoader.shared_hdf_field()

    @classmethod
    def insertion_order(cls, run_map: RunMap):
        obj = cls(run_map, run_map.path("insertion_order_map_age_over_distance.h5"))
        obj.check()
        return obj

    def load(self):
        items: List[ExecutionItem] = []
        rsd_list = list(range(1, 16))
        for idx_sim in self.run_map.iter_all_sim():
            if "insertionOrder" in idx_sim.group_name:
                items.append(
                    ExecutionItem(
                        fn=self._load,
                        kwargs=dict(isim=idx_sim, rsd_list=rsd_list),
                    )
                )
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        mean_of_means: List[pd.DataFrame] = [r[0] for r in ret]
        df: pd.DataFrame = pd.concat(mean_of_means, axis=0)
        self.save_hdf(self.hdf_age_over_dist_rsd_mean_of_means, frame=df)
        mean_of_mins: List[pd.DataFrame] = [r[1] for r in ret]
        df: pd.DataFrame = pd.concat(mean_of_mins, axis=0)
        self.save_hdf(self.hdf_age_over_dist_rsd_mean_of_mins, frame=df)

        min_of_mins: List[pd.DataFrame] = [r[2] for r in ret]
        df: pd.DataFrame = pd.concat(min_of_mins, axis=0)
        self.save_hdf(self.hdf_age_over_dist_rsd_min_of_mins, frame=df)

    @staticmethod
    def _load(isim: IndexedSimulation, rsd_list):
        e_cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        mm = MapSizeAndAgeOverDistance(  # <<< use this
            hdf_path=e_cfg.map_size_and_age_over_distance.path
        )
        metric_id = mm.metric_map["age_of_information"]
        df = mm.hdf_rsd.select(where=f"metric={metric_id}", columns=["mean"])
        df = (
            df.groupby(["rsd", "dist_left"])[["mean"]]
            .agg(["mean", "std"])
            .set_axis(["mean_age", "mean_age_std"], axis=1)
        )
        df.index.names = ["rsd", "dist"]

        df["run_id"] = isim.global_id
        df["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))

        df2 = mm.hdf_rsd.select(where=f"metric={metric_id}", columns=["min"])
        df2 = (
            df2.groupby(["rsd", "dist_left"])[["min"]]
            .agg(["mean", "std"])
            .set_axis(["mean_age", "mean_age_std"], axis=1)
        )
        df2.index.names = ["rsd", "dist"]

        df2["run_id"] = isim.global_id
        df2["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))

        df3 = mm.hdf_rsd.select(where=f"metric={metric_id}", columns=["min"])
        df3 = (
            df3.groupby(["rsd", "dist_left"])[["min"]]
            .min()
            .set_axis(["min_age"], axis=1)
        )
        df3.index.names = ["rsd", "dist"]

        df3["run_id"] = isim.global_id
        df3["ttl"] = int(isim.group_name.replace("insertionOrder_ttl", ""))

        return df, df2, df3


def sort_boxes_by_node_count(boxes, run_map: RunMap) -> pd.DataFrame:
    # sort by rsd and then by ttl (as index)
    boxes = boxes.reorder_levels((1, 0)).sort_index()
    # get rsd values sorted by ascending number of nodes
    rsd_order = (
        NumGlobalCount.insertion_order(run_map)
        .get()
        .frame()["mean"]
        .reset_index()
        .sort_values("mean")["rsd_id"]
        .values
    )
    # reorder boxes to match that oder
    boxes = boxes.loc[rsd_order]
    return boxes


def add_boxes_to_axes(ax: plt.Axes, boxes: pd.DataFrame, fliers: pd.DataFrame, colors):

    width = 2.5
    fixed_lbl = [0]  # dummy value

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

    return width, fixed_lbl


def plot_map_size(run_map: RunMap):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=65)
    fig, axes = plt.subplots(ncols=1, nrows=3, figsize=fig_size, sharex=True)
    num_cells = NumberOfCellsInMap.insertion_order(run_map)

    boxes = num_cells.hdf_nodes_after_120_boxes.frame()
    fliers = num_cells.hdf_nodes_after_120_fliers.frame()

    boxes = sort_boxes_by_node_count(boxes=boxes, run_map=run_map)
    fixed_lbl = boxes.index.get_level_values(0).unique().values
    for i, (idx, box) in enumerate(boxes.iterrows()):
        enb, ttl = idx
        b = box.to_dict()
        b["fliers"] = fliers.loc[pd.IndexSlice[ttl, enb]].values[0]
        ax: plt.Axes = axes[int(i % 3)]
        color = colors[enb]
        pos = int(i / 3) * 10
        # ax.scatter(pos, b["med"], color=color, marker=".")

        box_artist = ax.bxp(
            [b],
            positions=[pos],
            widths=5,
            showmeans=True,
            meanline=True,
            patch_artist=True,
            showfliers=True,
            **get_box_props(color),
        )
        for patch in box_artist["boxes"]:
            patch.set_facecolor(color)

    for i, ax in enumerate(axes):
        ax.set_xlim(-6, 146)
        ax.xaxis.set_major_locator(FixedLocator((range(0, 160, 10))))
        ax.xaxis.set_major_formatter(FixedFormatter(fixed_lbl))
        ax.set_xlabel("eNB")
        ax.set_ylabel("")
        if i == 0:
            ax.text(
                -5,
                550,
                "ttl 15",
                fontdict=dict(size="small"),
                transform=ax.transData,
                ha="left",
                va="top",
            )
            ax.set_ylim(0, 600),
            ax.yaxis.set_major_locator(MultipleLocator(500))
            ax.yaxis.set_minor_locator(MultipleLocator(100))
            ax.set_xlabel("")
        elif i == 1:
            ax.text(
                -5,
                1700,
                "ttl 60",
                fontdict=dict(size="small"),
                transform=ax.transData,
                ha="left",
                va="top",
            )
            ax.set_ylim(0, 1800),
            ax.yaxis.set_major_locator(MultipleLocator(1000))
            ax.yaxis.set_minor_locator(MultipleLocator(200))
            ax.set_xlabel("")
            ax.set_ylabel("Number of Map cells")
            ax.yaxis.set_label_coords(x=0.05, y=0.58, transform=fig.transFigure)
        else:
            ax.text(
                -5,
                25000,
                "ttl 3600",
                fontdict=dict(size="small"),
                transform=ax.transData,
                ha="left",
                va="top",
            )
            ax.set_ylim(0, 27000),
            ax.yaxis.set_major_locator(MultipleLocator(12500))
            ax.yaxis.set_minor_locator(MultipleLocator(12500 / 2))
            PlotUtil.move_last_y_ticklabel_down(ax, "25000")
            # PlotUtil.move_first_y_ticklabel_up(ax, "0")
            ax.set_xlabel("eNB")

    h = [
        Line2D([0], [0], color="darkred"),
        Line2D([0], [0], color="black", linestyle="dotted"),
    ]
    l = ["Median", "Average"]

    fig.legend(
        h,
        l,
        loc="lower right",
        # handler_map={},
        bbox_to_anchor=(0.55, -0.045),
        ncol=2,
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
        columnspacing=1.0,
        handlelength=1.0,
    )

    fig.tight_layout()
    fig.subplots_adjust(
        left=0.18, bottom=0.18, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )
    save_old(fig, run_map.path("insertion_order_map_size.pdf"))


def throughput_by_enb_for_all_ttls(run_map: RunMap):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=75)
    fig, ax = plt.subplots(1, 1, figsize=fig_size)

    box_df = ThroughputData(run_map).get()
    boxes = box_df.frame("throughput_boxes")
    fliers = box_df.frame("throughput_fliers").dropna()

    boxes = sort_boxes_by_node_count(boxes=boxes, run_map=run_map)

    width, fixed_lbl = add_boxes_to_axes(ax, boxes=boxes, fliers=fliers, colors=colors)

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

    box_df = EnbRBData.insertion_order(run_map).hdf_boxes
    boxes = box_df.frame("boxes")
    width = 2.5
    fixed_lbl = [0]

    boxes = sort_boxes_by_node_count(boxes, run_map)
    num_nodes = NumGlobalCount.insertion_order(r).hdf.frame()
    ax.grid(visible=True, which="major", axis="y", zorder=1)
    annotation_y = [
        2,
        3.8,
        3.8,
        7,
        7,
        8,
        8,
        8.5,
        10.5,
        10.5,
        11.5,
        11.5,
        11.5,
        11.8,
        11.8,
    ]

    ax.annotate(
        "Average number of nodes in eNB",
        xy=(0, 3.8),
        xytext=(30, 13.8),
        xycoords="data",
        size="xx-small",
        color="gray",
        ha="center",
        arrowprops=dict(
            arrowstyle="->",
            edgecolor="gray",
            connectionstyle="arc3,rad=00.2",
            linewidth=1,
        ),
    )

    for i, (idx, b) in enumerate(boxes.iterrows()):
        enb, ttl = idx
        x_index = int(i / 3)
        if ttl == 15:
            positions = -width + x_index * 10
        elif ttl == 60:
            positions = x_index * 10
            fixed_lbl.append(enb)
        else:
            positions = width + x_index * 10
            ax.annotate(
                "{x:.1f}".format(x=num_nodes.loc[enb, "mean"]),
                xy=(int(i / 3) * 10, b["mean"]),
                xytext=(int(i / 3) * 10, annotation_y[x_index]),
                xycoords="data",
                color="gray",
                # arrowprops=dict(arrowstyle="-"),
                size="x-small",
                ha="center",
                rotation=60,
            )

        ax.bar(
            positions,
            b["mean"],
            width=width,
            color=colors[enb],
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


class MultiWayNorm(matplotlib.colors.Normalize):
    def __init__(self, x, y, vmin=..., vmax=..., clip=...) -> None:
        super().__init__(vmin, vmax, clip)
        self.x = x
        self.y = y

    def __call__(self, value, clip: bool | None = None):
        x, y = [self.vmin, *self.x, self.vmax], [0, *self.y, 1]
        return np.ma.masked_array(np.interp(value, x, y, left=-np.inf, right=np.inf))

    def inverse(self, value):
        y, x = [self.vmin, *self.x, self.vmax], [0, *self.y, 1]
        return np.ma.masked_array(np.interp(value, x, y, left=-np.inf, right=np.inf))


def plot_map_age_over_dist(run_map: RunMap):

    map_age_dist = MapAgeOverDistance.insertion_order(run_map)
    df_min = map_age_dist.hdf_age_over_dist_rsd_mean_of_mins.frame()
    df = df_min.groupby(["rsd", "dist", "ttl"])["mean_age"].mean().unstack("rsd")
    # df_avg =  map_age_dist.hdf_age_over_dist_rsd_avg.frame()
    # df = df_avg.groupby(["rsd", "dist", "ttl"])["mean_age"].mean().unstack("rsd")

    # get rsd values sorted by ascending number of nodes
    rsd_order = (
        NumGlobalCount.insertion_order(run_map)
        .get()
        .frame()["mean"]
        .reset_index()
        .sort_values("mean")["rsd_id"]
        .values
    )

    for provider in [
        map_age_dist.hdf_age_over_dist_rsd_mean_of_means,
        map_age_dist.hdf_age_over_dist_rsd_mean_of_mins,
        map_age_dist.hdf_age_over_dist_rsd_min_of_mins,
    ]:

        out_path = run_map.path(f"insertion_order_{provider.group}.pdf")
        df = provider.frame()
        value = df.columns[0]
        print(value)
        df = df.groupby(["rsd", "dist", "ttl"])[value].mean().unstack("rsd")

        color_a = matplotlib.colormaps["Reds"](np.linspace(0.4, 0.8, num=256))
        color_b = matplotlib.colormaps["Blues"](np.linspace(0.4, 0.8, num=256))
        color_c = matplotlib.colormaps["Purples"](np.linspace(0.4, 0.8, num=256))
        map = matplotlib.colors.LinearSegmentedColormap.from_list(
            "age_map", np.vstack([color_a, color_b, color_c])
        )

        fig_size = PlotUtil.fig_size_mm(width=111, height=50)
        fig = plt.figure(figsize=fig_size)
        fig, axes = plt.subplots(
            nrows=1,
            ncols=4,
            figsize=fig_size,
            gridspec_kw=dict(width_ratios=[20, 20, 20, 2]),
        )
        ttls = [15, 60, 3600]
        vmaxs = [15, 60, 850]
        for ax, ttl, vmax in zip(axes[0:3], ttls, vmaxs):
            ax: plt.Axes
            _df: pd.DataFrame = df.loc[pd.IndexSlice[:, ttl], rsd_order]
            lbls = [str(i) for i in _df.columns]

            _x = np.arange(16) - 0.5
            _y = _df.index.get_level_values(0).values / 1000
            _y = np.append(_y, _y[-1] + 50 / 1000) - 25 / 1000
            pmesh = ax.pcolormesh(
                _x,
                _y,
                _df,
                shading="flat",
                norm=MultiWayNorm([15, 60], [1 / 3, 2 / 3], vmin=0, vmax=860),
                cmap=map,
                edgecolors=None,
            )
            ax.yaxis.set_major_locator(MultipleLocator(1))
            ax.yaxis.set_minor_locator(MultipleLocator(0.2))
            ax.set_ylim(0, 4.2)
            if ttl > 15:
                ax.set_yticklabels([])

            ax.text(
                0.99,
                0.99,
                f"ttl{ttl}",
                fontsize="x-small",
                transform=ax.transAxes,
                ha="right",
                va="top",
            )

            ax.set_xlim(-0.5, 14.5)
            ax.xaxis.set_major_locator(FixedLocator(list(range(15))[::2]))
            ax.xaxis.set_major_formatter(FixedFormatter(lbls[::2]))
            ax.xaxis.set_minor_locator(FixedLocator(list(range(15))[1::2]))
            ax.xaxis.set_minor_formatter(FixedFormatter(lbls[1::2]))
            # ax.set_xticklabels(lbls, fontsize="xx-small")
            dx = 0 / 72.0
            dy = 1 / 72.0
            offset = matplotlib.transforms.ScaledTranslation(
                dx, dy, fig.dpi_scale_trans
            )
            ax.tick_params("x", length=17, which="minor")
            for idx, label in enumerate(ax.xaxis.get_minorticklabels()):
                label.set_transform(label.get_transform() + offset)
                label.set_fontsize("xx-small")
                label.set_rotation(60)
            for idx, label in enumerate(ax.xaxis.get_majorticklabels()):
                label.set_fontsize("xx-small")
                label.set_rotation(60)

        axes[1].set_xlabel("eNB sorted by number of nodes")
        axes[0].set_ylabel("Cell distance in km")
        axes[0].yaxis.set_label_coords(x=0.05, y=0.58, transform=fig.transFigure)
        # fig.colorbar(im, ax=axes[-1], ticks=[0,5,10,15,30, 50, 200, 500, 800],)
        fig.colorbar(
            pmesh,
            ticks=[0, 5, 10, 15, 30, 45, 60, 500, 800],
            cax=axes[-1],
            use_gridspec=True,
        )
        fig.subplots_adjust(
            left=0.1, bottom=0.32, right=0.925, top=0.99, wspace=0.05, hspace=0.13
        )
        ax: plt.Axes = axes[-1]
        ax.text(
            0.9,
            0.18,
            "AoI in\nseconds",
            ha="left",
            fontsize="xx-small",
            transform=fig.transFigure,
        )
        save_old(fig, out_path)
        print("hi")


if __name__ == "__main__":
    logger.setLevel(DEBUG)
    r = get_or_create_run_map()

    # rb_utilization_by_enb_for_all_ttls_bar_chart(r)
    # throughput_by_enb_for_all_ttls(r)
    # plot_map_size(r)
    plot_map_age_over_dist(r)
    # x = MapAgeOverDistance.insertion_order(r)

    print("hi")
