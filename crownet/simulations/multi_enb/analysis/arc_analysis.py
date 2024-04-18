from __future__ import annotations

from abc import ABC
from dataclasses import dataclass, field
from itertools import product, repeat
from functools import partial
from logging import DEBUG
import os
import shutil
import sys
from threading import Lock
from typing import Any, List
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, DpmmCfgBuilder
from crownetutils.analysis.dpmm.dpmm_sql import DpmmSql
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.helper import save_box_plot_to_hdf
from crownetutils.analysis.hdf_providers.map_age_over_distance import (
    MapMeasurementsAgeOverDistance,
    MapSizeAndAgeOverDistance,
)
from crownetutils.analysis.hdf_providers.map_error_data import MapCountError
from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.utils.parallel import ExecutionItem, run_items
from crownetutils.utils.plot import (
    FigureSaver,
    FigureSaverList,
    FigureSaverSimple,
    MultiWayNorm,
    percentiles_dict,
)
from crownetutils.utils.styles import load_matplotlib_style
from matplotlib import pyplot as plt
import matplotlib
from matplotlib.gridspec import GridSpec
from matplotlib.lines import Line2D
from matplotlib.offsetbox import AnnotationBbox, TextArea
from matplotlib.patches import BoxStyle
from matplotlib.colors import get_named_colors_mapping
from matplotlib.figure import Figure, SubFigure
import numpy as np
import pandas as pd
from numpy.typing import NDArray
from crownetutils.analysis.common import (
    CacheFunc,
    CacheLoader,
    IndexedSimulation,
    IndexedSimulationFilterAll,
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

order_lbl_map = dict(
    insertionOrder="IRR",
    orderByDistanceAscending="DAsc",
    orderByDistanceDescending="DDesc",
)


def ttl_lbl_map(ttl: str | int):
    _map = {
        "15": "\emph{TTL}\,15",
        "60": "\emph{TTL}\,60",
        "3600": "\emph{TTL}\,$\infty$",
    }
    return _map[str(ttl).strip()]


def add_ax_text(
    lbl: str,
    ax: plt.Axes,
    x: float = 0.01,
    y: float = 0.97,
    ha="left",
    va="top",
    transform=None,
    **kwargs,
) -> None:
    transform = ax.transAxes if transform is None else transform
    kwargs.setdefault("fontsize", "small")
    ax.text(x, y, s=lbl, transform=transform, ha=ha, va=va, **kwargs)


class BoxPlotHdfProvider(BaseHdfProvider):
    def __init__(
        self,
        hdf_path: str,
        group: str = "root",
        allow_lazy_loading: bool = False,
        shared_loc: Lock = None,
    ):
        super().__init__(hdf_path, group, allow_lazy_loading, shared_loc)
        self.boxes: BaseHdfProvider = self.created_shared_provider("boxes")
        self.fliers: BaseHdfProvider = self.created_shared_provider("fliers")


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
        # SeedLocation([5, 6, 8, 9], STUDY_DIR),
        SeedLocation([2, 3, 5, 6, 8, 9], STUDY_DIR),
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
    if path is None:
        return
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


class IsimFilterStr:
    def __init__(self, str_val) -> None:
        self.str_val = str_val

    def __call__(self, isim: IndexedSimulation) -> bool:
        return self.str_val in isim.group_name


@dataclass
class ByOrderCache(CacheLoader):
    @classmethod
    def _insertion_order(cls, run_map, path_suffix):
        obj = cls(
            run_map,
            cache_path=run_map.path(f"insertion_order_{path_suffix}.h5"),
            idx_sim_filter=IsimFilterStr("insertionOrder"),
        )
        obj.check()
        return obj

    @classmethod
    def _dist_ascending(cls, run_map, path_suffix):
        obj = cls(
            run_map,
            cache_path=run_map.path(f"dist_ascending_{path_suffix}.h5"),
            idx_sim_filter=IsimFilterStr("orderByDistanceAscending"),
        )
        obj.check()
        return obj

    @classmethod
    def _dist_descending(cls, run_map, path_suffix):
        obj = cls(
            run_map,
            cache_path=run_map.path(f"dist_descending_{path_suffix}.h5"),
            idx_sim_filter=IsimFilterStr("orderByDistanceDescending"),
        )
        obj.check()
        return obj


@dataclass
class NumGlobalCount(ByOrderCache):
    """Number of nodes per RSD (Average over Seeds)"""

    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._insertion_order(run_map, "glb_mean_number_nodes_per_enb")

    @classmethod
    def dist_ascending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_ascending(run_map, "glb_mean_number_nodes_per_enb")

    @classmethod
    def dist_descending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_descending(run_map, "glb_mean_number_nodes_per_enb")

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.filtered_indexed_simulation_iter():
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
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        df["ttl"] = int(_ttl)
        return df


@dataclass
class EnbRBData(ByOrderCache):
    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._insertion_order(run_map, "rb_usage_by_ttl")

    @classmethod
    def dist_ascending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_ascending(run_map, "rb_usage_by_ttl")

    @classmethod
    def dist_descending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_descending(run_map, "rb_usage_by_ttl")

    hdf_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field(is_root=True)
    hdf_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_hist: BaseHdfProvider = CacheLoader.shared_hdf_field()

    def cache_exists(self) -> bool:
        return super().cache_exists() and all(
            [g in ["boxes", "fliers", "hist"] for g in self.hdf.get_groups()]
        )

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.filtered_indexed_simulation_iter():
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

        save_box_plot_to_hdf(bplot, hdf=self.hdf_boxes, fliers_hdf=self.hdf_fliers)

    @staticmethod
    def _load(isim: IndexedSimulation):
        cfg: DpmmCfg = isim.sim.dpmm_cfg
        df = BaseHdfProvider(cfg.enb_rb.path, group=cfg.enb_rb.group)
        if df.hdf_file_exists:
            df = df.frame()
        else:
            raise FileNotFoundError(df._hdf_path)
        df["run_id"] = isim.global_id
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        df["ttl"] = int(_ttl)
        df = df.reset_index().rename(columns={"enb_idx": "rsd_id"})
        df["rsd_id"] += 1
        df = df.set_index(["rsd_id", "time"])
        return df


@dataclass
class NumberOfCellsInMap(ByOrderCache):
    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._insertion_order(run_map, "num_cells_in_map")

    @classmethod
    def dist_ascending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_ascending(run_map, "num_cells_in_map")

    @classmethod
    def dist_descending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_descending(run_map, "num_cells_in_map")

    # helpers for ease access to specific group

    hdf_glb_after_120_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120_desc: BaseHdfProvider = CacheLoader.shared_hdf_field()

    hdf_glb_all_boxes: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_all_fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_all_desc: BaseHdfProvider = CacheLoader.shared_hdf_field()

    hdf_nodes_all_desc_by_ttl: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_all_desc_by_enb_ttl: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_all_boxes_by_enb_ttl: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_nodes_all_fliers_by_enb_ttl: BaseHdfProvider = CacheLoader.shared_hdf_field()

    hdf_nodes_after_120_desc_by_ttl: BaseHdfProvider = CacheLoader.shared_hdf_field(
        is_root=True
    )
    hdf_nodes_after_120_desc_by_enb_ttl: BaseHdfProvider = (
        CacheLoader.shared_hdf_field()
    )
    hdf_nodes_after_120_boxes_by_enb_ttl: BaseHdfProvider = (
        CacheLoader.shared_hdf_field()
    )
    hdf_nodes_after_120_fliers_by_enb_ttl: BaseHdfProvider = (
        CacheLoader.shared_hdf_field()
    )

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.filtered_indexed_simulation_iter():
            items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df_nodes = [i[0] for i in ret]
        df_nodes: pd.DataFrame = pd.concat(df_nodes, axis=0)
        after_120 = df_nodes["simtime"] > 120
        # describe
        self.save_hdf(
            self.hdf_nodes_all_desc_by_enb_ttl,
            df_nodes.groupby(["ttl", "enb"])["numberOfCells"].describe(),
        )
        self.save_hdf(
            self.hdf_nodes_all_desc_by_ttl,
            df_nodes.groupby("ttl")["numberOfCells"].describe(),
        )

        self.save_hdf(
            self.hdf_nodes_after_120_desc_by_enb_ttl,
            df_nodes[after_120].groupby(["ttl", "enb"])["numberOfCells"].describe(),
        )
        self.save_hdf(
            self.hdf_nodes_after_120_desc_by_ttl,
            df_nodes[after_120].groupby("ttl")["numberOfCells"].describe(),
        )

        # box plots
        df_boxes = df_nodes.groupby(["ttl", "enb"])["numberOfCells"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )
        save_box_plot_to_hdf(
            df_boxes,
            hdf=self.hdf_nodes_all_boxes_by_enb_ttl,
            fliers_hdf=self.hdf_nodes_all_fliers_by_enb_ttl,
        )

        df_boxes = (
            df_nodes[after_120]
            .groupby(["ttl", "enb"])["numberOfCells"]
            .agg(calc_box_stats(use_mpl_name=True, include_mean=True))
        )
        save_box_plot_to_hdf(
            df_boxes,
            hdf=self.hdf_nodes_after_120_boxes_by_enb_ttl,
            fliers_hdf=self.hdf_nodes_after_120_fliers_by_enb_ttl,
        )

        df_glb = [i[1] for i in ret]
        df_glb: pd.DataFrame = pd.concat(df_glb, axis=0)

        after_120 = df_glb["simtime"] > 120
        # describe
        self.save_hdf(
            self.hdf_glb_all_desc, df_glb.groupby("ttl")["numberOfCells"].describe()
        )
        self.save_hdf(
            self.hdf_glb_after_120_desc,
            df_glb[after_120].groupby("ttl")["numberOfCells"].describe(),
        )

        # box plots
        df_boxes_glb = df_glb.groupby(["ttl"])["numberOfCells"].agg(
            calc_box_stats(use_mpl_name=True, include_mean=True)
        )
        save_box_plot_to_hdf(
            df_boxes_glb, hdf=self.hdf_glb_all_boxes, fliers_hdf=self.hdf_glb_all_fliers
        )

        df_boxes_glb = (
            df_glb[after_120]
            .groupby(["ttl"])["numberOfCells"]
            .agg(calc_box_stats(use_mpl_name=True, include_mean=True))
        )
        save_box_plot_to_hdf(
            df_boxes_glb,
            hdf=self.hdf_glb_after_120_boxes,
            fliers_hdf=self.hdf_glb_after_120_fliers,
        )

    @staticmethod
    def _load(isim: IndexedSimulation):
        cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        df_nodes = pd.read_csv(cfg.map_size.path, index_col=None)
        pos = NodePositionWithRsdHdf(isim.sim, cfg.position.path)
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]

        df_nodes["run_id"] = isim.global_id
        df_nodes["ttl"] = int(_ttl)
        df_nodes = pos.merge_rsd_id_on_host_time_interval(
            df_nodes,
            time_col="simtime",
            host_id_col="hostId",
            append_interval=False,
            rsd_col_name="enb",
        )
        df_glb = pd.read_csv(cfg.map_size_global.path, index_col=None)
        df_glb["run_id"] = isim.global_id
        df_glb["ttl"] = int(_ttl)
        return df_nodes, df_glb


