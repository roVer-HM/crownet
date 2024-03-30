from functools import partial
import os
import glob
import re
import shutil
import io
import sys
from crownetutils.analysis.dpmm import MapType
from crownetutils.utils.logging import timing
import scipy.stats as stats
from typing import Any, List, Tuple
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg
from crownetutils.analysis.hdf_providers.map_error_data import MapCountError
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from matplotlib.ticker import (
    MultipleLocator,
    FixedLocator,
    FixedFormatter,
    LogFormatter,
)
from crownetutils.analysis.common import (
    CacheLoader,
    IndexedSimulation,
    RunContext,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from itertools import repeat
from crownetutils.analysis.dpmm.metadata import DpmmMetaData
from crownetutils.analysis.hdf.provider import (
    BaseHdfProvider,
    HdfGroupFactory,
    LazyHdfProvider,
)
from crownetutils.vadere.plot.topgraphy_plotter import VadereTopographyPlotter
from crownetutils.utils.dataframe import numeric_formatter, save_as_tex_table
from crownetutils.utils.parallel import ExecutionItem, run_items, run_kwargs_map
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.utils.plot import (
    PlotUtil_,
    mpl_table_cell_iter,
    percentile,
    with_axis,
)
import pandas as pd
import numpy as np
import seaborn as sns
import scipy
import matplotlib.pyplot as plt
import matplotlib.patches as pltPatch
import matplotlib.lines as pltLines

from crownetutils.utils.styles import (
    load_matplotlib_style,
    STYLE_TEX,
)

# load_matplotlib_style(STYLE_TEX)
load_matplotlib_style(os.path.join(os.path.dirname(__file__), "paper_tex.mplstyle"))

# SIM_RESULT_DIR = "/mnt/data1tb/results"
# S0_SIM_DATA_DIR = "/mnt/data1tb/results/arc-dsa_single_cell/s0_corridor_500kbps_map_table_count_est_and_no_rate_limit/"
# S1_SIM_DATA_DIR = "/mnt/data1tb/results/arc-dsa_single_cell/s1_corridor_5_to_500kbps_map_count_est_only"

S0_SIM_DATA_DIR = "/mnt/ssd_local/arc-dsa_single_cell/s0_corridor_500kbps_map_table_count_est_and_no_rate_limit/"
S1_SIM_DATA_DIR = (
    "/mnt/ssd_local/arc-dsa_single_cell/s1_corridor_5_to_500kbps_map_count_est_only/"
)
SIM_RESULT_DIR = "/mnt/ssd_local/arc-dsa_single_cell/analysis_dir"


class ThroughputCache(CacheLoader):
    G_rates_ts = "rates_over_time"
    G_qq = "qq"
    G_root = "tp_25"

    @staticmethod
    @timing
    def _load(isim: IndexedSimulation, target_rates: dict, freq: float):
        tx = NodeTxData(isim.sim.dpmm_cfg.node_tx.path)
        data = tx.get_tx_throughput_diff_by_app(
            bin_size=freq, throughput_unit=1000, target_rates=target_rates
        )
        data["run_id"] = isim.simulation_index
        data["sg"] = isim.group_name
        return data

    def __init__(self, run_map: RunMap) -> None:
        super().__init__(run_map, run_map.path("throughput.h5"), root_group=self.G_root)

    def cache_exists(self) -> bool:
        return super().cache_exists() and all(
            [
                self.hdf.contains_group(g)
                for g in [self.G_rates_ts, self.G_qq, self.G_root]
            ]
        )

    @property
    def qq(self) -> BaseHdfProvider:
        self.check()
        return self.hdf.created_shared_provider(self.G_qq)

    @property
    def rates_over_time(self) -> BaseHdfProvider:
        self.check()
        return self.hdf.created_shared_provider(self.G_rates_ts)

    def load(self):
        items: List[ExecutionItem] = []
        for sg, isim in self.run_map.iter_all_sim(with_group=True):
            target_rates = dict(
                d_map=sg.attr["target_rate_in_kbps"] / 8, d_beacon=50 / 8
            )
            item = ExecutionItem(
                fn=self._load,
                kwargs=dict(isim=isim, target_rates=target_rates, freq=25.0),
            )
            items.append(item)
        df = run_items(items, pool_size=10, raise_on_error=True, unpack=True)
        df: pd.DataFrame = pd.concat(df, axis=0)
        df = df.reset_index().set_index(["time", "sg", "run_id"]).sort_index()

        qq = df.groupby(["sg", "run_id"])["d_map"].mean()
        rates_over_time = df.groupby(["sg", "time"])["d_map"].agg(
            ["mean", percentile(25), percentile(75)]
        )

        self.hdf.write_frame(group=self.hdf.group, frame=df)
        self.hdf.write_frame(group=self.G_qq, frame=qq)
        self.hdf.write_frame(group=self.G_rates_ts, frame=rates_over_time)


class RandomMap(PlotUtil_):
    def __init__(self, run_map) -> None:
        self.run_map: RunMap = run_map

    def get_random_draw(self):
        hdf = self.run_map.get_hdf("dpmm_ground_truth.h5", "gt")
        data = self.get_data()
        seed_dist = data.groupby(["seed", "count"]).count().iloc[:, 0]
        seed_dist = seed_dist / seed_dist.groupby("seed").sum()
        if hdf.contains_group("msme_draw"):
            draw = hdf.get_dataframe("msme_draw")
        else:
            draw = self.build_draw_dpmm(hdf=hdf, data=data, seed_dist=seed_dist)

        draw = draw.reset_index()
        return draw

    def plot_random_msme(self):
        data = self.get_data()
        draw = self.get_random_draw()

        figsize = self.fig_size_cm(width=11.1, height=12.5)
        fig, ax_m = plt.subplot_mosaic("111;222", figsize=figsize)
        ax_hist = ax_m["1"]
        self.plot_cell_occupation_hist(data, ax=ax_hist)

        ax_box = ax_m["2"]
        self.plot_seed_random_msme(draw, ax_box)

        fig.tight_layout(h_pad=2.0)
        fig.subplots_adjust(left=0.158, bottom=0.1, right=0.99, top=0.99)

        self.move_last_y_ticklabel_down(ax_hist, "0.6")

        try:
            shutil.copy(
                self.run_map.path("rnd_msme.pdf"),
                self.run_map.path("rnd_msme_old.pdf"),
            )
        except:
            pass
        fig.savefig(self.run_map.path("rnd_msme.pdf"))
        print("done")

    def plot_seed_random_msme(self, draw: pd.DataFrame, ax: plt.Axes):
        seeds = draw["seed"].unique()
        colors = sns.color_palette("colorblind", n_colors=len(seeds))
        box_data = []
        for idx, (g, _df) in enumerate(draw.groupby("seed")):
            box_data.append(_df["msme"])

        ax.hlines(
            y=np.percentile(draw["msme"], 25),
            colors="red",
            xmin=-0.5,
            xmax=19.5,
            linestyles="dashed",
            label="Q1/3",
        )
        ax.hlines(
            y=np.percentile(draw["msme"], 75),
            colors="red",
            xmin=-0.5,
            xmax=19.5,
            linestyles="dashed",
            label="Q1/3",
        )
        ax.hlines(
            y=draw["msme"].mean(),
            colors="red",
            xmin=-0.5,
            xmax=19.5,
            linestyles="solid",
            label="mean",
        )
        print(draw["msme"].describe())
        draw["msme"].describe().to_csv(self.run_map.path("rnd_msme_stats.csv"))

        b = ax.boxplot(box_data, positions=range(20), flierprops={"marker": "."})
        # ax.xaxis.set_major_locator(MultipleLocator(2))
        lbls = [i if i % 2 == 0 else "" for i in range(20)]
        lbls[0] = "0"
        lbls[-1] = "19"
        ax.set_xticklabels(lbls)
        ax.set_xlim(-0.5, 19.5)
        ax.yaxis.set_major_locator(MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(MultipleLocator(0.05 / 5))
        ax.set_ylim(1.725, 1.925)
        ax.set_ylabel("MSME")
        ax.set_xlabel("Simulation Seeds")
        h, l = ax.get_legend_handles_labels()
        ax.legend(
            [h[-1], h[0]],
            [l[-1], l[0]],
            ncol=2,
            loc="lower left",
            frameon=False,
            borderpad=0.2,
            borderaxespad=0,
            labelspacing=0.1,
            handlelength=1.3,
            columnspacing=0.5,
            handletextpad=0.1,
        )

    def plot_cell_occupation_hist(
        self, data: pd.DataFrame, *, alpha=0.5, ax: plt.Axes = None
    ):
        seeds = data["seed"].unique()
        colors = sns.color_palette("colorblind", n_colors=len(seeds))
        _range = (data["count"].min(), data["count"].max())
        _bin_count = data["count"].max() + 1
        _bins = np.histogram(data["count"], bins=int(_bin_count))[1]
        s = stats.describe(data["count"])
        s2 = data["count"].describe().iloc[1:].apply("{:.4f}".format)

        def str_pad(strings):
            m_length = max([len(str(s)) for s in strings])

            def _f(s):
                pad = m_length - len(str(s))
                pad = " " * pad
                s = f"{s}:{pad} "
                return s

            return _f

        pad_f = str_pad(s2.index)
        lbl = ""
        for l, v in list(zip(s2.index, s2.values)):
            lbl += f"{pad_f(l)}{v}\n"

        lbl = lbl.replace("%", "\%")
        lbl = lbl[:-1]
        ax.text(
            9.353,
            0.58,
            lbl,
            transform=ax.transData,
            verticalalignment="top",
            horizontalalignment="right",
            bbox=dict(facecolor="white", alpha=1.0),
            fontdict={"family": "monospace", "size": "small"},
        )

        for idx, (g, _df) in enumerate(data.groupby("seed")):
            ax.hist(
                _df["count"],
                bins=10,
                range=(-0.5, 9.5),
                density=True,
                histtype="step",
                color=colors[idx],
            )
        ax.hist(
            data["count"],
            bins=10,
            range=(-0.5, 9.5),
            histtype="stepfilled",
            density=True,
            color="gray",
            alpha=0.30,
            label="all Seeds",
        )
        ax.vlines(
            data["count"].mean(),
            0,
            0.6,
            colors="black",
            linestyles="dashed",
            label="mean all Seeds",
        )

        ax.xaxis.set_major_locator(MultipleLocator(1))
        ax.yaxis.set_major_locator(MultipleLocator(0.1))
        ax.yaxis.set_minor_locator(MultipleLocator(0.1 / 2))
        ax.set_ylim(0, 0.6)
        ax.set_xlim(-0.5, 9.5)
        ax.set_xlabel("Number of agents in cell")
        ax.set_ylabel("Density")
        ax.legend(
            loc="lower right",
            frameon=False,
            borderpad=0.2,
            borderaxespad=0,
            labelspacing=0.1,
            handlelength=1.3,
        )
        # color_legend(ax, colors=colors, alpha=0.3, loc="upper right")
        return ax

    def build_draw_dpmm(
        self, hdf: BaseHdfProvider, data: pd.DataFrame, seed_dist: pd.Series
    ):
        buf = io.StringIO()

        def _print(_str):
            buf.write(str(_str))
            buf.write("\n")
            print(_str)

        seeds = data["seed"].unique()
        draw: List[pd.DataFrame] = []
        for seed in seeds:
            _print(f"for mobility seed: {seed}")
            _print(data[data["seed"] == seed]["count"].describe())
            _print(stats.describe(data[data["seed"] == seed]["count"]))

            _d = data[data["seed"] == seed]
            _d = _d.set_index(["simtime", "x", "y"]).sort_index()
            seed_rv = stats.rv_discrete(
                values=(seed_dist.loc[seed].index, seed_dist.loc[seed].values)
            )
            _df_draw = self.draw_dpmm(seed_rv, _d, N=1000, _print=_print)
            draw.append(_df_draw)

        draw = pd.concat(draw, axis=0)
        hdf.write_frame(frame=draw, group="msme_draw")
        with open(self.run_map.path("draw_msme.txt"), "w") as fd:
            buf.seek(0)
            fd.write(buf.read())
        return draw

    def get_data(self) -> pd.DataFrame:
        return self.lazy_read_dpmm_gt_from_sim_group(
            sg="map-500kbps",
            hdf_path=self.run_map.path("dpmm_ground_truth.h5"),
            group="gt",
        ).frame()

    def lazy_read_dpmm_gt_from_sim_group(
        self, sg: str, hdf_path: str, group: str = "root"
    ):
        gf = HdfGroupFactory(
            group_name=group,
            factory=partial(self.read_dpmm_gt_from_sim_group, sg=sg),
        )
        return LazyHdfProvider(hdf_path, group, gf)

    def read_dpmm_gt_from_sim_group(self, sg: str):

        df = []
        for sim in self.run_map[sg]:
            # select ground truth data from simulation run (i.e. seed)
            _df = (
                sim.get_dcdMap()
                .count_p[pd.IndexSlice[:, :, :, 0], ["count"]]
                .reset_index(["ID"], drop=True)
            )
            _df = _df.reorder_levels(["simtime", "x", "y"])
            # build missing (time, x, y) entries containing a count of zero.
            times = _df.index.get_level_values("simtime").sort_values().unique()
            full_idx = sim.get_dcdMap().metadata.create_full_index(
                times=times, real_coords=True
            )
            diff_idx = full_idx.difference(_df.index)
            _zero = pd.DataFrame(0, index=diff_idx, columns=["count"])
            _df = pd.concat([_df, _zero])
            # add seed column and add to list
            _df["seed"] = sim.run_context.opp_seed
            _df = _df.reset_index("simtime")
            _df = _corridor_filter_target_cells(_df).reset_index()
            df.append(_df)
        return pd.concat(df, axis=0)

    def draw_dpmm(
        self, seed_rv: stats.rv_discrete, gt: pd.DataFrame, N=200, _print=print
    ):
        msme = []
        msd = []
        for i in np.arange(N):
            _df = gt.rename(columns={"count": "gt"}).copy(deep=True)
            # draw from distribution
            _df["count"] = seed_rv.rvs(size=_df.shape[0])
            # _df["count"] = _df["gt"].mean()
            # calculate Mean squared cell error for ONE 'random measurement' thus the MSCE is
            # calcuated using N=1 --> MSCE = Suqared error
            _df["err"] = _df["count"] - _df["gt"]
            _df["sqerr"] = _df["err"] ** 2
            # MSME over time and the for the hole simulation
            _msme = _df.loc[:, ["sqerr"]].groupby(["simtime"]).mean().mean()
            _msd = _df.loc[:, ["err"]].groupby(["simtime"]).mean().mean()
            msme.append(_msme.values[0])
            msd.append(_msd.values[0])

        _print(f"MSME over N={N} meta seeds")
        msme = pd.Series(
            msme, name="msme", index=pd.Index(np.arange(N), name="meta_seed")
        )
        _print(msme.describe())
        _print(stats.describe(msme))
        msd = pd.Series(msd, name="msd", index=pd.Index(np.arange(N), name="meta_seed"))
        _print(msd.describe())
        _print(stats.describe(msd))

        df = pd.concat([msme, msd], axis=1)
        df["seed"] = gt["seed"].unique()[0]

        return df


def kBpsLbl(kbps):
    if isinstance(kbps, SimulationGroup):
        kbps = kbps.attr["target_rate_in_kbps"]
    kBps = kbps / 8
    return f"{kBps:.1f} kBps"


def _corridor_filter_target_cells(df: pd.DataFrame) -> pd.DataFrame:
    # remove cells under target area
    if isinstance(df, pd.MultiIndex):
        xy = df.to_frame()
    else:
        xy = df.index.to_frame()
    mask = np.repeat(False, xy.shape[0])
    # for x in [400, 405, 410]: # remove target cells
    for x in [400, 405, 410, 0, 5, 10]:  # remove source/target cells
        y = 180.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    # for x in [0.0, 5.0, 10.0]:    # remove target cells
    for x in [0.0, 5.0, 10.0, 400, 405, 410]:  # remove source/target cells
        y = 205.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    # limit y to corridors
    mask = mask | ~((xy["y"] == 180.0) | (xy["y"] == 205.0))
    mask = mask | (xy["x"] > 410)

    if isinstance(df, pd.MultiIndex):
        ret = xy[~mask].copy(deep=True)
        ret = pd.MultiIndex.from_frame(ret)
    else:
        ret = df[~mask].copy(deep=True)
    print(f"remove {df.shape[0]-ret.shape[0]} rows")
    return ret


def create_member_estimate_run_map(output_path, *args, **kwargs) -> RunMap:
    """All runs with different member estimates (neighborhood Table, Density Map, No adaption)"""
    os.makedirs(output_path, exist_ok=True)
    study_base = S0_SIM_DATA_DIR

    def create_sim_group(sim: Simulation, data: List[Simulation], *args, **kwargs):
        ctx: RunContext = data[0].run_context
        meta = {
            "map_target_rate": ctx.ini_get_or_default(
                "*.misc[*].app[1].scheduler.maxApplicationBandwidth", default=""
            ),
            "member_estimate": ctx.ini_get_or_default(
                "*.misc[*].app[*].scheduler.neighborhoodSizeProvider",
                apply=lambda x: x.replace("^.^.", "")
                .replace('"', "")
                .replace("app[1].app", "map"),
                default="ConstantRate",
            ),
        }
        if (
            ctx.ini_get(
                "*.misc[*].app[*].scheduler.typename",
                apply=lambda x: x.replace('"', ""),
            )
            == "IntervalScheduler"
        ):
            meta["map_target_rate"] = ""
            meta["member_estimate"] = "ConstantRate"

        if "map" in meta["member_estimate"].lower():
            meta["lbl"] = "Map count est."
        elif "table" in meta["member_estimate"].lower():
            meta["lbl"] = "Neighbor count est."
        else:
            meta["lbl"] = "Constant rate"

        sg = SimulationGroup(
            data=data,
            group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
            attr=meta,
        )
        return sg

    run_map: RunMap = RunMap(output_path)
    _study: SuqcStudy = SuqcStudy(study_base)
    _study.update_run_map(
        run_map=run_map,
        sim_group_factory=create_sim_group,
        sim_per_group=20,
        id_offset=0,
    )
    return run_map


def create_target_rate_run_map_new(output_path, *args, **kwargs) -> RunMap:
    """All runs with member estimate using density map and changing target rates [50...500kbps]"""
    os.makedirs(output_path, exist_ok=True)
    study_base = S1_SIM_DATA_DIR

    # 15 seeds only
    # seed_list = [0, 1, 2, 3, 4, 6, 7, 8, 10, 11, 12, 13, 14, 16, 18]

    # all 20 seeds
    seed_list = list(range(20))

    def create_sim_group(sim: Simulation, data: List[Simulation], *args, **kwargs):

        data = list(np.array(data)[seed_list])

        ctx: RunContext = data[0].run_context
        meta = {
            "map_target_rate": ctx.ini_get(
                "*.misc[*].app[1].scheduler.maxApplicationBandwidth"
            ),
            "member_estimate": ctx.ini_get(
                "*.misc[*].app[*].scheduler.neighborhoodSizeProvider",
                apply=lambda x: x.replace("^.^.", "")
                .replace('"', "")
                .replace("app[1].app", "map"),
            ),
        }
        meta["target_rate_in_kbps"] = float(meta["map_target_rate"].replace("kbps", ""))
        meta["lbl"] = meta["map_target_rate"]
        sg = SimulationGroup(
            data=data,
            group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
            attr=meta,
        )
        return sg

    run_map: RunMap = RunMap(output_path)
    _study: SuqcStudy = SuqcStudy(study_base)
    _study.update_run_map(
        run_map=run_map,
        sim_group_factory=create_sim_group,
        sim_per_group=20,
        id_offset=0,
    )
    return run_map


def create_target_rate_run_map(output_path, *args, **kwargs) -> RunMap:
    """All runs with member estimate using density map and changing target rates [50...500kbps]"""
    os.makedirs(output_path, exist_ok=True)

    def create_sim_group(sim: Simulation, data: List[Simulation], *args, **kwargs):
        ctx: RunContext = data[0].run_context
        meta = {
            "map_target_rate": ctx.ini_get(
                "*.misc[*].app[1].scheduler.maxApplicationBandwidth"
            ),
            "member_estimate": ctx.ini_get(
                "*.misc[*].app[*].scheduler.neighborhoodSizeProvider",
                apply=lambda x: x.replace("^.^.", "")
                .replace('"', "")
                .replace("app[1].app", "map"),
            ),
        }
        meta["target_rate_in_kbps"] = float(meta["map_target_rate"].replace("kbps", ""))
        meta["lbl"] = meta["map_target_rate"]
        sg = SimulationGroup(
            data=data,
            group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
            attr=meta,
        )
        return sg

    run_map: RunMap = RunMap(output_path)

    # 1 500kbps
    root_target_rate_500 = os.path.join(SIM_RESULT_DIR, "s1_corridor_ramp_down/")
    _study: SuqcStudy = SuqcStudy(root_target_rate_500)
    map_filter = lambda x: x[0] >= 20 and x[0] < 40
    _study.update_run_map(
        run_map=run_map,
        sim_group_factory=create_sim_group,
        sim_per_group=20,
        id_offset=-20,
        id_filter=map_filter,
    )

    # 2 [50, 100, 150, 250] kbps
    root_target_rate_lt500 = os.path.join(
        SIM_RESULT_DIR, "s1_corridor_ramp_down_rate_lt500kbps"
    )
    # map_filter = lambda x: not(x[0]>=20 and x[0]<60)
    map_filter = lambda x: True
    _study = SuqcStudy(root_target_rate_lt500)
    _study.update_run_map(
        run_map=run_map,
        sim_group_factory=create_sim_group,
        sim_per_group=20,
        id_offset=run_map.max_id + 1,
        id_filter=map_filter,
    )

    root_target_rate_lt500 = os.path.join(
        SIM_RESULT_DIR, "s1_corridor_ramp_down_rate_lt500kbps_1"
    )
    # map_filter = lambda x: not(x[0]>=20 and x[0]<60)
    map_filter = lambda x: True
    _study = SuqcStudy(root_target_rate_lt500)
    _study.update_run_map(
        run_map=run_map,
        sim_group_factory=create_sim_group,
        sim_per_group=20,
        id_offset=run_map.max_id + 1,
        id_filter=map_filter,
    )

    return run_map


def get_ped_count():
    csv_path = "../study/corridor_trace_5perSpawn_ramp_down.d/numAgents_corridor_2x5m_d20_5perSpawn_ramp_down_*.csv"
    seed_re = re.compile(".*_(\d+)\.csv")
    df = []
    for _p in glob.glob(csv_path, recursive=False):
        seed = int(seed_re.match(_p).groups()[0])
        _df = pd.read_csv(_p, sep=" ", skiprows=2)
        _df = _df.set_axis(["time", "ped_count"], axis=1)
        _df["time"] *= 0.4
        _df["seed"] = seed
        df.append(_df)
    df = pd.concat(df, axis=0, ignore_index=True, verify_integrity=False).sort_values(
        ["time", "seed"]
    )
    return df


class ThroughputPlotter(PlotUtil_):
    def __init__(self, run_map) -> None:
        super().__init__()
        self.run_map: RunMap = run_map

    @staticmethod
    def getLogFormatter():
        class LogFormatterCustom(LogFormatter):
            def _num_to_string(self, x, vmin, vmax):
                if x > 10000:
                    s = "%1.0e" % x
                elif x == -1:
                    s = "%1.0f" % x
                elif x < -1:
                    s = "%1.0e" % x
                else:
                    s = self._pprint_val(x, vmax - vmin)
                return s

        return LogFormatterCustom(
            base=10, labelOnlyBase=False, minor_thresholds=(2.5, 0.9)
        )

    # @with_axis
    def msce_over_target_rate_box(self, *, ax: plt.Axes = None):

        figsize = self.fig_size_mm(111, 50)
        fig, ax = self.check_ax(figsize=figsize)
        scenario = VadereTopographyPlotter(
            "../study/corridor_trace_5perSpawn_ramp_down.d/corridor_2x5m_d20_5perSpawn_ramp_down.out/BASIS_corridor_2x5m_d20_5perSpawn_ramp_down.out.scenario"
        )
        # scenario = VaderScenarioPlotHelper("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
        # same top for all. Just use first simulation
        m: DpmmMetaData = self.run_map[0].simulations[0].get_dcdMap().metadata
        _free, _covered = scenario.get_legal_cells(m.create_full_index_slice())
        self.cell_to_tex(
            _corridor_filter_target_cells(_free),
            c=5.0,
            fd=self.run_map.path("cell_tikz.tex"),
            attr=["cell"],
        )

        fig: plt.Figure = ax.get_figure()

        # get ascending order of target rates
        sg_names = dict(
            sorted(self.run_map.items(), key=lambda x: x[1].attr["target_rate_in_kbps"])
        ).keys()
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        colors = self.get_default_color_cycle()
        box_data = []
        pos = []
        for i, g in enumerate(sg_names):
            sg: SimulationGroup = self.run_map[g]
            if _hdf.contains_group(g):
                df = _hdf.get_dataframe(g)
            else:
                df = OppAnalysis.sg_get_msce_data(
                    sim_group=sg,
                    cell_count=len(_corridor_filter_target_cells(_free)),
                )
                _hdf.write_frame(group=g, frame=df)
            df_m = df.groupby("run_id").agg(
                ["mean", "std", percentile(25), percentile(75)]
            )
            if isinstance(df_m.columns, pd.MultiIndex):
                df_m = df_m.droplevel(0, axis=1)
            df_m["tp"] = int(sg.attr["map_target_rate"].replace("kbps", ""))
            # ax.scatter(df_m["tp"], df_m["mean"], marker="x", color="black")
            pos.append(sg.attr["target_rate_in_kbps"] / 8)
            box_data.append(df_m["mean"])
        df: List[pd.DataFrame] = [_d.reset_index(drop=True) for _d in box_data]
        box_df = pd.concat(
            df,
            axis=1,
            ignore_index=True,
        )
        m, t, b, func, r2 = self.exp_fit(pos, box_df)
        xs = np.linspace(0.5, 70, 200)
        ys = func(xs)
        ax.plot(xs, ys, linestyle="dashed", color="darkred", label="Exp. fit")
        h, l = ax.get_legend_handles_labels()
        h.append(pltPatch.Patch("white", "white"))
        l.append(f"$R^2={r2:.6f}$")
        ax.legend().remove()
        ax.legend(
            h,
            l,
            loc="lower left",
            frameon=True,
            fontsize="x-small",
        )

        w = 0.08
        width = lambda p, w: 10 ** (np.log10(p) + w / 2.0) - 10 ** (
            np.log10(p) - w / 2.0
        )
        b = ax.boxplot(box_data, positions=pos, widths=width(pos, w))
        c = list(repeat("gray", len(pos)))
        self.color_box_plot(b, fill_color=c, flier_size=5, flier="x", ax=ax)
        plt.setp(
            b["fliers"],
            mec="black",
            # mfc=colors[i],
            color="gray",
            mfc="white",
            marker=".",
            markersize=5,
        )

        ax.set_xlabel("Target transmission rate in kBps (log scale)")
        ax.set_ylabel("Mean squared\ncell error (MSCE)")
        ax.xaxis.set_label_coords(0.585, 0.099, transform=fig.transFigure)
        ax.set_xscale("log")
        _yminor_tick = 0.025 / 2
        ax.yaxis.set_major_locator(MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(MultipleLocator(0.05 / 5))
        ax.xaxis.set_minor_formatter(self.getLogFormatter())
        ax.tick_params(axis="x", which="minor")
        ax.set_ylim(0.25 - _yminor_tick, 0.41)
        ax.set_xlim(0.5, 70.0)
        ax.grid(visible=True, axis="both")
        ax.grid(visible=True, which="minor", axis="x", linestyle="--")

        fig.tight_layout()
        fig.subplots_adjust(
            left=0.2, bottom=0.22, right=0.995, top=0.99, wspace=0.08, hspace=0.15
        )
        fig.savefig(self.run_map.path("msce_over_target_rate_by_seed.pdf"))

    def exp_fit(self, pos, data):
        ys = data.mean().to_numpy()
        xs = np.array(pos)

        def _exp(x, m, t, b):
            return m * np.exp(-t * x) + b

        p0 = (1, 1, 0.25)
        ret = scipy.optimize.curve_fit(_exp, xs, ys, p0, full_output=True)
        m, t, b = ret[0]
        sq_diff = np.square(ys - _exp(xs, m, t, b))
        sq_diff_from_mean = np.square(ys - np.mean(ys))
        r2 = 1 - np.sum(sq_diff) / np.sum(sq_diff_from_mean)
        return m, t, b, lambda x: _exp(x, m, t, b), r2

    def plot_qq(self):
        figsize = self.fig_size_mm(111, 65)
        fig, ax = self.check_ax(figsize=figsize)

        data = ThroughputCache(self.run_map).qq.frame()

        for sg_name, _d in data.groupby("sg"):
            target_rate = self.run_map[sg_name].attr["target_rate_in_kbps"] / 8
            x = np.repeat(target_rate, len(_d))
            ax.scatter(x, _d, marker="x", color="black")

        ax.set_xlabel("Target Tx rate in kB/s")
        ax.set_ylabel("Actual Tx rate\nin kB/s")
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.xaxis.set_minor_formatter(self.getLogFormatter())
        ax.yaxis.set_minor_formatter(self.getLogFormatter())
        ax.tick_params(axis="both", which="minor", labelsize="medium")
        ax.set_xlim(0.5, 74.0)
        ax.set_ylim(0.5, 74.0)
        ax.grid(visible=True, axis="both", which="major")
        # ax.grid(visible=True, which="minor", axis="both", linestyle="--")
        fig.legend(
            [
                pltLines.Line2D([0], [0], color="darkred"),
                pltLines.Line2D([0], [0], marker="x", color="black", linestyle=""),
            ],
            ["QQ-Line", "Data"],
            loc="lower left",
            bbox_to_anchor=(72, -8),
            bbox_transform=ax.transData,
            handlelength=1.0,
            frameon=False,
        )

        x = np.linspace(-5, 75, num=200)
        ax.plot(x, x, color="darkred")
        ax.set_aspect("equal", adjustable="box")
        fig.tight_layout()
        fig.savefig(self.run_map.path("rate-qq-scatter_log.pdf"))

        ax.set_xscale("linear")
        ax.set_yscale("linear")
        ax.xaxis.set_major_locator(MultipleLocator(10))
        ax.yaxis.set_major_locator(MultipleLocator(10))
        ax.xaxis.set_minor_locator(MultipleLocator(5))
        ax.yaxis.set_minor_locator(MultipleLocator(5))
        ax.set_xlim(-1.0, 74.0)
        ax.set_ylim(-1.0, 75.0)
        fig.tight_layout()
        fig.subplots_adjust(
            left=0.0, bottom=0.22, right=1.0, top=0.99, wspace=0.08, hspace=0.15
        )
        fig.savefig(self.run_map.path("rate-qq-scatter_lin.pdf"))

    @with_axis
    def plot_target_rate_ts(self, *, ax: plt.Axes = None):

        # figsize = self.fig_size_cm(width=11.1, ratio=1 / 1.3)
        fig, ax = self.check_ax(figsize=(8.9, 1 + 8.9 / (16 / 9)))
        data = ThroughputCache(self.run_map).rates_over_time.frame()

        colors = self.get_default_color_cycle()
        target_rates = []
        for i, (sg_name, _d) in enumerate(data.groupby("sg")):
            rate = self.run_map[sg_name].attr["target_rate_in_kbps"] / 8
            self.fill_between(
                _d.reset_index().iloc[:, 1:],
                line_args=dict(color=colors[i], label=kBpsLbl(rate)),
                fill_args=dict(label="dummy"),
                ax=ax,
            )
            target_rates.append(rate)

        h, l = self.merge_legend_patches(*ax.get_legend_handles_labels())
        ax.hlines(target_rates, 0, 900, colors="red", linestyles="solid")
        for _rate in target_rates:
            if _rate == target_rates[0]:
                y = -1.0
            elif _rate == target_rates[1]:
                continue
            else:
                y = _rate
            ax.text(
                x=855,
                y=y,
                s=f"{_rate:.1f} kBps",
                color="red",
                transform=ax.transData,
            )
        ax.legend().remove()
        ax.grid(visible=False, axis="y")

        ax.set_xlabel("Simulation time in seconds")
        ax.set_ylabel("Tx rate in kBps")
        ax.set_xlim((0, 850))
        ax.set_ylim(-4, 70)
        ax.xaxis.set_major_locator(MultipleLocator(50))
        ax.xaxis.set_minor_locator(MultipleLocator(25))
        ax.yaxis.set_major_locator(MultipleLocator(5))
        ax.yaxis.set_minor_locator(MultipleLocator(2.5))
        ax.tick_params(axis="y", which="both", right=True)
        fig.tight_layout(rect=(0, 0.1, 1, 1))
        fig.legend(h, l, loc="lower center", ncol=5, fontsize="medium", frameon=False)
        fig.savefig(self.run_map.path("rates_over_time.pdf"))

    @with_axis
    def plot_ped_count(self, *, ax: plt.Axes = None) -> Tuple[plt.Figure, plt.Axes]:
        df = get_ped_count()
        df = (
            df.loc[:, ["time", "ped_count"]]
            .groupby(["time"])
            .agg(["mean", percentile(25), percentile(75)])
        )
        df = df.droplevel(0, axis=1).reset_index()
        self.fill_between(
            data=df,
            plot_lbl="mean ped. count with Q1/3",
            line_args=dict(color="black"),
            fill_args=dict(label="Q1/3"),
            ax=ax,
        )
        h, l = self.merge_legend_patches(*ax.get_legend_handles_labels())
        ax.legend().remove()
        ax.legend(h, l, loc="upper left")
        return ax.get_figure(), ax

    @with_axis
    def plot_target_rate_diff(self, *, freq: float = 25.0, ax: plt.Axes = None):
        """Not used in paper"""
        # get ascending order of target rates
        fig1, (ax1a, ax1b) = plt.subplots(2, 1, figsize=(16, 9))
        fig2, (ax2a, ax2b) = plt.subplots(2, 1, figsize=(16, 9))

        sg_names = dict(
            sorted(self.run_map.items(), key=lambda x: x[1].attr["target_rate_in_kbps"])
        ).keys()
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        colors = self.get_default_color_cycle()
        sg_data_relative: List[pd.DataFrame] = []
        sg_data_abs: List[pd.DataFrame] = []
        stat: List[pd.DataFrame] = []
        for i, g in enumerate(sg_names):
            sg: SimulationGroup = self.run_map[g]
            for id, _, sim in sg.simulation_iter(enum=True):
                sim: Simulation
                target_rates = dict(
                    d_map=sg.attr["target_rate_in_kbps"] / 8, d_beacon=50 / 8
                )
                tx = NodeTxData(sim.dpmm_cfg.node_tx.path)
                data = tx.get_tx_throughput_diff_by_app(
                    bin_size=freq, throughput_unit=1000, target_rates=target_rates
                )
                data = data.loc[200:]
                relative_diff = data["diff_d_map"] / data["d_map"]
                relative_abs = data["diff_d_map"]

                sg_data_relative.append(relative_diff.reset_index())
                sg_data_abs.append(relative_abs.reset_index())

                self.plot_ecdf(
                    relative_diff, ax=ax1a, label=kBpsLbl(sg), color=colors[i]
                )
                self.plot_ecdf(
                    relative_abs, ax=ax2a, label=kBpsLbl(sg), color=colors[i]
                )
            # relative
            sg_data_relative = pd.concat(
                sg_data_relative, axis=0, ignore_index=True, verify_integrity=False
            )
            stat.append(
                sg_data_relative.iloc[:, [1]]
                .describe()
                .set_axis([("relative", f"{kBpsLbl(sg)}")], axis=1)
            )
            sg_data_relative = (
                sg_data_relative.groupby("time")
                .agg(["mean", percentile(25), percentile(75)])
                .droplevel(0, axis=1)
            )
            self.fill_between(
                sg_data_relative.reset_index(),
                line_args=dict(color=colors[i], label=kBpsLbl(sg)),
                fill_args=dict(label="dummy"),
                ax=ax1b,
            )
            sg_data_relative = []
            # absolute
            sg_data_abs = pd.concat(
                sg_data_abs, axis=0, ignore_index=True, verify_integrity=False
            )
            stat.append(
                sg_data_abs.iloc[:, [1]]
                .describe()
                .set_axis([("absolute", f"{kBpsLbl(sg)}")], axis=1)
            )
            sg_data_abs = (
                sg_data_abs.groupby("time")
                .agg(["mean", percentile(25), percentile(75)])
                .droplevel(0, axis=1)
            )
            self.fill_between(
                sg_data_abs.reset_index(),
                line_args=dict(color=colors[i], label=kBpsLbl(sg)),
                fill_args=dict(label="dummy"),
                ax=ax2b,
            )
            sg_data_abs = []

        stat = pd.concat(stat, axis=1)
        stat.columns = pd.MultiIndex.from_tuples(
            list(stat.columns), names=["diff", "rate"]
        )
        stat = stat.sort_values(["diff", "rate"], axis=1).T.iloc[:, 1:]
        stat.to_csv(self.run_map.path("diff_stats.csv"))
        # tbl, ax = self.df_to_table(df=stat.applymap("{:.2f}".format).reset_index())
        # tbl.savefig(self.run_map.path("diff_stats.pdf"))

        def as_float(*kw, **kwargs):
            return kw[0].apply(lambda x: x.replace("kBps", "").strip()).astype(float)

        _i = pd.IndexSlice
        stat_rel = stat.loc[_i["relative", :], :].reset_index()
        stat_rel = stat_rel.sort_values(by="rate", key=as_float).iloc[:, 1:]
        save_as_tex_table(
            stat_rel,
            self.run_map.path("diff_stat_rel.tex"),
            selected_only=False,
            col_format=numeric_formatter(precision=3),
        )
        stat_abs = stat.loc[_i["absolute", :], :].reset_index()
        stat_abs = stat_abs.sort_values(by="rate", key=as_float).iloc[:, 1:]
        save_as_tex_table(
            stat_abs,
            self.run_map.path("diff_stat_abs.tex"),
            selected_only=False,
            col_format=numeric_formatter(precision=3),
        )

        # absolute ecdf
        ax2a.xaxis.set_major_locator(MultipleLocator(0.5))
        ax2a.xaxis.set_minor_locator(MultipleLocator(0.25))
        ax2a.yaxis.set_major_locator(MultipleLocator(0.1))
        ax2a.yaxis.set_minor_locator(MultipleLocator(0.05))
        ax2a.set_xlabel("Absolute target rate difference in kBps")
        h, l = ax2a.get_legend_handles_labels()
        _h = []
        _l = []
        for i in range(len(sg_names)):
            _h.append(h[i * len(sg)])
            _l.append(l[i * len(sg)])
        ax2a.legend().remove()
        ax2a.legend(_h, _l, loc="lower right", ncol=2)

        # absolute ts
        h, l = self.merge_legend_patches(*ax2b.get_legend_handles_labels())
        ax2b.legend().remove()
        ax2b.legend(h, l, loc="lower center", ncol=5)
        ax2b.yaxis.set_major_locator(MultipleLocator(1.0))
        ax2b.yaxis.set_minor_locator(MultipleLocator(0.5))
        ax2b.xaxis.set_major_locator(MultipleLocator(50))
        ax2b.xaxis.set_minor_locator(MultipleLocator(25))
        # ax1b.set_ylim(-0.4, 0.6)
        ax2b.set_ylim(-2, 6.5)
        ax2b.set_xlabel("Simulation time in seconds")
        ax2b.set_ylabel("Absolute target rate difference in kBps")
        fig2.tight_layout()
        fig2.savefig(self.run_map.path("absolute_rate_diff.pdf"))

        # relative
        ax1a.xaxis.set_major_locator(MultipleLocator(0.2))
        ax1a.xaxis.set_minor_locator(MultipleLocator(0.1))
        ax1a.yaxis.set_major_locator(MultipleLocator(0.1))
        ax1a.set_xlabel("Relative difference to target rate")
        h, l = ax1a.get_legend_handles_labels()
        _h = []
        _l = []
        for i in range(len(sg_names)):
            _h.append(h[i * len(sg)])
            _l.append(l[i * len(sg)])
        ax1a.legend().remove()
        ax1a.legend(_h, _l, loc="upper left")

        h, l = self.merge_legend_patches(*ax1b.get_legend_handles_labels())
        ax1b.legend().remove()
        ax1b.legend(h, l, loc="lower center", ncol=3)
        ax1b.yaxis.set_major_locator(MultipleLocator(0.1))
        ax1b.xaxis.set_major_locator(MultipleLocator(50))
        ax1b.xaxis.set_minor_locator(MultipleLocator(25))
        ax1b.set_ylim(-0.4, 0.6)
        ax1b.set_xlabel("Simulation time in seconds")
        ax1b.set_xlabel("Relative difference to target rate")

        fig1.tight_layout()
        fig1.savefig(self.run_map.path("relative_rate_diff.pdf"))


class MemberEstPlotter(PlotUtil_):
    def __init__(self, run_map) -> None:
        super().__init__()
        self.run_map: RunMap = run_map

    # def _collect_tx_throughput(self, sg: SimulationGroup, freq="20_0"):
    #     df = []
    #     sim: Simulation
    #     for _, id, sim in sg.simulation_iter(enum=True):
    #         _df = sim.from_hdf("tk_pkt_bytes.h5", f"tx_throughput{freq}")
    #         _df["run"] = id
    #         df.append(_df)
    #     return pd.concat(df, axis=0, verify_integrity=False).sort_index()

    def _collect_tx_throughput(self, sg: SimulationGroup, freq="20_0"):
        df = []
        sim: Simulation
        for _, id, sim in sg.simulation_iter(enum=True):
            tx = NodeTxData(sim.dpmm_cfg.node_tx.path)
            _df = tx.tx_bytes_per_app(app_names=["b", "m"])
            _df = sim.from_hdf("tk_pkt_bytes.h5", f"tx_throughput{freq}")
            _df["run"] = id
            df.append(_df)
        return pd.concat(df, axis=0, verify_integrity=False).sort_index()

    def _get_msce_over_tp(self, _hdf: BaseHdfProvider, sg: SimulationGroup):
        scenario = VadereTopographyPlotter(
            "../study/corridor_trace_5perSpawn_ramp_down.d/corridor_2x5m_d20_5perSpawn_ramp_down.out/BASIS_corridor_2x5m_d20_5perSpawn_ramp_down.out.scenario"
        )
        g = sg.group_name
        # scenario = VaderScenarioPlotHelper("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
        # same top for all. Just use first simulation
        m: DpmmMetaData = self.run_map[0].simulations[0].get_dcdMap().metadata
        _free, _covered = scenario.get_legal_cells(m.create_full_index_slice())
        if _hdf.contains_group(g):
            _df = _hdf.get_dataframe(g)
        else:
            _df = OppAnalysis.sg_get_msce_data(
                sim_group=sg,
                cell_count=len(_corridor_filter_target_cells(_free)),
            )
            _hdf.write_frame(group=g, frame=_df)
        return _df

    @with_axis
    def plot_ped_count(self, *, ax: plt.Axes = None) -> Tuple[plt.Figure, plt.Axes]:
        df = get_ped_count()
        df = (
            df.loc[:, ["time", "ped_count"]]
            .groupby(["time"])
            .agg(["mean", percentile(25), percentile(75)])
        )
        df = df.droplevel(0, axis=1).reset_index()
        self.fill_between(
            data=df,
            plot_lbl="mean ped. count with Q1/3",
            line_args=dict(color="black"),
            fill_args=dict(label="Q1/3"),
            ax=ax,
        )
        return ax.get_figure(), ax

    def plot_msce(self, with_random: bool = False):
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        box_data = []
        pos = []
        for g in ["ConstantRate-", "nTable-500kbps", "map-500kbps"]:
            sg: SimulationGroup = self.run_map[g]
            _df = self._get_msce_over_tp(_hdf, sg)
            df_m = _df.groupby("run_id").agg(
                ["mean", "std", percentile(25), percentile(75)]
            )
            if isinstance(df_m.columns, pd.MultiIndex):
                df_m = df_m.droplevel(0, axis=1)
            pos.append(sg.attr["member_estimate"])
            box_data.append(df_m["mean"])
        colors = self.get_default_color_cycle()[0:3]
        colors = [colors[-1], colors[0], colors[1]]
        positions = [10, 20, 30]
        labels = ["Constant\nrate", "Neighbor count\nestimate", "Map count\nestiamte"]
        if with_random:
            colors.append(self.get_default_color_cycle()[4])
            _df = RandomMap(self.run_map).get_random_draw()
            box_data.append(_df["msme"])
            positions = [10, 20, 30, 40]
            labels = [
                "Constant\nrate",
                "Neighbor\ncount est.",
                "Map count\nest.",
                "Random",
            ]

        figsize = self.fig_size_cm(width=11.1, height=11.5)
        fig, _ax = plt.subplot_mosaic(
            "zzaaaaaaaa;zzaaaaaaaa;zzaaaaaaaa;zzaaaaaaaa;bbbbbbbbbb;bbbbbbbbbb;bbbbbbbbbb",
            figsize=figsize,
        )
        _null = _ax["z"]
        _null.set_axis_off()
        ax = _ax["a"]
        tbl = _ax["b"]
        b = ax.boxplot(box_data, positions=positions, widths=3.5)
        ax.set_xticklabels(labels)
        ax.yaxis.set_major_locator(MultipleLocator(0.025))
        ax.yaxis.set_minor_locator(MultipleLocator(0.025 / 5))
        ax.set_ylim(0.175, 0.275)
        ax.grid(visible=True, which="major", axis="both")
        self.move_last_y_ticklabel_down(ax, lbl_str="0.275")
        ax.set_ylabel("Mean squared map error\n(MSME)")
        ax.yaxis.set_label_coords(-0.14, 0.45)
        self.color_box_plot(b, fill_color=colors, ax=ax)
        box_data: List[pd.DataFrame] = [_df.reset_index(drop=True) for _df in box_data]
        df = pd.concat(box_data, axis=1, ignore_index=True)
        df = df.describe().iloc[1:, :]
        df = df.applymap(lambda x: f"{x:.4f}")
        df.columns = labels
        df.index.name = ""
        t: plt.Table
        _, _, t = self.df_to_table(
            df.reset_index(),
            ax=tbl,
            col_width=[5.6, 6, 11.8, 6],
            use_col_labels=False,
            cell_height=0.08,
            col_header_height=0.08,
            cell_align=["right", "center", "center", "center"],
        )
        t.set_fontsize("small")
        _rows, _ = mpl_table_cell_iter._tbl_dim(t)
        for _key, cell in mpl_table_cell_iter.by_col(t):
            row = mpl_table_cell_iter.row(_key)
            if row == 0:
                brtl = "T"
            elif row == _rows - 1:
                brtl = "B"
            else:
                brtl = "open"
            cell.visible_edges = brtl

        fig.tight_layout()
        fig.subplots_adjust(left=0.01, bottom=0.005, right=0.99, top=0.995, hspace=2.5)
        try:
            shutil.copy(
                self.run_map.path("member_est_msme.pdf"),
                self.run_map.path("member_est_msme_old.pdf"),
            )
        except:
            pass
        fig.savefig(self.run_map.path("member_est_msme.pdf"))

        print("done")

    @with_axis
    def plot_tx_interval(self, freq=1.0, *, ax: plt.Axes = None):
        _hdf = BaseHdfProvider(self.run_map.path("tx_interval.h5"))
        color = self.get_default_color_cycle()[0:3]
        ls = ["--", ":", "o"]
        for g in ["nTable-500kbps", "map-500kbps"]:
            sg: SimulationGroup = self.run_map[g]
            if _hdf.contains_group(g):
                df = _hdf.get_dataframe(g)
            else:
                df = []
                for idx, sim in sg.simulation_iter():
                    x = NodeTxData(hdf_path=sim.dpmm_cfg.node_tx.path)
                    d = x.hdf.select(
                        key="d_beacon/tx_interval", columns=["time", "tx_interval"]
                    ).reset_index()
                    d["run_id"] = idx
                    df.append(d)

                df = pd.concat(df)
                _hdf.write_frame(group=g, frame=df)
            data = self.append_bin(
                data=df[["time", "tx_interval"]],
                bin_size=freq,
                start=0.0,
                agg=["mean", percentile(25), percentile(75)],
            )
            idx = list(self.run_map.keys()).index(g)
            self.fill_between(
                data=data.loc[
                    :,
                    [
                        "bin_right",
                        "tx_interval_mean",
                        "tx_interval_p25",
                        "tx_interval_p75",
                    ],
                ],
                line_args=dict(
                    color=color[idx],
                    linestyle=ls[idx],
                    label=f"Mean tx interval: {sg.attr['lbl']} with Q1/3",
                ),
                fill_args=dict(label="dummy"),
                ax=ax,
            )

        h, l = self.merge_legend_patches(*ax.get_legend_handles_labels())
        ax.legend().remove()
        ax.legend(h, l, loc="lower right", ncol=2)
        try:
            shutil.copy(
                self.run_map.path("tx_interval.pdf"),
                self.run_map.path("tx_interval_old.pdf"),
            )
        except:
            pass
        ax.get_figure().savefig(self.run_map.path("tx_interval.pdf"))

    @with_axis
    def plot_throughput_ts_fill(
        self, bin="25_0", *, ax: plt.Axes = None
    ) -> Tuple[plt.Figure, plt.Axes]:
        color = self.get_default_color_cycle()[0:3]
        marker = ["|", ".", "x"]
        fig: plt.Figure = ax.get_figure()
        for i, (g, sg) in enumerate(self.run_map.items()):
            df = self._collect_tx_throughput(sg, bin).loc[:, ["m"]].reset_index()
            df = df.groupby("time").agg(["mean", percentile(25), percentile(75)])
            # df = df.groupby("time").agg(["mean", "min", "max"])
            df = df.droplevel(0, axis=1)
            self.fill_between(
                data=df.reset_index(),
                plot_lbl=f"Mean tx-rate: {sg.attr['lbl']}",
                line_args=dict(color=color[i], marker=marker[i]),
                fill_args=dict(label=f"Q1/3: {sg.attr['lbl']}"),
                ax=ax,
            )
        t = np.sort(df.index.unique())
        ax.hlines(
            500 / 8,
            0,
            t.max() + 25,
            color="darkred",
            linestyles="dashed",
        )
        # ax.text(
        #     50,
        #     100,
        #     kBpsLbl(500),
        #     color="darkred",
        #     fontsize="small",
        #     transform=ax.transData,
        # )
        ax.annotate(
            kBpsLbl(500),
            xy=(75, 500 / 8),
            xycoords="data",
            xytext=(120, 90),
            textcoords="data",
            size="small",
            va="center",
            ha="center",
            color="darkred",
            arrowprops=dict(
                arrowstyle="->",
                facecolor="darkred",
                edgecolor="darkred",
                connectionstyle="arc3,rad=-0.2",
            ),
        )

        ax.set_ylabel("Throughput in kB/s")
        ax.set_xlabel("Simulation time in seconds")
        ax.yaxis.set_major_locator(MultipleLocator(20))
        ax.yaxis.set_minor_locator(MultipleLocator(10))
        ax.set_ylim(0, 160)
        ax.xaxis.set_major_locator(MultipleLocator(50))
        ax.xaxis.set_minor_locator(MultipleLocator(25))
        ax.set_xlim(25, 875)
        ax.legend(loc="upper left", ncol=2)
        self.move_last_y_ticklabel_down(ax, "160")
        return fig, ax

    def get_dmap_and_nt_ts_data(self, cache_name: str = "map_nt.h5"):

        _hdf = BaseHdfProvider(self.run_map.path(cache_name))
        if not (_hdf.contains_group("map") and _hdf.contains_group("nt")):
            sg: SimulationGroup = self.run_map["nTable-500kbps"]
            g = sg.group_name
            for run_id, _, sim in sg.simulation_iter(enum=True):
                print(f"{run_id}-{g}")
                sim: Simulation

                _nt = sim.sql.vec_data(
                    sim.sql.m_table(), "tableSize:vector", drop="vectorId"
                )
                nt.append(_nt)
            sg: SimulationGroup = self.run_map["map-500kbps"]
            g = sg.group_name
            for run_id, _, sim in sg.simulation_iter(enum=True):
                print(f"{run_id}-{g}")
                sim: Simulation
                map_count_error = MapCountError(
                    hdf_path=sim.dpmm_cfg.map_count_error.path
                )
                _dmap = (
                    map_count_error.hdf_map_measure.select(columns=["map_mean_count"])
                    .reset_index()
                    .set_axis(["time", "value"], axis=1)
                )
                dmap.append(_dmap)

            dmap = pd.concat(dmap, axis=0, ignore_index=True, verify_integrity=False)
            nt = pd.concat(nt, axis=0, ignore_index=True, verify_integrity=False)
            _hdf.write_frame("map", dmap)
            _hdf.write_frame("nt", nt)
        else:
            dmap = _hdf.get_dataframe("map")
            nt = _hdf.get_dataframe("nt")
        return dmap, nt

    def plot_throughput_ped_count_ts(self, bin="25_0"):
        rc = {
            "legend.fontsize": "small",
            "axes.labelsize": "large",
            "xtick.labelsize": "medium",
            "ytick.labelsize": "medium",
        }
        with plt.rc_context(rc=rc):
            figsize = self.fig_size_cm(width=11.1, ratio=1 / 1.3)
            fig, axes = plt.subplot_mosaic(
                mosaic="T;T;T;P;P", figsize=figsize, sharex=True
            )

            ax_t: plt.Axes = axes["T"]
            self.plot_throughput_ts_fill(bin=bin, ax=axes["T"])

            ax_p: plt.Axes = axes["P"]
            self.plot_ped_count(ax=ax_p)
            self.plot_dmap_and_nt_ts(ax=ax_p)

            # legend for lower 'P' plot.
            h, l = self.merge_legend_patches(*ax_p.get_legend_handles_labels())
            ax_p.legend().remove()
            ax_p.legend(
                h,
                l,
                loc="lower right",
                handlelength=1.3,
                handletextpad=0.5,
                bbox_to_anchor=(873, 1),
                bbox_transform=ax_p.transData,
                borderpad=0.2,
                borderaxespad=0,
            )

            ax_p.set_xlabel("Simulation time in seconds")
            ax_p.xaxis.set_major_locator(MultipleLocator(100))
            ax_p.xaxis.set_minor_locator(MultipleLocator(25))

            ax_p.set_ylabel("Ped. count")
            ax_p.yaxis.set_major_locator(MultipleLocator(50))
            ax_p.yaxis.set_minor_locator(MultipleLocator(10))
            ax_p.set_ylim(0, 175)

            ss_start = 380
            ss_end = 780
            ax_t.vlines(
                [ss_start, ss_end],
                0,
                160,
                colors="gray",
                linestyles="dashdot",
                zorder=0,
            )
            ax_p.vlines(
                [ss_start, ss_end],
                0,
                180,
                colors="gray",
                linestyles="dashdot",
                zorder=0,
            )
            ax_t.annotate(
                "",
                xy=(ss_start, 110),
                xycoords="data",
                xytext=(ss_end, 110),
                textcoords="data",
                arrowprops=dict(arrowstyle="<->", color="gray"),
            )
            ax_t.annotate(
                "Pedestrian flow\nsteady state",
                xy=(ss_start, 110),
                xycoords="data",
                xytext=((ss_start + (780 - ss_start) / 2), 111),
                textcoords="data",
                ha="center",
                va="bottom",
                color="gray",
                fontsize="small",
            )

            # legend for upper 'T' plot.
            h, l = ax_t.get_legend_handles_labels()
            l = [l_.replace("Constant rate", "Constant rate*") for l_ in l]
            h_new = []
            l_new = []
            i = 0
            while i < len(h):
                if l[i].startswith("Mean"):
                    h_new.append((h[i], h[i + 1]))
                    l_new.append(f"{l[i]} with Q1/3")
                    i += 2
                else:
                    h_new.append(h[i])
                    l_new.append(l[i])
                    i += 1
            l_new.append("* (no adaption)")
            h_new.append(pltPatch.Patch(edgecolor="white", facecolor="white"))

            l_new = [_l.replace("Mean tx-rate: ", "") for _l in l_new]

            ax_t.get_legend().remove()
            _legend = ax_t.legend(
                h_new,
                l_new,
                loc="lower right",
                handlelength=1.3,
                handletextpad=0.5,
                bbox_to_anchor=(873, 1),
                bbox_transform=ax_t.transData,
                borderpad=0.2,
                borderaxespad=0,
            )
            _legend.get_texts()[1].set_weight("bold")
            ax_t.set_xlabel("")

            fig.tight_layout(w_pad=0.05, h_pad=0.05, rect=(0, 0, 1, 1), pad=1.02)
            fig.subplots_adjust(left=0.158, bottom=0.1, right=0.99, top=0.99)
            shutil.copy(
                self.run_map.path(f"throughput_pedcount_ts_{bin}.pdf"),
                self.run_map.path(f"throughput_pedcount_ts_{bin}_old.pdf"),
            )
            fig.savefig(self.run_map.path(f"throughput_pedcount_ts_{bin}.pdf"))

    @with_axis
    def plot_throughput_ts_box(self, *, ax: plt.Axes = None):
        df = []
        color = self.get_default_color_cycle()[0:3]
        _xoffset = -10

        ax_zoom = ax.inset_axes([175, 0, 700, 50], transform=ax.transData)
        for i, (g, sg) in enumerate(self.run_map.items()):
            df = self._collect_tx_throughput(sg, "50_0").loc[:, ["m"]].reset_index()
            t = np.sort(df["time"].unique())
            pos = t - _xoffset
            _xoffset += 10
            for _a in [ax, ax_zoom]:
                _, bp = df.groupby(["time"]).boxplot(
                    subplots=False,
                    column="m",
                    positions=pos,
                    rot=90,
                    widths=9,
                    showmeans=True,
                    grid=True,
                    return_type="both",
                    ax=_a,
                )
                self.color_box_plot(bp, fill_color=color[i], ax=_a)

        ax.hlines(
            500 / 8,
            0,
            t.max() + 25,
            color="red",
            linestyles="dashed",
            label="Target rate",
        )

        ax.set_ylabel("Throughput in kB/s")
        ax.set_xlabel("Simulation time in seconds")
        for _a in [ax, ax_zoom]:
            _a.yaxis.set_major_locator(MultipleLocator(10))
            _a.yaxis.set_minor_locator(MultipleLocator(5))
            _a.xaxis.reset_ticks()
            _a.xaxis.set_major_locator(FixedLocator(t))
            _a.xaxis.set_major_formatter(FixedFormatter(t))
        ax.set_xlim(25, t.max() + 25)

        ax_zoom.hlines(
            500 / 8,
            0,
            t.max() + 25,
            color="red",
            linestyles="dashed",
            label="Target rate",
        )
        ax_zoom.set_xlim(175, 875)
        ax_zoom.set_ylim(60, 95)
        ax_zoom.set_xticklabels([])
        ax_zoom.set_yticklabels([])
        for spine in ax_zoom.spines.values():
            spine.set_edgecolor("black")

        ax.indicate_inset_zoom(ax_zoom, edgecolor="black", linewidth=2.0)

        lines = [
            pltLines.Line2D([0], [0], color="red", linestyle="dashed"),
        ]
        lines.extend([pltPatch.Patch(edgecolor="black", facecolor=c) for c in color])
        labels = ["Target rate", *[sg.attr["lbl"] for sg in self.run_map.values()]]
        ax.legend(handles=lines, labels=labels, loc="upper left")
        fig = ax.get_figure()
        fig.tight_layout()
        fig.savefig(self.run_map.path("throughput_ts_for_member_estiamtes_pox.pdf"))

    @with_axis
    def plot_dmap_and_nt_ts(self, *, ax: plt.Axes):

        dmap, nt = self.get_dmap_and_nt_ts_data()
        # with right
        dmap = (
            self.append_bin(
                dmap,
                start=0.0,
                bin_size=1.0,
                closed="right",
                agg=["mean", percentile(25), percentile(75)],
            )
            .reset_index(drop=True)
            .iloc[
                :,
                [
                    -1,
                    0,
                    1,
                    2,
                ],
            ]
        )
        nt = (
            self.append_bin(
                nt,
                start=0.0,
                bin_size=1.0,
                closed="right",
                agg=["mean", percentile(25), percentile(75)],
            )
            .reset_index(drop=True)
            .iloc[
                :,
                [
                    -1,
                    0,
                    1,
                    2,
                ],
            ]
        )

        c1, c2 = self.get_default_color_cycle()[:2]

        self.fill_between(
            data=nt,
            line_args=dict(label="Neighbor count with Q1/3", color=c1),
            fill_args=dict(label="dummy"),
            ax=ax,
        )
        self.fill_between(
            data=dmap,
            line_args=dict(label="Map count with Q1/3", color=c2),
            fill_args=dict(label="dummy"),
            ax=ax,
        )


def main():
    study_out = SIM_RESULT_DIR
    os.makedirs(study_out, exist_ok=True)

    r1 = RunMap.load_or_create(
        create_f=create_target_rate_run_map_new,
        output_path=os.path.join(study_out, "s1"),
        load_if_present=True,
    )
    ThroughputCache(r1).get()
    r1_plotter = ThroughputPlotter(r1)
    r1_plotter.plot_qq()
    r1_plotter.msce_over_target_rate_box()
    r1_plotter.plot_target_rate_ts()

    r2 = RunMap.load_or_create(
        create_f=create_member_estimate_run_map,
        output_path=os.path.join(study_out, "s0"),
        load_if_present=True,
    )
    r2_plotter = MemberEstPlotter(r2)
    r2_plotter.plot_throughput_ped_count_ts(bin="25_0")
    r2_plotter.plot_msce()
    r2_plotter.plot_tx_interval(freq=25.0)

    rnd = RandomMap(r2)
    rnd.plot_random_msme()
    print("done")


if __name__ == "__main__":

    main()
