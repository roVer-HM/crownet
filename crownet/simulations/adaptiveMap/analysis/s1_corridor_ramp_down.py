import os
import glob
import re
from typing import List, Tuple
from matplotlib.ticker import MultipleLocator, FixedLocator, FixedFormatter
from roveranalyzer.analysis.common import (
    RunContext,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from roveranalyzer.simulators.crownet.common.dcd_metadata import DcdMetaData
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.simulators.vadere.plots.scenario import VaderScenarioPlotHelper
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.analysis.omnetpp import HdfExtractor, OppAnalysis
from roveranalyzer.analysis.plot.app_misc import PlotAppMisc, PlotAppTxInterval
from roveranalyzer.utils.plot import FigureSaverSimple, PlotUtil_, percentile, with_axis
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as pltPatch
import matplotlib.lines as pltLines
from matplotlib.collections import PatchCollection

from roveranalyzer.utils.styles import (
    load_matplotlib_style,
    STYLE_SIMPLE_169,
    STYLE_TEX,
)

load_matplotlib_style(STYLE_TEX)


def _corridor_filter_target_cells(df: pd.DataFrame) -> pd.DataFrame:
    # remove cells under target area
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
            meta["lbl"] = "Map est."
        elif "table" in meta["member_estimate"].lower():
            meta["lbl"] = "NT est."
        else:
            meta["lbl"] = "Const. rate"
        sg = SimulationGroup(
            data=data,
            group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
            attr=meta,
        )
        return sg

    run_map: RunMap = RunMap(output_path)
    root_target_rate_500 = "/mnt/data1tb/results/s1_corridor_ramp_down/"
    _study: SuqcStudy = SuqcStudy(root_target_rate_500)
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
        meta["lbl"] = meta["map_target_rate"]
        sg = SimulationGroup(
            data=data,
            group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
            attr=meta,
        )
        return sg

    run_map: RunMap = RunMap(output_path)
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
    return run_map


def _read_target_rate_500_kbps():
    root_dir = "/mnt/data1tb/results/s1_corridor_ramp_down/"
    # 0-20 nTable
    # *.misc[*].app[*].scheduler.neighborhoodSizeProvider = "^.^.nTable"
    study = SuqcStudy(root_dir)
    nTable_f = lambda x: x[0] < 20
    ret = study.get_failed_missing_runs(nTable_f)
    if len(ret) > 0:
        raise ValueError(f"found erroneous sim: {ret}")
    sims: List[Simulation] = [
        study.get_run_as_sim(key) for key, _ in study.get_run_items(nTable_f)
    ]


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


class TpPlotter(PlotUtil_):
    def __init__(self, run_map) -> None:
        super().__init__()
        self.run_map: RunMap = run_map

    @with_axis
    def msce_over_target_rate_box(self, *, ax: plt.Axes = None):

        scenario = VaderScenarioPlotHelper(
            "../study/corridor_trace_5perSpawn_ramp_down.d/corridor_2x5m_d20_5perSpawn_ramp_down.out/BASIS_corridor_2x5m_d20_5perSpawn_ramp_down.out.scenario"
        )
        # scenario = VaderScenarioPlotHelper("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
        # same top for all. Just use first simulation
        m: DcdMetaData = self.run_map[0].simulations[0].get_dcdMap().metadata
        _free, _covered = scenario.get_legal_cells(m.create_full_index_slice())

        fig: plt.Figure = ax.get_figure()
        sg: SimulationGroup
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        for i, (g, sg) in enumerate(self.run_map.items()):
            m.create_full_index
            if _hdf.contains_group(g):
                df = _hdf.get_dataframe(g)
            else:
                df = OppAnalysis.sg_get_msce_data(
                    sim_group=sg,
                    cell_count=len(_free),
                    cell_slice_fc=_corridor_filter_target_cells,
                )
                _hdf.write_frame(group=g, frame=df)
            df_m = df.groupby("run_id").agg(
                ["mean", "std", percentile(25), percentile(75)]
            )
            df_m = df_m.droplevel(0, axis=1)
            df_m["tp"] = int(sg.attr["map_target_rate"].replace("kbps", ""))
            ax.scatter(df_m["tp"], df_m["mean"], marker="x", color="black")

        ax.set_xlim(25, 525)
        ax.xaxis.set_major_locator(MultipleLocator(50))
        ax.xaxis.set_minor_locator(MultipleLocator(25))
        ax.set_xlabel("Target transmission rate")
        ax.set_ylabel("Mean squared cell error (MSCE)")

        fig.tight_layout()
        fig.savefig(self.run_map.path("msce_over_target_rate_by_seed.pdf"))


class MemberEstPlotter(PlotUtil_):
    def __init__(self, run_map) -> None:
        super().__init__()
        self.run_map: RunMap = run_map

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

    def _collect_tx_throughput(self, sg: SimulationGroup, freq="20_0"):
        df = []
        sim: Simulation
        for _, id, sim in sg.simulation_iter(enum=True):
            _df = sim.from_hdf("tk_pkt_bytes.h5", f"tx_throughput{freq}")
            _df["run"] = id
            df.append(_df)
        return pd.concat(df, axis=0, verify_integrity=False).sort_index()

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
                    sg, app_type="map", interval_type="real", jobs=5
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
    def throughput_ts_fill_plot(
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
            linestyles="dashed",
            label="Target rate",
        )

        ax.set_ylabel("Throughput in kB/s")
        ax.set_xlabel("Simulation time in seconds")
        ax.yaxis.set_major_locator(MultipleLocator(10))
        ax.yaxis.set_minor_locator(MultipleLocator(5))
        ax.set_ylim(-5, 170)
        ax.xaxis.set_major_locator(MultipleLocator(50))
        ax.xaxis.set_minor_locator(MultipleLocator(25))
        ax.set_xlim(25, 875)
        ax.legend(loc="upper left")
        return fig, ax

    def throughput_ped_count_ts(self, bin="25_0"):
        fig, axes = plt.subplot_mosaic(mosaic="T;T;T;P", figsize=(16, 9), sharex=True)

        ax_t: plt.Axes = axes["T"]
        self.throughput_ts_fill_plot(bin=bin, ax=axes["T"])
        ax_p: plt.Axes = axes["P"]
        self.plot_ped_count(ax=axes["P"])
        ax_tx_interval: plt.Axes = ax_t.twinx()
        self.plot_tx_interval(freq=float(bin.replace("_", ".")), ax=ax_tx_interval)
        ax_tx_interval.set_ylim(-0.5, 17)
        ax_tx_interval.yaxis.set_major_locator(MultipleLocator(1))
        ax_tx_interval.yaxis.set_minor_locator(MultipleLocator(0.5))
        ax_tx_interval.yaxis.set_major_formatter(lambda x, pos: x if x < 7 else "")
        ax_tx_interval.set_ylabel("Tx interval in seconds")
        ax_tx_interval.grid(False)

        ax_p.set_xlabel("Simulation time in seconds")
        ax_p.set_ylabel("Ped. count")
        ax_p.yaxis.set_major_locator(MultipleLocator(50))
        ax_p.yaxis.set_minor_locator(MultipleLocator(25))
        ax_p.set_ylim(0, 175)

        h, l = ax_t.get_legend_handles_labels()
        l = [l_.replace("Const. rate", "Const. rate*") for l_ in l]
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

        target_rate_index = l_new.index("Target rate")
        for _list in [h_new, l_new]:
            v = _list[target_rate_index]
            del _list[target_rate_index]
            _list.insert(0, v)
        l_new = [_l.replace("Mean tx-rate: ", "") for _l in l_new]
        h_new.insert(1, pltPatch.Patch(edgecolor="white", facecolor="white"))
        l_new.insert(1, "Mean tx-rate:")

        ax_t.get_legend().remove()
        _legend = ax_t.legend(h_new, l_new, loc="upper left")
        _legend.get_texts()[1].set_weight("bold")
        ax_t.set_xlabel("")
        fig.tight_layout()
        fig.savefig(self.run_map.path(f"throughput_pedcount_ts_{bin}.pdf"))

    @with_axis
    def throughput_ts_box_plot(self, *, ax: plt.Axes = None):
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
        # ax.set_ylim(0, 120)
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


if __name__ == "__main__":
    # _read_target_rate_500_kbps()
    # create_target_rate_run_map("/mnt/data1tb/results/_dyn_tx/s1_crorridor_001")
    # r1 = RunMap.load_or_create(
    #     create_f=create_target_rate_run_map,
    #     output_path="/mnt/data1tb/results/_dyn_tx/s1_corridor_rate_cmp",
    #     # load_if_present=False
    # )
    # r1_plotter = TpPlotter(r1)
    # r1_plotter.msce_over_target_rate_box()
    r2 = RunMap.load_or_create(
        create_f=create_member_estimate_run_map,
        output_path="/mnt/data1tb/results/_dyn_tx/s1_corridor_member_estimate_cmp",
        # load_if_present=False
    )
    r2_plotter = MemberEstPlotter(r2)
    # for sg in r2.values():
    #     sg_tx_throughput_cache(sg, 1.0)
    r2_plotter.throughput_ped_count_ts(bin="25_0")
    r2_plotter.throughput_ped_count_ts(bin="1_0")
    print("done")