@dataclass
class ThroughputData(ByOrderCache):
    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._insertion_order(run_map, "throughput_by_ttl")

    @classmethod
    def dist_ascending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_ascending(run_map, "throughput_by_ttl")

    @classmethod
    def dist_descending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_descending(run_map, "throughput_by_ttl")

    hdf_e_map_boxes: BaseHdfProvider = ByOrderCache.shared_hdf_field(is_root=True)
    hdf_e_map_fliers: BaseHdfProvider = ByOrderCache.shared_hdf_field()
    hdf_diff_e_map_boxes: BaseHdfProvider = ByOrderCache.shared_hdf_field()
    hdf_diff_e_map_fliers: BaseHdfProvider = ByOrderCache.shared_hdf_field()

    def load(self):
        items: List[ExecutionItem] = []
        rsd_list = list(range(1, 16))
        for idx_sim in self.filtered_indexed_simulation_iter():
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
        save_box_plot_to_hdf(
            bbox=bplot["e_map"],
            hdf=self.hdf_e_map_boxes,
            fliers_hdf=self.hdf_e_map_fliers,
        )
        save_box_plot_to_hdf(
            bbox=bplot["diff_e_map"],
            hdf=self.hdf_diff_e_map_boxes,
            fliers_hdf=self.hdf_diff_e_map_fliers,
        )

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
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        df["ttl"] = int(_ttl)
        return df


