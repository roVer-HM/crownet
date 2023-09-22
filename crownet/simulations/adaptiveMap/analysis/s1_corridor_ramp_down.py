import os
import glob
import re
from typing import List, Tuple
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType
from matplotlib.ticker import (
    MultipleLocator,
    FixedLocator,
    FixedFormatter,
    LogFormatter,
)
from crownetutils.analysis.common import (
    RunContext,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from itertools import repeat
from crownetutils.analysis.dpmm.metadata import DpmmMetaData
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.vadere.plot.topgraphy_plotter import VadereTopographyPlotter
from crownetutils.utils.dataframe import numeric_formatter, save_as_tex_table
from crownetutils.utils.parallel import run_kwargs_map
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.utils.plot import PlotUtil_, percentile, with_axis
import pandas as pd
import numpy as np
import scipy
import matplotlib.pyplot as plt
import matplotlib.patches as pltPatch
import matplotlib.lines as pltLines

from crownetutils.utils.styles import (
    load_matplotlib_style,
    STYLE_TEX,
)

load_matplotlib_style(STYLE_TEX)


def kBpsLbl(kbps):
    if isinstance(kbps, SimulationGroup):
        kbps = kbps.attr["target_rate_in_kbps"]
    kBps = kbps / 8
    return f"{kBps:.1f} kBps"


def __func_prepare_throughput_cache(
    sim: Simulation, sg: str, run_id: int, freq: float, target_rate: float
):
    df = OppAnalysis.get_sent_packet_throughput_diff_by_app(
        sim.sql,
        target_rate,
        freq,
        sim.get_base_provider("root", sim.path("tx_pkt_bytes.h5")),
    )
    return sg, run_id, df


def prepare_throughput_cache(run_map: RunMap, freq: float = 25.0):

    kw_list = []
    sg: SimulationGroup
    for g, sg in run_map.items():
        for id, _, sim in sg.simulation_iter(enum=True):
            kw_list.append(
                dict(
                    sim=sim,
                    sg=g,
                    run_id=id,
                    freq=freq,
                    target_rate=dict(m=sg.attr["target_rate_in_kbps"] / 8, b=50 / 8),
                )
            )
    ret = run_kwargs_map(
        __func_prepare_throughput_cache, kwargs_iter=kw_list, pool_size=5
    )
    # ret = __func_prepare_throughput_cache(**kw_list[0])
    print("hi")


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


def sg_tx_throughput_cache(sg: SimulationGroup, freq: float):
    kw_list = [dict(sim=sim, freq=freq) for sim in sg.simulations]
    print(f"preparing {len(kw_list)} simulations for tx_cache")
    run_kwargs_map(sim_tx_throughput_cache, kw_list, pool_size=20)


def sim_tx_throughput_cache(sim: Simulation, freq: float):
    _hdf = sim.get_base_provider("tk_pkt_bytes", sim.path("tk_pkt_bytes.h5"))
    g2 = f"tx_throughput{str(freq).replace('.', '_')}"
    if _hdf.contains_group(_hdf.group) and _hdf.contains_group(g2):
        print("nothing to do")
        return
    if not _hdf.contains_group(_hdf.group):
        print(f"read tx bytes {sim.label}")
        tx_pkt = OppAnalysis.get_sent_packet_bytes_by_app(sim.sql)
        tx_pkt = tx_pkt.reset_index().set_index(["app", "time"]).sort_index()
        print("write tx bytes to hdf")
        _hdf.write_frame(_hdf.group, tx_pkt)
    else:
        tx_pkt = _hdf.get_dataframe()
    tx_rate = tx_pkt.reset_index(["app"])

    if not _hdf.contains_group(g2):
        print(f"create throughput based on interval {freq} for sim: {sim.label}")
        bins = pd.interval_range(
            start=0.0,
            end=tx_rate.index.get_level_values(0).max(),
            freq=freq,
            closed="right",
        )
        # rate in kilo bit per seconds
        tx_rate = (
            (tx_rate.groupby([pd.cut(tx_rate.index, bins), "app"]).sum() / freq / 1000)
            .unstack("app")
            .droplevel(0, axis=1)
        )
        tx_rate.index = bins.right
        tx_rate.index.name = "time"
        _hdf.write_frame(g2, tx_rate)


def create_member_estimate_run_map(output_path, *args, **kwargs) -> RunMap:
    """All runs with different member estimates (neighborhood Table, Density Map, No adaption)"""
    os.makedirs(output_path, exist_ok=True)
    study_base = "/mnt/data1tb/results/arc-dsa_single_cell/s0_corridor_500kbps_map_table_count_est_and_no_rate_limit/"
    base_cfg = DpmmCfg.load(
        str_fd=os.path.join(study_base, "map_cfg_tmpl.json"), base_dir="foo"
    )
    base_cfg.base_dir = None

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

        # set cfg
        for s in data:
            s.dpmm_cfg = base_cfg.copy(base_dir=s.data_root)

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
    study_base = "/mnt/data1tb/results/arc-dsa_single_cell/s1_corridor_5_to_500kbps_map_count_est_only/"
    base_cfg = DpmmCfg.load(
        str_fd=os.path.join(study_base, "map_cfg_tmpl.json"), base_dir="foo"
    )
    base_cfg.base_dir = None

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
        # set cfg
        for s in data:
            s.dpmm_cfg = base_cfg.copy(base_dir=s.data_root)
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
    root_target_rate_500 = "/mnt/data1tb/results/s1_corridor_ramp_down/"
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
    root_target_rate_lt500 = (
        "/mnt/data1tb/results/s1_corridor_ramp_down_rate_lt500kbps/"
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

    root_target_rate_lt500 = (
        "/mnt/data1tb/results/s1_corridor_ramp_down_rate_lt500kbps_1/"
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

        rc = {
            "axes.labelsize": "x-large",
            "xtick.labelsize": "x-large",
            "ytick.labelsize": "x-large",
        }
        with plt.rc_context(rc):
            fig, ax = self.check_ax(figsize=(8.9, 8.9 / (16 / 9)))
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
                sorted(
                    self.run_map.items(), key=lambda x: x[1].attr["target_rate_in_kbps"]
                )
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
                        cell_slice_fc=_corridor_filter_target_cells,
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

            box_df = pd.concat(
                [_d.reset_index(drop=True) for _d in box_data],
                axis=1,
                ignore_index=True,
            )
            m, t, b, func, r2 = self.exp_fit(pos, box_df)
            xs = np.linspace(0.5, 70, 200)
            ys = func(xs)
            ax.plot(xs, ys, linestyle="dashed", color="red", label="Exp. fit")
            h, l = ax.get_legend_handles_labels()
            h.append(pltPatch.Patch("white", "white"))
            l.append(f"$R^2={r2:.6f}$")
            ax.legend().remove()
            ax.legend(h, l, loc="lower left")

            w = 0.08
            width = lambda p, w: 10 ** (np.log10(p) + w / 2.0) - 10 ** (
                np.log10(p) - w / 2.0
            )
            b = ax.boxplot(box_data, positions=pos, widths=width(pos, w))
            c = list(repeat("gray", len(pos)))
            self.color_box_plot(b, fill_color=c, flier_size=5, flier="o", ax=ax)
            plt.setp(
                b["fliers"],
                mec="black",
                # mfc=colors[i],
                color="gray",
                mfc="white",
                marker="o",
                markersize=5,
            )

            ax.set_xlabel("Target transmission rate in kBps (log scale)")
            ax.set_ylabel("Mean squared map error (MSME)")
            ax.set_xscale("log")
            _yminor_tick = 0.025 / 2
            ax.yaxis.set_major_locator(MultipleLocator(0.025))
            ax.yaxis.set_minor_locator(MultipleLocator(0.025 / 2))
            ax.xaxis.set_minor_formatter(self.getLogFormatter())
            ax.tick_params(axis="x", which="minor", labelsize="large")
            ax.set_ylim(0.25 - _yminor_tick, 0.50)
            ax.set_xlim(0.5, 70.0)
            ax.grid(visible=True, axis="both")
            ax.grid(visible=True, which="minor", axis="x", linestyle="--")

            fig.tight_layout()
            fig.savefig(self.run_map.path("msme_over_target_rate_by_seed.pdf"))

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

    @with_axis
    def plot_target_rate_ts(self, *, ax: plt.Axes = None):

        rc = {
            "axes.labelsize": "x-large",
            "xtick.labelsize": "x-large",
            "ytick.labelsize": "x-large",
        }
        with plt.rc_context(rc):
            fig, ax = self.check_ax(figsize=(8.9, 1 + 8.9 / (16 / 9)))
            fig2, ax2 = self.check_ax(figsize=(5.9, 5.9))
            sg_names = list(
                dict(
                    sorted(
                        self.run_map.items(),
                        key=lambda x: x[1].attr["target_rate_in_kbps"],
                    )
                ).keys()
            )
            # sg_names = [sg_names[-1]]
            colors = self.get_default_color_cycle()
            sg_data = []
            target_rates = []
            pos = []
            box_data = []
            for i, g in enumerate(sg_names):
                sg: SimulationGroup = self.run_map[g]
                for id, _, sim in sg.simulation_iter(enum=True):
                    _hdf = sim.get_base_provider(path="tx_pkt_bytes.h5")
                    data = _hdf.get_dataframe("tx_throughput_diff_25_0")
                    data = data.loc[:, ["m"]]
                    data["run"] = id
                    sg_data.append(data.reset_index())
                sg_data = pd.concat(
                    sg_data, axis=0, ignore_index=True, verify_integrity=False
                )
                box_data.append(
                    sg_data[sg_data["time"] >= 200].groupby("run").mean()["m"]
                )
                sg_data = (
                    sg_data[["time", "m"]]
                    .groupby("time")
                    .agg(["mean", percentile(25), percentile(75)])
                    .droplevel(0, axis=1)
                )
                self.fill_between(
                    sg_data.reset_index(),
                    line_args=dict(color=colors[i], label=kBpsLbl(sg)),
                    fill_args=dict(label="dummy"),
                    ax=ax,
                )
                sg_data = []
                target_rates.append(sg.attr["target_rate_in_kbps"] / 8)

            # w = 0.08
            # width = lambda p, w: 10**(np.log10(p)+w/2.)-10**(np.log10(p)-w/2.)
            # b = ax2.boxplot(box_data, positions=target_rates, widths=width(target_rates, w))
            # self.color_box_plot(b, fill_color="gray", flier_size=5, flier="o", ax=ax2)
            # plt.setp(
            #     b["fliers"],
            #     mec="black",
            #     # mfc=colors[i],
            #     color="gray",
            #     mfc="white",
            #     marker="o",
            #     markersize=5,
            # )

            for x, y in zip(target_rates, box_data):
                ax2.scatter(np.repeat(x, len(y)), y, marker="x", color="black")

            ax2.set_xlabel("Target transmission rate in kBps")
            ax2.set_ylabel("Actual transmission rate in kBps")
            ax2.set_xscale("log")
            ax2.set_yscale("log")
            ax2.xaxis.set_minor_formatter(self.getLogFormatter())
            ax2.yaxis.set_minor_formatter(self.getLogFormatter())
            ax2.tick_params(axis="both", which="minor", labelsize="medium")
            ax2.set_xlim(0.5, 70.0)
            ax2.set_ylim(0.5, 70.0)
            ax2.grid(visible=True, axis="both")
            ax2.grid(visible=True, which="minor", axis="both", linestyle="--")

            x = np.linspace(-5, 70, num=200)
            ax2.plot(x, x, color="red")
            ax2.set_aspect("equal", adjustable="box")
            fig2.tight_layout()
            fig2.savefig(self.run_map.path("rate-qq-scatter_log.pdf"))

            ax2.set_xscale("linear")
            ax2.set_yscale("linear")
            ax2.xaxis.set_major_locator(MultipleLocator(10))
            ax2.yaxis.set_major_locator(MultipleLocator(10))
            ax2.xaxis.set_minor_locator(MultipleLocator(5))
            ax2.yaxis.set_minor_locator(MultipleLocator(5))
            ax2.set_xlim(-1.0, 70.0)
            ax2.set_ylim(-1.0, 70.0)
            fig2.tight_layout()
            fig2.savefig(self.run_map.path("rate-qq-scatter_lin.pdf"))
            # ax2.grid(visible=False, which="minor", axis="both", linestyle="--")
            # fig2.savefig(self.run_map.path("rate-qq-box.pdf"))

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
            fig.legend(
                h, l, loc="lower center", ncol=5, fontsize="medium", frameon=False
            )
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
    def plot_target_rate_diff(self, *, ax: plt.Axes = None):
        # get ascending order of target rates
        fig1, (ax1a, ax1b) = plt.subplots(2, 1, figsize=(16, 9))
        fig2, (ax2a, ax2b) = plt.subplots(2, 1, figsize=(16, 9))

        sg_names = dict(
            sorted(self.run_map.items(), key=lambda x: x[1].attr["target_rate_in_kbps"])
        ).keys()
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        colors = self.get_default_color_cycle()
        sg_data_relative = []
        sg_data_abs = []
        stat = []
        for i, g in enumerate(sg_names):
            sg: SimulationGroup = self.run_map[g]
            for id, _, sim in sg.simulation_iter(enum=True):
                _hdf = sim.get_base_provider(path="tx_pkt_bytes.h5")
                data = _hdf.get_dataframe("tx_throughput_diff_25_0")
                data = data.loc[200:]
                relative_diff = data["diff_m"] / data["m"]
                relative_abs = data["diff_m"]
                sg_data_relative.append(relative_diff.reset_index())
                sg_data_abs.append(relative_abs.reset_index())
                self.ecdf(relative_diff, ax=ax1a, label=kBpsLbl(sg), color=colors[i])
                self.ecdf(relative_abs, ax=ax2a, label=kBpsLbl(sg), color=colors[i])
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
            _h.append(h[i * 20])
            _l.append(l[i * 20])
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
            _h.append(h[i * 20])
            _l.append(l[i * 20])
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

        print("hi")


class MemberEstPlotter(PlotUtil_):
    def __init__(self, run_map) -> None:
        super().__init__()
        self.run_map: RunMap = run_map

    def _collect_tx_throughput(self, sg: SimulationGroup, freq="20_0"):
        df = []
        sim: Simulation
        for _, id, sim in sg.simulation_iter(enum=True):
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
                cell_slice_fc=_corridor_filter_target_cells,
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

    def plot_msce(self):
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
        rc = {
            "axes.labelsize": "xx-large",
            "xtick.labelsize": "x-large",
            "ytick.labelsize": "x-large",
        }
        colors = self.get_default_color_cycle()[0:3]
        colors = [colors[-1], colors[0], colors[1]]
        with plt.rc_context(rc):
            fig, _ax = plt.subplot_mosaic("a;a;a;b;b", figsize=(8.9, 8.9))
            ax = _ax["a"]
            tbl = _ax["b"]
            b = ax.boxplot(box_data, positions=[10, 20, 30], widths=3.5)
            ax.set_xticklabels(
                ["Constant rate", "Neighbor count est.", "Map count est."]
            )
            ax.yaxis.set_major_locator(MultipleLocator(0.025))
            ax.yaxis.set_minor_locator(MultipleLocator(0.025 / 2))
            ax.set_ylim(0.2125, 0.35)
            ax.set_ylabel("Mean squared map error (MSME)")
            self.color_box_plot(b, fill_color=colors, ax=ax)
            box_data = [_df.reset_index(drop=True) for _df in box_data]
            df = pd.concat(box_data, axis=1, ignore_index=True)
            df = df.describe().iloc[1:, :]
            df = df.applymap(lambda x: f"{x:.4f}")
            df.columns = ["Constant rate", "Neighbor count est.", "Map count est."]
            df.index.name = ""
            t: plt.Table
            _, _, t = self.df_to_table(df.reset_index(), ax=tbl)
            t.set_fontsize(14)
            fig.tight_layout()
            fig.savefig(self.run_map.path("member_est_msme.pdf"))

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
                df = OppAnalysis.sg_get_txAppInterval(
                    sg,
                    module_names_f=lambda x: x.m_map(path="scheduler"),
                    interval_type="real",
                    jobs=5,
                )
                _hdf.write_frame(group=g, frame=df)
            data = self.append_bin(
                data=df[["time", "txInterval"]],
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
                        "txInterval_mean",
                        "txInterval_p25",
                        "txInterval_p75",
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
        return ax.get_figure(), ax

    @with_axis
    def plot_throughput_ts_fill(
        self, bin="25_0", *, ax: plt.Axes = None
    ) -> Tuple[plt.Figure, plt.Axes]:
        color = self.get_default_color_cycle()[0:3]
        marker = ["x", "2", "o"]
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
            color="red",
            linestyles="solid",
        )
        ax.text(
            52,
            500 / 8 + 5,
            kBpsLbl(500),
            color="red",
            fontsize="large",
            transform=ax.transData,
        )

        ax.set_ylabel("Throughput in kB/s")
        ax.set_xlabel("Simulation time in seconds")
        ax.yaxis.set_major_locator(MultipleLocator(20))
        ax.yaxis.set_minor_locator(MultipleLocator(10))
        ax.set_ylim(-5, 170)
        ax.xaxis.set_major_locator(MultipleLocator(50))
        ax.xaxis.set_minor_locator(MultipleLocator(25))
        ax.set_xlim(25, 875)
        ax.legend(loc="upper left", ncol=2)
        return fig, ax

    def plot_throughput_ped_count_ts(self, bin="25_0"):
        rc = {
            "legend.fontsize": "xx-large",
            "axes.labelsize": "xx-large",
            "xtick.labelsize": "x-large",
            "ytick.labelsize": "x-large",
        }
        with plt.rc_context(rc=rc):
            fig, axes = plt.subplot_mosaic(
                mosaic="T;T;P", figsize=(8.9 * 2, (8.9 * 2) / (16 / 9)), sharex=True
            )

            ax_t: plt.Axes = axes["T"]
            self.plot_throughput_ts_fill(bin=bin, ax=axes["T"])

            ax_p: plt.Axes = axes["P"]
            self.plot_ped_count(ax=ax_p)
            self.plot_dmap_and_nt_ts(ax=ax_p)

            # legend for lower 'P' plot.
            h, l = self.merge_legend_patches(*ax_p.get_legend_handles_labels())
            h.insert(1, pltPatch.Patch("white", "white"))
            l.insert(1, "")
            ax_p.legend().remove()
            ax_p.legend(h, l, loc="lower right", ncol=2)

            ax_p.set_xlabel("Simulation time in seconds")
            ax_p.set_ylabel("Ped. count")
            ax_p.yaxis.set_major_locator(MultipleLocator(25))
            ax_p.set_ylim(0, 175)

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
            _legend = ax_t.legend(h_new, l_new, loc="lower right", ncol=2)
            _legend.get_texts()[1].set_weight("bold")
            ax_t.set_xlabel("")

            fig.tight_layout()
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

        dmap = []
        nt = []

        _hdf = BaseHdfProvider(self.run_map.path("map_nt.h5"))
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
                _dmap = (
                    sim.get_dcdMap()
                    .map_count_measure()
                    .loc[:, ["map_mean_count"]]
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
    base_path = "/mnt/data1tb/results/arc-dsa_single_cell/"
    study_out = os.path.join(base_path, "study_out")
    os.makedirs(study_out, exist_ok=True)

    # r1 = RunMap.load_or_create(
    #     # create_f=create_target_rate_run_map,
    #     # output_path="/mnt/data1tb/results/_dyn_tx/s1_corridor_rate_cmp",
    #     create_f=create_target_rate_run_map_new,
    #     output_path=os.path.join(study_out, "s1"),
    #     load_if_present=True,
    # )
    # r1_plotter = ThroughputPlotter(r1)
    # r1_plotter.msce_over_target_rate_box()
    # prepare_throughput_cache(r1)
    # r1_plotter.plot_target_rate_diff()
    # r1_plotter.plot_target_rate_ts()

    r2 = RunMap.load_or_create(
        create_f=create_member_estimate_run_map,
        output_path=os.path.join(study_out, "s0"),
        load_if_present=True,
    )
    r2_plotter = MemberEstPlotter(r2)
    # for sg in r2.values():
    #     sg_tx_throughput_cache(sg, 25.0)
    r2_plotter.plot_throughput_ped_count_ts(bin="25_0")
    r2_plotter.plot_msce()
    r2_plotter.plot_tx_interval(freq=25.0)
    print("done")


if __name__ == "__main__":

    main()