@dataclass
class MapAgeOverDistanceBoxPlots(CacheLoader):
    @classmethod
    def get(cls, run_map: RunMap) -> MapAgeOverDistanceBoxPlots:
        obj = cls(
            run_map,
            cache_path=run_map.path(f"map_age_over_dist_box_with_more_qq.h5"),
            idx_sim_filter=IndexedSimulationFilterAll(),
        )
        obj.check()
        return obj

    boxes: BaseHdfProvider = CacheLoader.shared_hdf_field(is_root=True)
    fliers: BaseHdfProvider = CacheLoader.shared_hdf_field()

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.filtered_indexed_simulation_iter():
            items.append(
                ExecutionItem(
                    fn=self._load,
                    kwargs=dict(isim=idx_sim),
                )
            )
        ret: List[pd.DataFrame] = run_items(
            items=items, pool_size=12, raise_on_error=True, unpack=True
        )
        ret: pd.DataFrame = pd.concat(ret, axis=0)
        print(f"shape {ret.shape}")

        boxes = ret.groupby(["order", "ttl", "dist_left"])["aoi"].agg(
            [
                "std",
                "min",
                "max",
                percentiles_dict(
                    1, 10, 20, 30, 40, 60, 70, 80, 90, 99
                ),  # 50 will be median provide by calc_stats. Is much faster than single values.
                calc_box_stats(use_mpl_name=True, include_mean=True),
            ]
        )
        boxes = pd.concat(
            [
                boxes[["std", "min", "max"]],
                pd.DataFrame.from_records(
                    boxes["percentile_records"], index=boxes.index
                ),
                pd.DataFrame.from_records(boxes["box_stats"], index=boxes.index),
            ],
            axis=1,
        )
        fliers = (
            boxes["fliers"]
            .explode()
            .to_frame()
            .set_axis(["fliers"], axis=1)
            .astype(float)
        )
        boxes = boxes.drop(columns=["fliers"])
        self.save_hdf(self.boxes, boxes)
        self.save_hdf(self.fliers, fliers)
        # save_box_plot_to_hdf(boxes["aoi"], hdf=self.boxes, fliers_hdf=self.fliers)

    def _load(self, isim: IndexedSimulation):
        e_cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        mm = MapSizeAndAgeOverDistance(  # <<< use this
            hdf_path=e_cfg.map_size_and_age_over_distance.path
        )
        metric_id = mm.metric_map["age_of_information"]
        df = mm.hdf.select(where=f"metric={metric_id}", columns=["mean"]).set_axis(
            ["aoi"], axis=1
        )
        df["run_id"] = isim.global_id
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        _order = isim.group_name[0 : (ttl_index - 4)]
        df["order"] = _order
        df["ttl"] = int(_ttl)
        return df


@dataclass
class MapAgeOverDistance(ByOrderCache):
    @classmethod
    def insertion_order(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._insertion_order(run_map, "map_age_over_distance")

    @classmethod
    def dist_ascending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_ascending(run_map, "map_age_over_distance")

    @classmethod
    def dist_descending(cls, run_map: RunMap) -> NumGlobalCount:
        return cls._dist_descending(run_map, "map_age_over_distance")

    hdf_age_over_dist_rsd: BaseHdfProvider = CacheLoader.shared_hdf_field()

    def load(self):
        items: List[ExecutionItem] = []
        rsd_list = list(range(1, 16))
        for idx_sim in self.filtered_indexed_simulation_iter():
            items.append(
                ExecutionItem(
                    fn=self._load,
                    kwargs=dict(isim=idx_sim, rsd_list=rsd_list),
                )
            )
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        ret = pd.concat(ret, axis=0)

        self.save_hdf(self.hdf_age_over_dist_rsd, frame=ret)

    @staticmethod
    def _load(isim: IndexedSimulation, rsd_list):
        e_cfg = DpmmCfgBuilder.load_entropy_cfg(isim.sim.data_root)
        mm = MapSizeAndAgeOverDistance(  # <<< use this
            hdf_path=e_cfg.map_size_and_age_over_distance.path
        )
        metric_id = mm.metric_map["age_of_information"]
        w_mean_of_means = mm.hdf_rsd.select(
            where=f"metric={metric_id}", columns=["count", "mean"]
        )
        weighted_means: List[pd.Series] = []
        for _idx, _df in w_mean_of_means.groupby(["rsd", "dist_left"]):
            w = _df["count"] / _df["count"].sum()
            w_mean = pd.Series(
                (w * _df["mean"]).sum(),
                name="average_aoi",
                index=pd.MultiIndex.from_tuples([(_idx)], names=["rsd", "dist"]),
            )
            weighted_means.append(w_mean)

        ret: pd.DataFrame = pd.concat(weighted_means).to_frame()
        ret["run_id"] = isim.global_id
        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        ret["ttl"] = int(_ttl)

        df = mm.hdf_rsd.select(
            where=f"metric={metric_id}",
            columns=["min", "p1", "p25", "p50", "p75", "p99", "max"],
        )
        _ret = df.groupby(["rsd", "dist_left"]).agg(["mean"]).droplevel(1, axis=1)
        _ret.columns = [f"average_{i}_aoi" for i in _ret.columns]
        _ret.index.names = ["rsd", "dist"]
        ret = pd.concat([ret, _ret], axis=1)

        _ret = (
            df.groupby(["rsd", "dist_left"])[["min"]]
            .min()
            .set_axis(["min_of_min_aoi"], axis=1)
        )
        _ret.index.names = ["rsd", "dist"]
        ret = pd.concat([ret, _ret], axis=1)
        ret = ret.reset_index().set_index(["ttl", "run_id", "rsd", "dist"])

        return ret


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


def _plot_map_size_boxes(
    boxes: pd.DataFrame, fliers: pd.DataFrame, axes: NDArray, colors
):
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


def plot_map_size(
    run_map: RunMap,
    num_cells_cache_f: CacheFunc,
    rsd_order: np.ndarray,
    saver: FigureSaverSimple,
):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=65)
    fig, axes = plt.subplots(ncols=1, nrows=3, figsize=fig_size, sharex=True)
    num_cells: NumberOfCellsInMap = num_cells_cache_f(run_map)

    boxes: pd.DataFrame = num_cells.hdf_nodes_after_120_boxes_by_enb_ttl.frame()
    fliers = num_cells.hdf_nodes_after_120_fliers_by_enb_ttl.frame()

    boxes = boxes.reorder_levels((1, 0)).sort_index().loc[rsd_order]
    fixed_lbl = boxes.index.get_level_values(0).unique().values

    _plot_map_size_boxes(boxes, fliers, axes, colors)

    ttl_stats = num_cells.hdf_nodes_after_120_desc_by_ttl.frame()
    for i, ax in enumerate(axes):
        ax.set_xlim(-6, 146)
        ax.xaxis.set_major_locator(FixedLocator((range(0, 160, 10))))
        ax.xaxis.set_major_formatter(FixedFormatter(fixed_lbl))
        ax.set_xlabel("eNB")
        ax.set_ylabel("")
        if i == 0:
            ax_lbl = "{}\t{:,.1f}$\pm${:,.1f}".format(
                ttl_lbl_map(15),
                ttl_stats.loc[15, "mean"],
                ttl_stats.loc[15, "std"],
            )
            add_ax_text(ax_lbl, ax=ax)
            ax.set_ylim(0, 600),
            ax.yaxis.set_major_locator(MultipleLocator(500))
            ax.yaxis.set_minor_locator(MultipleLocator(100))
            ax.set_xlabel("")
        elif i == 1:
            ax_lbl = "{}\t{:,.1f}$\pm${:,.1f}".format(
                ttl_lbl_map(60),
                ttl_stats.loc[60, "mean"],
                ttl_stats.loc[60, "std"],
            )
            add_ax_text(ax_lbl, ax=ax)
            ax.set_ylim(0, 1800),
            ax.yaxis.set_major_locator(MultipleLocator(1000))
            ax.yaxis.set_minor_locator(MultipleLocator(200))
            ax.set_xlabel("")
            ax.set_ylabel("Number of Map cells")
            ax.yaxis.set_label_coords(x=0.05, y=0.58, transform=fig.transFigure)
        else:
            ax_lbl = "{}\t{:,.1f}$\pm${:,.1f}".format(
                ttl_lbl_map(3600),
                ttl_stats.loc[3600, "mean"],
                ttl_stats.loc[3600, "std"],
            )
            add_ax_text(ax_lbl, ax=ax)
            add_ax_text(ttl_lbl_map(3600), ax=ax)
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
    saver(fig, "map_size", tight_layout=False)


def plot_throughput_by_enb_for_all_ttls(
    run_map: RunMap, tp_data_cache: CacheFunc, rsd_order, saver: FigureSaverSimple
):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=65)
    fig, ax = plt.subplots(1, 1, figsize=fig_size)

    tp: ThroughputData = tp_data_cache(run_map)
    boxes = tp.hdf_e_map_boxes.frame()
    fliers = tp.hdf_e_map_fliers.frame().dropna()

    boxes = boxes.reorder_levels((1, 0)).sort_index().loc[rsd_order]

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
        text="{}\n{}\n{}".format(ttl_lbl_map(15), ttl_lbl_map(60), ttl_lbl_map(3600)),
        xy=(1, 3),
        xytext=(0.08, -0.38),
        xycoords=ax.transAxes,
        fontsize="xx-small",
        ha="right",
        rotation=70,
    )

    ax.yaxis.set_label_coords(-0.08, 0.5, transform=ax.transAxes)
    fig.tight_layout()

    fig.subplots_adjust(
        left=0.12, bottom=0.3, right=0.995, top=0.99, wspace=0.08, hspace=0.15
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
    l = ["Fliers", "Shared bandwidth limit"]

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
        Line2D([0], [0], color="darkred"),
        Line2D([0], [0], color="black", linestyle="dotted"),
    ]
    l = ["Median", "Average"]

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

    saver(fig, "throughput", tight_layout=False)


def plot_rb_utilization_by_enb_for_all_ttls_histogram(run_map: RunMap):

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
def plot_rb_utilization_by_enb_for_all_ttls_bar_chart(
    run_map: RunMap,
    enb_rb_cache_f: CacheFunc,
    glb_node_count_cache_f: CacheFunc,
    rsd_order: np.ndarray,
    saver: FigureSaverSimple,
):

    colors = get_enb_colors(run_map, with_zero=False)

    fig_size = PlotUtil.fig_size_mm(width=111, height=65)
    fig, ax = plt.subplots(1, 1, figsize=fig_size)

    box_df: BaseHdfProvider = enb_rb_cache_f(run_map).hdf_boxes
    boxes = box_df.frame("boxes")
    width = 2.5
    fixed_lbl = [0]

    boxes = boxes.reorder_levels((1, 0)).sort_index().loc[rsd_order]
    num_nodes = glb_node_count_cache_f(run_map).hdf.frame()
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
        xytext=(40, 13.8),
        xycoords="data",
        size="x-small",
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
        text="{}\n{}\n{}".format(ttl_lbl_map(15), ttl_lbl_map(60), ttl_lbl_map(3600)),
        xy=(1, 3),
        xytext=(0.08, -0.38),
        xycoords=ax.transAxes,
        fontsize="xx-small",
        ha="right",
        rotation=70,
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

    saver(fig, "rb_utilization", tight_layout=False)


def plot_map_age_over_dist(
    run_map: RunMap, cache_f: CacheFunc, rsd_order: np.ndarray, saver: FigureSaver
):

    map_age_dist: MapAgeOverDistance = cache_f(run_map)

    data = map_age_dist.hdf_age_over_dist_rsd.frame()

    mean_max_std_dist = (
        data.index.to_frame()
        .reset_index(drop=True)
        .groupby(["ttl", "rsd"])["dist"]
        .max()
        .groupby("ttl")
        .agg(["mean", "std", "max"])
    )

    cols = ["average_aoi", "average_min_aoi", "min_of_min_aoi"]
    for col in cols:

        df = data.groupby(["rsd", "dist", "ttl"])[col].mean().unstack("rsd")

        color_a = matplotlib.colormaps["Reds"](np.linspace(0.4, 0.8, num=256))
        color_b = matplotlib.colormaps["Blues"](np.linspace(0.4, 0.8, num=256))
        color_c = matplotlib.colormaps["Purples"](np.linspace(0.4, 0.8, num=256))
        map = matplotlib.colors.LinearSegmentedColormap.from_list(
            "age_map", np.vstack([color_a, color_b, color_c])
        )

        fig_size = PlotUtil.fig_size_mm(width=111, height=65)
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
            ax.set_ylim(0, 4.5)
            if ttl > 15:
                ax.set_yticklabels([])

            dist_str = "{mean:.1f}$\pm${std:.1f}\,km".format(
                **((mean_max_std_dist.loc[ttl] / 1000).to_dict())
            )
            add_ax_text(ttl_lbl_map(ttl), ax=ax, x=0.02, y=0.98, fontsize="xx-small")
            add_ax_text(
                dist_str, ax=ax, ha="right", x=0.99, y=0.98, fontsize="xx-small"
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
        axes[1].xaxis.set_label_coords(x=0.5, y=0.07, transform=fig.transFigure)
        axes[0].set_ylabel("Cell distance in km")
        axes[0].yaxis.set_label_coords(x=0.05, y=0.58, transform=fig.transFigure)
        # fig.colorbar(im, ax=axes[-1], ticks=[0,5,10,15,30, 50, 200, 500, 800],)
        fig.colorbar(
            pmesh,
            ticks=[0, 5, 10, 15, 30, 45, 60, 500, 800],
            cax=axes[-1],
            use_gridspec=True,
        )
        ax: plt.Axes = axes[-1]
        ax.text(
            0.9,
            0.12,
            "AoI in\nseconds",
            ha="left",
            fontsize="xx-small",
            transform=fig.transFigure,
        )
        fig.subplots_adjust(
            left=0.1, bottom=0.25, right=0.925, top=0.99, wspace=0.05, hspace=0.13
        )

        saver(fig, col, tight_layout=False)


def plot_insertion_order(r: RunMap, rsd_order):

    saver = get_muli_saver(r.path("insertion_order"), "fsft_")

    plot_rb_utilization_by_enb_for_all_ttls_bar_chart(
        run_map=r,
        enb_rb_cache_f=EnbRBData.insertion_order,
        glb_node_count_cache_f=NumGlobalCount.insertion_order,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_throughput_by_enb_for_all_ttls(
        run_map=r,
        tp_data_cache=ThroughputData.insertion_order,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_size(
        run_map=r,
        num_cells_cache_f=NumberOfCellsInMap.insertion_order,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_age_over_dist(
        run_map=r,
        cache_f=MapAgeOverDistance.insertion_order,
        rsd_order=rsd_order,
        saver=saver,
    )


def get_muli_saver(path: str, prefix: str) -> FigureSaver:
    pdf_saver = FigureSaverSimple(override_base_path=path, figure_type=".pdf")
    png_saver = FigureSaverSimple(override_base_path=path, figure_type=".png", dpi=600)
    pdf_saver.with_prefix(prefix, count=-1)
    png_saver.with_prefix(prefix, count=-1)

    return FigureSaverList(pdf_saver, png_saver)


def plot_dist_ascending(r: RunMap, rsd_order):

    saver = get_muli_saver(r.path("dist_ascending"), "d_asc_")

    plot_rb_utilization_by_enb_for_all_ttls_bar_chart(
        run_map=r,
        enb_rb_cache_f=EnbRBData.dist_ascending,
        glb_node_count_cache_f=NumGlobalCount.dist_ascending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_throughput_by_enb_for_all_ttls(
        run_map=r,
        tp_data_cache=ThroughputData.dist_ascending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_size(
        run_map=r,
        num_cells_cache_f=NumberOfCellsInMap.dist_ascending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_age_over_dist(
        run_map=r,
        cache_f=MapAgeOverDistance.dist_ascending,
        rsd_order=rsd_order,
        saver=saver,
    )


def plot_dist_descending(r: RunMap, rsd_order):

    saver = get_muli_saver(r.path("dist_descending"), "d_desc_")

    plot_rb_utilization_by_enb_for_all_ttls_bar_chart(
        run_map=r,
        enb_rb_cache_f=EnbRBData.dist_descending,
        glb_node_count_cache_f=NumGlobalCount.dist_descending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_throughput_by_enb_for_all_ttls(
        run_map=r,
        tp_data_cache=ThroughputData.dist_descending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_size(
        run_map=r,
        num_cells_cache_f=NumberOfCellsInMap.dist_descending,
        rsd_order=rsd_order,
        saver=saver,
    )

    plot_map_age_over_dist(
        run_map=r,
        cache_f=MapAgeOverDistance.dist_descending,
        rsd_order=rsd_order,
        saver=saver,
    )


def plot_all():
    r = get_or_create_run_map()

    # get rsd values sorted by ascending number of nodes
    rsd_order = (
        NumGlobalCount.insertion_order(r)
        .get()
        .frame()["mean"]
        .reset_index()
        .sort_values("mean")["rsd_id"]
        .values
    )
    plot_insertion_order(r, rsd_order)
    plot_dist_ascending(r, rsd_order)
    plot_dist_descending(r, rsd_order)


class NumberOfCellsGlbByRSD(CacheLoader):

    hdf_glb_all_count_by_rsd: BaseHdfProvider = CacheLoader.shared_hdf_field()
    hdf_glb_after_120_count_by_rsd: BaseHdfProvider = CacheLoader.shared_hdf_field()

    _sql = "select d.uid, d.simtime, d.x , d.y from dcd_map_glb as d where d.uid > {lower_bound} and d.uid <= {upper_bound}"

    def load(self):
        items: List[ExecutionItem] = []
        for idx_sim in self.filtered_indexed_simulation_iter():
            items.append(ExecutionItem(fn=self._load, kwargs=dict(isim=idx_sim)))
        ret = run_items(items=items, pool_size=6, raise_on_error=True, unpack=True)
        df: List[pd.DataFrame] = [i[0] for i in ret]
        df: pd.DataFrame = pd.concat(df, axis=0)

        self.save_hdf(self.hdf_glb_all_count_by_rsd, frame=df)

        mask_120 = df.index.get_level_values("simtime").values > 120
        self.save_hdf(self.hdf_glb_after_120_count_by_rsd, frame=df[mask_120])

    @staticmethod
    def _load(isim: IndexedSimulation):

        sim: Simulation = isim.sim
        e_cfg = DpmmCfgBuilder.load_entropy_cfg(sim.path())
        df = pd.read_csv(e_cfg.map_size_global_by_rsd.path)

        ttl_index = isim.group_name.index("_ttl") + 4
        _ttl = isim.group_name[ttl_index:]
        df["run_id"] = isim.global_id
        df["ttl"] = int(_ttl)

        return df

    @staticmethod
    def calc(cells: pd.DataFrame, enb_pos: NDArray):
        selected_enb = np.zeros(cells.shape[0], dtype=int) - 1
        selected_dist = np.zeros(cells.shape[0], dtype=float) + np.inf
        for enb_ix, enb in enumerate(enb_pos):
            dist = np.linalg.norm(cells - enb, axis=1)
            mask_new_is_smaller = dist < selected_dist
            selected_dist[mask_new_is_smaller] = dist[mask_new_is_smaller]
            selected_enb[mask_new_is_smaller] = enb_ix + 1

        cells_by_enb = (
            np.concatenate([cells.T.flatten(), selected_enb]).reshape((3, -1)).T
        )
        cells_by_enb = pd.DataFrame(cells_by_enb, columns=["x", "y", "enb"])
        print("Hi")
        return cells_by_enb


def get_total_cell_count_by_rsd(run_map: RunMap) -> pd.DataFrame:
    df = []
    # todo one seed
    for isim in run_map.iter_all_sim():
        if isim.simulation_index == 0:
            c = DpmmCfgBuilder.load_entropy_cfg(isim.sim.path())
            _d = pd.read_csv(c.map_size_global_by_rsd.path)
            ttl_index = isim.group_name.index("_ttl") + 4
            _ttl = isim.group_name[ttl_index:]
            _d["run"] = isim.global_id
            _d["ttl"] = int(_ttl)
            df.append(_d)

    df = pd.concat(df)
    return df


def plot_s4_map_size():
    r = get_or_create_run_map()
    saver = get_muli_saver(r.path("s4"), "s4_")

    data_f: List[NumberOfCellsInMap] = [
        NumberOfCellsInMap.insertion_order(r),
        NumberOfCellsInMap.dist_ascending(r),
        NumberOfCellsInMap.dist_descending(r),
    ]

    fig_size = PlotUtil.fig_size_mm(width=210, height=70)
    fig, all_axes = plt.subplots(nrows=3, ncols=3, sharex=True, figsize=fig_size)
    ttl = [15, 60, 3600]
    colors = get_enb_colors(r, with_zero=False)

    rsd_order = (
        NumGlobalCount.insertion_order(r)
        .get()
        .frame()["mean"]
        .reset_index()
        .sort_values("mean")["rsd_id"]
        .values
    )

    for subfig_idx, (ttl, data, order_lbl) in enumerate(
        zip(ttl, data_f, ["IRR", "DAsc", "DDesc"])
    ):
        sfig: SubFigure
        axes: List[plt.Axes] = all_axes[:, subfig_idx]
        boxes = data.hdf_nodes_after_120_boxes_by_enb_ttl.frame()
        fliers = data.hdf_nodes_after_120_fliers_by_enb_ttl.frame()

        boxes = boxes.reorder_levels((1, 0)).sort_index().loc[rsd_order]
        fixed_lbl = boxes.index.get_level_values(0).unique().values

        ttl_stats = data.hdf_nodes_after_120_desc_by_ttl.frame()
        _plot_map_size_boxes(boxes, fliers, axes, colors)

        for row_idx, ax in enumerate(axes):
            ax.set_xlim(-6, 146)
            ax.xaxis.set_major_locator(FixedLocator((range(0, 160, 10))))
            ax.xaxis.set_major_formatter(FixedFormatter(fixed_lbl))
            ax.set_ylabel("")
            if row_idx == 0:
                ax_lbl = "{}\t$\widehat{{N}}: {:,.1f}\pm${:,.1f}".format(
                    ttl_lbl_map(15),
                    ttl_stats.loc[15, "mean"],
                    ttl_stats.loc[15, "std"],
                )
                add_ax_text(ax_lbl, ax=ax, y=0.965, fontsize="xx-small")
                ax.set_ylim(0, 600),
                ax.yaxis.set_major_locator(MultipleLocator(500))
                ax.yaxis.set_minor_locator(MultipleLocator(100))
                ax.set_xlabel("")
            elif row_idx == 1:
                ax_lbl = "{}\t$\widehat{{N}}: {:,.1f}\pm${:,.1f}".format(
                    ttl_lbl_map(60),
                    ttl_stats.loc[60, "mean"],
                    ttl_stats.loc[60, "std"],
                )
                add_ax_text(ax_lbl, ax=ax, y=0.965, fontsize="xx-small")
                ax.set_ylim(0, 1800),
                ax.yaxis.set_major_locator(MultipleLocator(1000))
                ax.yaxis.set_minor_locator(MultipleLocator(200))
                ax.set_xlabel("")
                ax.set_ylabel("Number of Map cells")
                ax.yaxis.set_label_coords(x=0.025, y=0.58, transform=fig.transFigure)
            else:
                ax_lbl = "{}\t$\widehat{{N}}: {:,.0f}\pm${:,.0f}".format(
                    ttl_lbl_map(3600),
                    ttl_stats.loc[3600, "mean"],
                    ttl_stats.loc[3600, "std"],
                )
                add_ax_text(ax_lbl, ax=ax, y=0.97, fontsize="xx-small")
                ax.set_ylim(0, 27500),
                ax.yaxis.set_major_locator(MultipleLocator(12500))
                ax.yaxis.set_minor_locator(MultipleLocator(12500 / 2))
                PlotUtil.move_last_y_ticklabel_down(ax, "25000")
                # PlotUtil.move_first_y_ticklabel_up(ax, "0")
                ax.set_xlabel(f"{order_lbl}:\teNB")
            if subfig_idx != 0:
                ax.yaxis.set_ticklabels([])

    fig.tight_layout()
    fig.subplots_adjust(
        left=0.09, bottom=0.18, right=0.995, top=0.99, wspace=0.08, hspace=0.05
    )
    saver(fig, "map_size", tight_layout=False)


def plot_s4_age_over_map_extend():
    r = get_or_create_run_map()

    saver = get_muli_saver(r.path("s4"), "s4_")

    fig_size = PlotUtil.fig_size_mm(width=111, height=110)
    m: MapAgeOverDistanceBoxPlots = MapAgeOverDistanceBoxPlots.get(r)
    fig, axes = plt.subplots(ncols=1, nrows=3, figsize=fig_size, sharex=True)
    ttl = [15, 60, 3600]
    colors = plt.colormaps["tab10"]([0, 1, 2])
    order: List[str] = [
        "insertionOrder",
        "orderByDistanceAscending",
        "orderByDistanceDescending",
    ]
    for t, ax in zip(ttl, axes):
        ax: plt.Axes
        for o, c in zip(order, colors):
            boxes = m.boxes.select(where=f"order={o} and ttl={t}")
            dist = boxes.index.get_level_values("dist_left") / 1000
            line = ax.plot(
                dist,
                boxes["mean"],
                linestyle="solid",
                label=f"{order_lbl_map[o]} mean",
                color=c,
            )
            line = ax.plot(
                dist,
                boxes["med"],
                linestyle="dotted",
                label=f"{order_lbl_map[o]} median",
                color=c,
            )
            ax.fill_between(
                x=dist,
                y1=boxes["p10"],
                y2=boxes["p90"],
                label="p10/90",
                color=c,
                alpha=0.2,
            )

        if t == 15:
            ax.yaxis.set_major_locator(MultipleLocator(5))
            ax.yaxis.set_minor_locator(MultipleLocator(1))
            ax.set_ylim(0, 16)
        elif t == 60:
            ax.yaxis.set_major_locator(MultipleLocator(10))
            ax.yaxis.set_minor_locator(MultipleLocator(2))
            ax.set_ylim(0, 65)
            ax.set_ylabel("Age of Information (AoI) in seconds")
            ax.yaxis.set_label_coords(0.05, 0.6, transform=fig.transFigure)

        else:
            ax.yaxis.set_major_locator(MultipleLocator(200))
            ax.yaxis.set_minor_locator(MultipleLocator(50))
            ax.set_ylim(0, 1100)

        add_ax_text(ttl_lbl_map(t), ax=ax, x=0.99, ha="right")

        ax.xaxis.set_major_locator(MultipleLocator(1))
        ax.xaxis.set_minor_locator(MultipleLocator(0.200))
        ax.set_xlim(0, 4.5)
        ax.grid(visible="major", axis="x")
    ax = axes[-1]
    h, l = ax.get_legend_handles_labels()
    h = [h[0], (h[1], h[2]), h[3], (h[4], h[5]), h[6], (h[7], h[8])]
    l = [
        l[0],
        f"{l[1]}\n with {l[2]}",
        l[3],
        f"{l[4]}\n with {l[5]}",
        l[6],
        f"{l[7]}\n with {l[8]}",
    ]
    fig.legend(
        h,
        l,
        ncol=3,
        bbox_transform=fig.transFigure,
        bbox_to_anchor=(0.95, 0.15),
        frameon=False,
        fontsize="x-small",
        labelspacing=0.5,
        columnspacing=1.2,
        handlelength=1.5,
    )
    ax.set_xlabel("Average map extend in km")
    fig.tight_layout()
    fig.subplots_adjust(
        left=0.15,
        bottom=0.25,
        right=0.995,
        top=0.99,
        wspace=0.02,
        hspace=0.09,
    )
    saver(fig, "age_over_map_extend", tight_layout=False)


if __name__ == "__main__":
    logger.setLevel(DEBUG)

    # plot_all()
    plot_s4_age_over_map_extend()
    # plot_s4_map_size()
