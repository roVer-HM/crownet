from __future__ import annotations
from itertools import chain
import os
import re
from typing import List
from roveranalyzer.analysis.common import RunMap, Simulation, SimulationGroup, SuqcStudy
from roveranalyzer.analysis.omnetpp import (
    CellOccupancy,
    CellOccupancyInfo,
    OppAnalysis,
)
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.utils import logging
from roveranalyzer.utils.dataframe import (
    format_frame,
    numeric_formatter,
    save_as_tex_table,
    siunitx,
)
from roveranalyzer.utils.plot import (
    PlotUtil,
    paper_rc,
    plt_rc_same,
    matplotlib_set_latex_param,
)

from matplotlib.patches import Patch
from s1_1d_bm import SimFactory, ts_x, ts_y
from functools import partial
from copy import deepcopy
from omnetinireader.config_parser import ObjectValue
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator, MultipleLocator, AutoMinorLocator
from pandas import IndexSlice as _i


def _remove_target_cells(df: pd.DataFrame) -> pd.DataFrame:
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


def _get_msce_with_lbl(run_map: RunMap) -> pd.DataFrame:
    data = OppAnalysis.run_get_msce_data(
        run_map,
        hdf_path="cell_mse_no_missing.h5",
        cell_count=1,  # will be ignored for entropy maps
        cell_slice_fc=_remove_target_cells,  # remove source/destination cells
        pool_size=24,
    )
    data = data.to_frame()
    data = data.join(run_map.id_to_label_series(enumerate_run=True), on="run_id")
    return data


def _filter_single_rate(sim_group: SimulationGroup, rate) -> bool:
    return sim_group.attr["gt_change_rate"] == rate


class SimF(SimFactory):
    _base_lbl = "S0"

    def __call__(self, sim: Simulation, **kwds) -> SimulationGroup:
        iat = sim.run_context.ini_get(
            "*.bonnMotionServer.traceFile", regex=r".*iat_(\d+)_.*", apply=int
        )
        if iat == 50:
            ts = ts_x
        elif iat == 25:
            ts = ts_y
        map_t = sim.run_context.ini_get(
            "*.misc[*].app[0].scheduler.generationInterval", r"^(\d+[a-z]*).*"
        )
        entropy_provider: ObjectValue = sim.run_context.ini_get(
            "*.globalDensityMap.entropyProvider"
        )
        attr = deepcopy(kwds.get("attr", {}))
        attr["gt_change_rate"] = entropy_provider["coefficients"][1]
        if map_t[-2:] == "ms":
            attr["transmission_interval_ms"] = int(map_t[0:-2])  # remove 'ms' unit
        else:
            attr["transmission_interval_ms"] = int(map_t)  # remove 'ms' unit

        attr[
            "lbl"
        ] = f"{self._base_lbl}:{self.group_num}: Map $\Delta t = {map_t}$ Change rate= {attr['gt_change_rate']}"
        attr[
            "lbl_short"
        ] = f"{self._base_lbl}:{self.group_num}-{map_t}-{attr['gt_change_rate']} "
        attr["ts"] = ts
        kwds["attr"] = attr
        ret = SimulationGroup(group_name=f"{self._base_lbl}:{self.group_num}", **kwds)
        self.group_num += 1
        return ret


def get_run_map_single_run(output_dir, data_root: str | List[str]) -> RunMap:
    if isinstance(data_root, str):
        data_root = [data_root]

    run_map = RunMap(output_dir)
    sim_factory = SimF()
    for r in data_root:
        run1 = SuqcStudy(r)
        run_map = run1.update_run_map(
            run_map,
            sim_per_group=20,
            id_offset=run_map.max_id + 1,
            sim_group_factory=sim_factory,
        )
        print(f"{len(run_map)}")
    return run_map


def plot_packet_loss_ratio_over_time(run_map: RunMap):
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label", "run_id"])
    ax = data.loc[_i[:, :, 10], ["cell_mse"]].reset_index().plot("simtime", "cell_mse")

    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    data = (
        data.join(lbl_to_rate.set_index("label"), on="label")
        .reset_index()
        .set_index(["simtime", "label", "rate"])
        .sort_index()
    )
    o = CellOccupancy.sim_create_cell_occupation_info(run_map.get_sim_by_id(10))
    # o = CellOccupancyInfo.from_hdf(run_map.path("cell_occupation.h5"))
    print("hi")


def save_msce_interval_info(run_map: RunMap):
    """Plot interval with errors. Fixed. Error due to value encoding in map packets (16 bit in 1/100)"""
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    slice_05_a = slice(1248.0, 1382.0)
    group_05 = [
        g.group_name for g in run_map.values() if g.attr["gt_change_rate"] == 0.5
    ]
    for g_name in group_05:
        g = run_map[g_name]
        print(f"create figure: {g_name}")
        for sim in g:
            _plot_msce_interval_info(sim, g.lbl, g_name, slice_05_a)

    slice_1_a = slice(622.0, 726.0)
    slice_1_b = slice(1272.0, 1282.0)
    group_1 = [
        g.group_name for g in run_map.values() if g.attr["gt_change_rate"] == 1.0
    ]
    for g_name in group_1:
        g = run_map[g_name]
        print(f"create figure: {g_name}")
        for sim in g:
            _plot_msce_interval_info(sim, g.lbl, g_name, slice_1_a)
            _plot_msce_interval_info(sim, g.lbl, g_name, slice_1_b)


def _plot_msce_interval_info(sim: Simulation, lbl, group_name, t_slice: slice):
    hdf = sim.base_hdf(group_name="cell_measures")
    with hdf.query as ctx:
        data = ctx.select(
            key=hdf.group,
            where=f"(simtime>{t_slice.start}) & (simtime<{t_slice.stop})",
            columns=["cell_mse"],
        )
    t_i = f"[{t_slice.start}, {t_slice.stop}]"
    with plt.rc_context(plt_rc_same()):
        fig, (a, b, c) = plt.subplots(nrows=3, ncols=1, figsize=(16, 2 * 9))
        fig.suptitle(f" {lbl} for interval {t_i}")
        data.hist(ax=a)
        a.set_title(f"Histogram of MSCE of interval {t_i}")
        a.set_ylabel("count")
        a.set_xlabel("MSCE")
        a.xaxis.set_major_locator(MultipleLocator(100))
        d = data.reset_index("simtime", drop=True).reset_index()
        dd = d.pivot_table(
            values="cell_mse", aggfunc="mean", index="x", columns="y", fill_value=0
        )
        dd.reset_index().plot(x="x", y=205.0, kind="scatter", ax=b)
        b.set_title(f"mean MSCE for upper corridor (y=205) in interval {t_i} ")
        b.set_ylabel("mean squared cell error (MSCE)")
        b.set_xticks(np.arange(0, 450, 20))
        dd.reset_index().plot(x="x", y=180.0, kind="scatter", ax=c)
        c.set_title(f"mean MSCE for upper corridor (y=180) in interval {t_i} ")
        c.set_ylabel("mean squared cell error (MSCE)")
        c.set_xticks(np.arange(0, 450, 20))
        fig.tight_layout(rect=(0, 0, 1, 0.98))

        file_name = (
            f"MSCE_{group_name}_interval_{int(t_slice.start)}-{int(t_slice.stop)}.pdf"
        )
        fig.savefig(os.path.join(sim.data_root, file_name))

    print("break")


def plot_msme_err_details(run_map: RunMap):
    """MSME for each seed separately"""
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    data = (
        data.join(lbl_to_rate.set_index("label"), on="label")
        .reset_index()
        .set_index(["simtime", "label", "rate"])
        .sort_index()
    )
    rates = lbl_to_rate["rate"].unique()
    ax_param = {
        0.01: [
            lambda ax, t: ax.set_ylim(0.0, 0.25)
            if t < 80000
            else ax.set_ylim(0.0, 2.00)
        ],
        0.1: [
            lambda ax, t: ax.set_ylim(0.0, 25.0)
            if t < 80000
            else ax.set_ylim(0.0, 200.0)
        ],
        0.5: [
            lambda ax, t: ax.set_ylim(0.0, 500.0)
            if t < 80000
            else ax.set_ylim(0.0, 3000.0)
        ],
        1.0: [],
        # 0.5: [lambda ax: ax.set_ylim(0.0, 250)],
        # 1.0: [lambda ax: ax.set_ylim(0.0, 1000)],
    }
    top_std = (
        data.groupby(["label", "rate", "run_id"])["cell_mse"]
        .max()
        .groupby(["label", "rate"])
        .nlargest(5)
        .droplevel([0, 1], axis=0)
        .reset_index()
        .set_index(["rate", "label"])
    )
    top_std["seed_id"] = top_std["run_id"] % 20
    top_std.to_csv(run_map.path("msme_detail_top3.csv"))

    with run_map.pdf_page("msme_detail.pdf") as pdf:
        for rate in rates[:-1]:
            df = data.loc[_i[:1200, :, rate], :]
            sg = [run_map[_l] for _l in df.index.get_level_values("label").unique()]
            sg.sort(key=lambda x: x.attr["transmission_interval_ms"])
            # top3std = df.groupby(["label", "run_id"])["cell_mse"].std().groupby("label").nlargest(3).droplevel(0, axis=0).reset_index()
            # top3std["seed_index"]  = top3std["run_id"] % 20
            with plt.rc_context(plt_rc_same(rc={"legend.fontsize": "small"})):
                fig, axes = plt.subplots(len(sg), 1, figsize=(16, 32))
                for ax, g in zip(axes, sg):
                    _df = (
                        df.loc[_i[:, g.group_name], ["run_index", "cell_mse"]]
                        .copy()
                        .reset_index()
                    )
                    ax.set_title(g.lbl)
                    ax.xaxis.set_major_locator(MaxNLocator(12))
                    for f in ax_param[rate]:
                        f(ax, g.attr["transmission_interval_ms"])
                    for run_idx, d in _df.groupby("run_index"):
                        ax.plot(d["simtime"], d["cell_mse"], label=run_idx)

                axes[-1].legend(ncol=10, loc="lower center", bbox_to_anchor=(0.5, -0.9))
                fig.suptitle(
                    f"MSME details over all runs (seed combinations) with change rate {rate}"
                )
                print("foo")
                fig.tight_layout(rect=(0.0, 0.05, 1.0, 0.99))
                pdf.savefig(fig)
                plt.close(fig)


def _plot_msme_err_detail_paper(ax_epsilon: plt.Axes, data, groups, colors):
    for (g_name, c) in list(zip(groups, colors)):
        _df = data[f"diff_{g_name}"]
        g = run_map[g_name]
        lbl = f"{g.group_name}  $\\Delta t_{{Map}} = {g.attr['transmission_interval_ms']/1000}\,s$"
        ax_epsilon.plot(_df.index.get_level_values(0), _df, label=lbl, color=c)

    ax_epsilon.set_xlabel("Simulation time in seconds")
    ax_epsilon.set_ylabel("residual MSME ratio comp. to S0:0")
    ax_epsilon.set_ylim(0, 1200)
    ax_epsilon.xaxis.set_major_locator(MultipleLocator(200))
    ax_epsilon.set_ylim(-0.2, 1.4)
    ax_epsilon.legend(
        loc="upper right",
        bbox_to_anchor=(1.0, 1.03),
        handletextpad=0.2,
        columnspacing=0.5,
    )


def plot_msme_err_detail_paper(run_map: RunMap):
    """MSME for each seed separately"""
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    data = (
        data.join(lbl_to_rate.set_index("label"), on="label")
        .reset_index()
        .set_index(["simtime", "label", "rate"])
        .sort_index()
    )
    rate = 0.01
    seed = 16
    groups = ["S0:0", "S0:12", "S0:16", "S0:20"]
    _c = sns.color_palette("colorblind", n_colors=11)
    colors = [_c[0], _c[3], _c[4], _c[5]]
    data = (
        data.loc[:1200, groups, :]
        .groupby(["simtime", "label"])["cell_mse"]
        .mean()
        .unstack("label")
    )
    for c in data.columns:
        data[f"diff_{c}"] = (data[c] - data["S0:0"]) / data["S0:0"]

    with plt.rc_context(
        paper_rc(
            tick_labelsize="large",
            rc={"legend.fontsize": "x-large", "axes.labelsize": "large"},
        )
    ):
        ax_epsilon: plt.Axes
        fig, ax_epsilon = plt.subplots(nrows=1, ncols=1, figsize=(5, 5), sharex=True)
        _plot_msme_err_detail_paper(ax_epsilon, data, groups, colors)
        fig.tight_layout()
        fig.savefig(run_map.path("s0-epsilon.pdf"))


def plot_s0(run_map: RunMap):
    pass


def _plot_msme_err_paper(
    ax_ts: plt.Axes, ax_ts2: plt.Axes, err_bars: pd.DataFrame, rate, alpha
):
    # inset zoom....
    ax_zoom = ax_ts2.inset_axes([0.4, 0.05, 0.30, 0.35])
    df = err_bars.loc[_i[:1200, :, rate]].copy().reset_index()
    _data = list(run_map.iter(lambda x: x.attr["gt_change_rate"] == rate))

    colors = sns.color_palette("colorblind", n_colors=11)
    for idx, (_, g) in enumerate(_data):
        _m = df["label"] == g.group_name
        _d = df.loc[_m]
        lbl = f"{g.group_name}  $\\Delta t_{{Map}} = {g.attr['transmission_interval_ms']/1000}\,s$"
        ax_ts.plot(
            _d["simtime"],
            _d["mean"],
            label=lbl,
            color=colors[idx],
        )
        ax_ts.fill_between(
            _d["simtime"],
            _d["mean"] - _d["std"],
            _d["mean"] + _d["std"],
            alpha=alpha,
            color=colors[idx],
        )
        if idx < 6:
            ax_ts2.plot(
                _d["simtime"],
                _d["mean"],
                label=lbl,
                color=colors[idx],
            )
            ax_zoom.plot(
                _d["simtime"],
                _d["mean"],
                label=lbl,
                color=colors[idx],
            )

    ax_ts.set_ylabel("MSME")
    ax_ts.set_ylim(0.0, 2.0)
    ax_ts.xaxis.set_major_locator(MultipleLocator(200))
    ax_ts.set_xlabel("Simtime in seconds")
    ax_ts2.set_ylabel("MSME")
    ax_ts2.set_ylim(0.0, 0.125)
    ax_ts2.xaxis.set_major_locator(MultipleLocator(200))
    ax_ts2.set_xlabel("Simtime in seconds")
    ax_ts2.hlines(
        (24.0035 * rate) ** 2,
        xmin=0.0,
        xmax=1200,
        color="red",
        label="mean mobility error",
    )
    ax_zoom.hlines(
        (24.0035 * rate) ** 2,
        xmin=0.0,
        xmax=1200,
        color="red",
        label="mean mobility error",
    )

    # sub region of the original image
    x1, x2, y1, y2 = 490, 700, 0.054, 0.059
    ax_zoom.set_xlim(x1, x2)
    ax_zoom.set_ylim(y1, y2)
    ax_zoom.set_xticklabels([])
    ax_zoom.set_yticklabels([])
    for spine in ax_zoom.spines.values():
        spine.set_edgecolor("black")

    ax_ts2.indicate_inset_zoom(ax_zoom, edgecolor="black", linewidth=2.0)

    h, l = ax_ts2.get_legend_handles_labels()
    _m = re.compile("S0:(\d+)")
    lbl = [_m.match(_lbl).groups()[0] for _lbl in l[:-1] if _m.match(_lbl)]
    _lbl = ",".join(lbl)
    ax_ts2.text(
        0.02,
        0.95,
        f"Zoomed with scenario:\nS0:{_lbl}",
        horizontalalignment="left",
        verticalalignment="top",
        fontsize="xx-large",
        transform=ax_ts2.transAxes,
    )
    return h, l


def plot_msme_err_paper(run_map: RunMap):
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    err_bars = data.groupby(["simtime", "label"])["cell_mse"].agg(["mean", "std"])
    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    err_bars = err_bars.join(lbl_to_rate.set_index("label"), on="label", how="left")
    err_bars = (
        err_bars.reset_index().set_index(["simtime", "label", "rate"]).sort_index()
    )
    rate = 0.01
    alpha = 0.15
    with plt.rc_context(
        paper_rc(rc={"legend.fontsize": "x-large", "axes.titlesize": "x-large"})
    ):
        fig, (ax_ts, ax_ts2) = plt.subplots(1, 2, figsize=(20, 6))
        h, l = _plot_msme_err_paper(ax_ts, ax_ts2, err_bars, rate, alpha)
        _h = h[-1]
        _l = l[-1]

        h, l = ax_ts.get_legend_handles_labels()
        _mid = int(np.ceil(len(h) / 2))
        _idx = [(i, i + _mid) for i in range(_mid)]
        _idx = list(chain(*_idx))
        if len(_idx) > len(h):
            del _idx[-1]
        h = list(np.array(h)[_idx])
        l = list(np.array(l)[_idx])
        for a in fig.get_axes():
            if a.get_legend() is not None:
                a.get_legend().remove()
        fig.legend(
            handles=[*h, _h],
            labels=[*l, _l],
            ncol=6,
            framealpha=0.0,
            loc="lower center",
        )
        fig.tight_layout(rect=(0.0, 0.15, 1.0, 1.0))
        fig.savefig(run_map.path("s0-msme.pdf"))


def plot_s0_summary_paper(run_map: RunMap):
    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    err_bars = data.groupby(["simtime", "label"])["cell_mse"].agg(["mean", "std"])
    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    err_bars = err_bars.join(lbl_to_rate.set_index("label"), on="label", how="left")
    err_bars = (
        err_bars.reset_index().set_index(["simtime", "label", "rate"]).sort_index()
    )
    rate = 0.01
    alpha = 0.15

    ## difff

    data_diff = _get_msce_with_lbl(run_map)
    data_diff = data_diff.reset_index().set_index(["simtime", "label"])

    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    data_diff = (
        data_diff.join(lbl_to_rate.set_index("label"), on="label")
        .reset_index()
        .set_index(["simtime", "label", "rate"])
        .sort_index()
    )
    rate = 0.01
    seed = 16
    groups = ["S0:0", "S0:12", "S0:16", "S0:20"]
    _c = sns.color_palette("colorblind", n_colors=11)
    colors = [_c[0], _c[3], _c[4], _c[5]]
    data_diff = (
        data_diff.loc[:1200, groups, :]
        .groupby(["simtime", "label"])["cell_mse"]
        .mean()
        .unstack("label")
    )
    for c in data_diff.columns:
        data_diff[f"diff_{c}"] = (data_diff[c] - data_diff["S0:0"]) / data_diff["S0:0"]
    with plt.rc_context(
        paper_rc(rc={"legend.fontsize": "x-large", "axes.titlesize": "x-large"})
    ):
        fig, axes = plt.subplots(2, 2, figsize=(20, 13))
        (ax_ts, ax_ts2, ax_ratio, ax_diff) = list(chain(*axes))
        h, l = _plot_msme_err_paper(ax_ts, ax_ts2, err_bars, rate, alpha)
        plot_cell_knowledge_ratio(run_map, ax_ratio, savefig=False)
        ax_ratio.get_legend().remove()
        ax_ratio.set_title("")
        ax_ratio.set_ylabel("Cell knowledge ratio $k_C$")
        ax_ratio.set_xlabel("Simulation time in second")
        ax_ratio.set_xlim(0, 1200)
        ax_ratio.xaxis.set_major_locator(MultipleLocator(200))
        _plot_msme_err_detail_paper(ax_diff, data_diff, groups, colors)

        _h = h[-1]
        _l = l[-1]

        h, l = ax_ts.get_legend_handles_labels()
        _mid = int(np.ceil(len(h) / 2))
        _idx = [(i, i + _mid) for i in range(_mid)]
        _idx = list(chain(*_idx))
        if len(_idx) > len(h):
            del _idx[-1]
        h = list(np.array(h)[_idx])
        l = list(np.array(l)[_idx])
        # for a in fig.get_axes():
        #     if a.get_legend() is not None:
        #         a.get_legend().remove()
        fig.legend(
            handles=[*h, _h],
            labels=[*l, _l],
            ncol=6,
            framealpha=0.0,
            loc="lower center",
        )
        fig.tight_layout(rect=(0.0, 0.12, 1.0, 1.0), h_pad=6)
        fig.savefig(run_map.path("s0-summary.pdf"))
        print("Hi")


def plot_msme_err(run_map: RunMap):
    """MSME over all seeds"""

    data = _get_msce_with_lbl(run_map)
    data = data.reset_index().set_index(["simtime", "label"])

    err_bars = data.groupby(["simtime", "label"])["cell_mse"].agg(["mean", "std"])
    lbl_to_rate = pd.DataFrame(
        [(n, g.attr["gt_change_rate"]) for n, g in run_map.items()],
        columns=["label", "rate"],
    )
    err_bars = err_bars.join(lbl_to_rate.set_index("label"), on="label", how="left")
    err_bars = (
        err_bars.reset_index().set_index(["simtime", "label", "rate"]).sort_index()
    )
    rates = err_bars.index.get_level_values("rate").unique().sort_values()
    rates = rates[:-1]

    ax_param = {
        0.01: [lambda ax: ax.set_ylim(0.0, 0.50)],
        0.1: [lambda ax: ax.set_ylim(0.0, 50.0)],
        0.5: [],
        1.0: [],
        # 0.5: [lambda ax: ax.set_ylim(0.0, 250)],
        # 1.0: [lambda ax: ax.set_ylim(0.0, 1000)],
    }

    with plt.rc_context(plt_rc_same(rc={"legend.fontsize": "small"})):
        fig, axes = plt.subplots(rates.shape[0], 1, figsize=(16, 16))

        for ax, rate in zip(axes, rates):
            df = err_bars.loc[_i[:1200, :, rate]].copy().reset_index()
            # zorder = rates[0] + 10
            x = 0
            _data = list(run_map.iter(lambda x: x.attr["gt_change_rate"] == rate))
            colors = sns.color_palette("husl", len(_data))
            for idx, (_, g) in enumerate(_data):
                _m = df["label"] == g.group_name
                _d = df.loc[_m]
                lbl = f"{g.group_name}: Map $\\Delta t = {g.attr['transmission_interval_ms']}ms$"
                ax.plot(
                    _d["simtime"],
                    _d["mean"],
                    label=lbl,
                    color=colors[idx],
                )
                ax.fill_between(
                    _d["simtime"],
                    _d["mean"] - _d["std"],
                    _d["mean"] + _d["std"],
                    alpha=0.2,
                    color=colors[idx],
                )

            ax.text(
                1.05,
                1,
                f"Mean squared map error (MSME) \n for change rate {rate} s",
                horizontalalignment="left",
                verticalalignment="top",
                fontsize="xx-large",
                transform=ax.transAxes,
            )
            ax.set_ylabel("MSME")
            for f in ax_param[rate]:
                f(ax)
            # ax.set_xticks(np.arange(0, 1600, 200))
            ax.xaxis.set_major_locator(MultipleLocator(100))
            if rate == rates[-1]:
                ax.set_xlabel("Simtime in seconds")

        fig.suptitle("Mean squared map error over time")
        fig.tight_layout(pad=1.05, rect=(0, 0, 0.98, 0.98))
        for ax in axes:
            ax.legend(loc="center left", fontsize="large", bbox_to_anchor=(1.03, 0.4))
        fig.savefig(run_map.path("msme_over_time.pdf"))
        # fig.savefig(run_map.path("msme_over_time_zoom.pdf"))


def _get_cell_occupation_ratio(run_map: RunMap) -> CellOccupancyInfo:
    return CellOccupancy.run_create_cell_occupation_info(
        run_map,
        hdf_path="cell_occupation.h5",
        interval_bin_size=100.0,
        frame_c=_remove_target_cells,
        # pool_size=1,
        pool_size=30,
    )


def plot_cell_occupation_ratio(run_map: RunMap):
    ret = _get_cell_occupation_ratio(run_map)
    CellOccupancy.plot_cell_occupation_info(ret, run_map, "cell_occupation_info.pdf")


def _plot_cell_empty_intervals_paper(
    df: CellOccupancyInfo, colors, alpha, m_seeds, ax: plt.Axes
):

    data = df.occup_interval_length.loc[_i[:, :, False, :], ["delta"]]
    _range = (data.min()[0], data.max()[0])
    _bin_count = np.ceil(data.groupby("seed").count().mean() ** 0.5)[0]
    _bins = np.histogram(data, bins=int(_bin_count))[1]
    for seed_idx, (seed, color) in enumerate(list(zip(m_seeds, colors))):
        ax.hist(
            data.loc[:, :, False, seed],
            bins=_bins,
            range=_range,
            density=True,
            label=seed_idx,
            color=color,
            alpha=alpha,
        )

    ax.hist(
        data,
        linewidth=2.5,
        bins=_bins,
        range=_range,
        density=True,
        histtype="step",
        label="over all seeds",
        color="black",
    )
    ax.set_ylabel("density")
    ax.set_xlabel("Mean empty interval length $\hat{\Delta t^E}$ in seconds")
    ax.set_ylim(0, 0.05)
    # ax.set_title("Histogram of mean empty interval length $\hat{\Delta t^E}$")

    _transparent = (1, 1, 1, 0.0)
    patches1 = [Patch(facecolor=(*c, alpha), edgecolor=(*c, alpha)) for c in colors]
    patches2 = [Patch(facecolor=_transparent, edgecolor=_transparent) for _ in colors]
    patches2[-1] = Patch(facecolor="white", edgecolor="black")
    patches = [val for pair in zip(patches1, patches2) for val in pair]
    lables = ["" for _ in patches]
    lables[-2] = "Seed 0..19"
    lables[-1] = "Over all seeds"
    ax.legend(
        handles=patches,
        labels=lables,
        ncol=len(colors),
        facecolor=_transparent,
        framealpha=0.0,
        handletextpad=0.5,
        handlelength=1.0,
        columnspacing=-0.5,
    )


def _plot_cell_occupation_ratio_paper(
    df: pd.DataFrame, colors, alpha, m_seeds, ax: plt.Axes
):

    _range = (df.occup_sim_by_cell.min()[0], df.occup_sim_by_cell.max()[0])
    _bin_count = np.ceil(df.occup_sim_by_cell.groupby("seed").count().mean() ** 0.5)[0]
    _bins = np.histogram(df.occup_sim_by_cell, bins=int(_bin_count))[1]
    for seed_idx, (seed, color) in enumerate(list(zip(m_seeds, colors))):
        ax.hist(
            df.occup_sim_by_cell.loc[:, :, seed],
            bins=_bins,
            range=_range,
            density=True,
            label=seed_idx,
            color=color,
            alpha=alpha,
        )

    ax.hist(
        df.occup_sim_by_cell,
        linewidth=2.5,
        bins=_bins,
        range=_range,
        density=True,
        histtype="step",
        label="over all seeds",
        color="black",
    )
    ax.set_ylabel("density")
    ax.set_xlabel("Mean occupation ratio $\hat{o}$")
    ax.set_ylim(0, 50.0)
    # ax.set_title("Histogram of mean cell occupancy ratio $\hat{o}$")

    _transparent = (1, 1, 1, 0.0)
    patches1 = [Patch(facecolor=(*c, alpha), edgecolor=(*c, alpha)) for c in colors]
    patches2 = [Patch(facecolor=_transparent, edgecolor=_transparent) for _ in colors]
    patches2[-1] = Patch(facecolor="white", edgecolor="black")
    patches = [val for pair in zip(patches1, patches2) for val in pair]
    lables = ["" for _ in patches]
    lables[-2] = "Seed 0..19"
    lables[-1] = "Over all seeds"
    ax.legend(
        handles=patches,
        labels=lables,
        ncol=len(colors),
        facecolor=_transparent,
        framealpha=0.0,
        handletextpad=0.5,
        handlelength=1.0,
        columnspacing=-0.5,
    )


def _plot_cell_occupation_stats(info: CellOccupancyInfo, ax: plt.Axes, tex_path=None):
    stat = info.occup_sim_by_cell.describe().set_axis(["$\hat{o_c}$"], axis=1)
    stat2 = (
        info.occup_interval_length.loc[:, "delta"]
        .groupby("cell_occupied")
        .describe()
        .T.rename(columns={True: "$\hat{\Delta t^O}$", False: "$\hat{\Delta t^E}$"})
    )
    stat = pd.concat([stat, stat2], axis=1)
    stat = format_frame(stat, col_list=stat.columns, si_func=siunitx(precision=4))
    stat.index.name = "Stat."
    stat = stat.reset_index()
    stat = stat.drop(0, axis=0)
    if ax is not None:
        ax.axis("off")
        tbl = ax.table(cellText=stat.values, colLabels=stat.columns, loc="center")
        tbl.scale(1, 2)
    if tex_path is not None:
        # todo: hack for latex math in table header
        save_as_tex_table(
            stat,
            tex_path,
            add_default_latex_format=False,
            str_replace=lambda x: x.replace(r"\textbackslash ", "\\")
            .replace(r"\textasciicircum", "_")
            .replace(r"\_", "_")
            .replace(r"\$", r"$")
            .replace(r"\{", r"{")
            .replace(r"%", r"\%")
            .replace(r"\}", r"}"),
        )


def plot_cell_occupation_ratio_paper(run_map: RunMap):
    df = _get_cell_occupation_ratio(run_map)
    m_seeds = run_map.get_mobility_seed_set()
    colors = sns.color_palette("colorblind", n_colors=len(m_seeds))
    with plt.rc_context(
        paper_rc(rc={"legend.fontsize": "medium", "axes.titlesize": "x-large"})
    ):
        fig, (ax_ratio, ax_inter) = plt.subplots(1, 2, figsize=(20 * 3 / 5, 6))
        _alpha = 0.3
        # cell occupation ratio
        _plot_cell_occupation_ratio_paper(df, colors, _alpha, m_seeds, ax_ratio)
        # empty interval distribution
        _plot_cell_empty_intervals_paper(df, colors, _alpha, m_seeds, ax_inter)
        # stats
        _plot_cell_occupation_stats(
            df, ax=None, tex_path=run_map.path("occup_stat.tex")
        )
        fig.tight_layout()
        fig.savefig(run_map.path("occup_ratio.pdf"))


def plot_occupation_intervals(run_map: RunMap):
    """Mean occupation intervals over all mobility seeds"""
    data = _get_cell_occupation_ratio(run_map)
    # get all sim ids of one (random) group
    reps = list(run_map.values())[0].ids()
    intervals = data.occup_interval_length
    # create descriptive statistics
    intervals_stats = (
        intervals.groupby("cell_occupied")
        .describe()
        .T.droplevel(0, axis=0)
        .reset_index()
        .rename(columns={True: "Occupied", False: "Empty"})
    )
    intervals_stats.columns = ["Stat.", *intervals_stats.columns[1:]]
    intervals_stats.to_csv(run_map.path(f"occupation_interval_description.csv"))
    save_as_tex_table(
        intervals_stats,
        run_map.path("occupation_interval_description.tex"),
        selected_only=False,
        col_format=numeric_formatter(precision=3),
    )

    lbl = {True: "Occupied", False: "Empty"}
    for _occup in [True, False]:
        with plt.rc_context(paper_rc()):
            fig, ax = PlotUtil.check_ax()
            ax.hist(intervals.loc[_i[:, :, _occup, :], ["delta"]], bins=100)
            ax.set_title(f"{lbl[_occup]} interval length distribution")
            ax.set_ylabel("Count")
            ax.set_xlabel("Interval length in seconds")
            fig.savefig(run_map.path(f"occupation_interval_hist_{lbl[_occup]}.pdf"))


def _get_create_cell_knowledge_ratio(run_map: RunMap):
    """Get cell knowledge ratio based on one change_rate sim group."""
    h5 = BaseHdfProvider(run_map.path("cell_knowledge_ratio.h5"))
    change_rate = [g.attr["gt_change_rate"] for g in run_map.values()][0]
    if not h5.hdf_file_exists:
        # only use one change rate because knowledge_ratio does not depend on it
        CellOccupancy.run_create_cell_knowledge_ratio(
            run_map,
            hdf_path="cell_knowledge_ratio.h5",
            sim_group_filter=partial(_filter_single_rate, rate=change_rate),
            frame_c=_remove_target_cells,
        )
    return h5, change_rate


def plot_cell_knowledge_ratio(
    run_map: RunMap, ax_k_ratio=None, ax_k_ratio_zoom=None, savefig=True
):
    h5, change_rate = _get_create_cell_knowledge_ratio(run_map)
    fig, ax_k_ratio = PlotUtil.check_ax(ax_k_ratio)
    fig2, ax_k_ratio_zoom = PlotUtil.check_ax(ax_k_ratio_zoom)

    sim_groups = [g for g in run_map.values() if _filter_single_rate(g, change_rate)]
    sim_groups.sort(key=lambda x: x.attr["transmission_interval_ms"])

    for g in sim_groups:
        with h5.query as ctx:
            data = ctx.select(
                key="knowledge_ratio", where=f"rep in  {g.ids()}", columns=["a_c"]
            )
        df = (
            data.reset_index(["rep", "m_seed"], drop=True)
            .groupby(["simtime"])["a_c"]
            .agg(["mean", "std"])
        )
        lbl = f"Map $\Delta t = {g.attr['transmission_interval_ms']}\,ms$"
        ax_k_ratio.plot(df.index, df["mean"], label=lbl)

        ax_k_ratio_zoom.plot(df.index, df["mean"], label=lbl)

    ax_k_ratio.legend()
    ax_k_ratio.set_title(f"Cell knowledge over time")
    ax_k_ratio_zoom.legend()
    ax_k_ratio_zoom.set_title(f"Cell knowledge over time (zoomed)")
    ax_k_ratio_zoom.set_ylim(0.9, 1.0)
    ax_k_ratio_zoom.set_xlim(400, 525)

    if savefig:
        fig.tight_layout()
        fig.savefig(run_map.path("cell_knowledge_ratio.pdf"))
        fig2.savefig(run_map.path("cell_knowledge_ratio_zoom.pdf"))


def create_variation_tbl(run_map: RunMap):

    records = [
        {
            "lbl": gn,
            "rate": g.attr["gt_change_rate"],
            "t_map": g.attr["transmission_interval_ms"],
        }
        for gn, g in run_map.items()
    ]
    tbl = (
        pd.DataFrame.from_records(records)
        .set_index(["rate", "t_map"])
        .sort_index()
        .unstack(["rate"])
    )
    idx = [
        f"\\SI[round-precision=2]{{{x/1000.}}}" + "{\\second}"
        for x in tbl.index.get_level_values(0)
    ]
    tbl.index = idx
    col = [
        (l0, f"\\SI[round-precision=2]{{{l1}}}" + "{\per\second}")
        for l0, l1 in tbl.columns
    ]
    tbl.columns = pd.MultiIndex.from_tuples(col, names=tbl.columns.names)
    tbl = tbl.reset_index()
    save_as_tex_table(
        tbl,
        run_map.path("S0-variation_tbl.tex"),
        add_default_latex_format=False,
        str_replace=lambda x: x.replace("index", "$\Delta t_{Map}$")
        .replace(r"{r}{lbl", r"{c}{change rate $\alpha$")
        .replace(r"\textbackslash ", "\\")
        .replace(r"\{", r"{")
        .replace(r"\}", r"}"),
    )


if __name__ == "__main__":
    # mf_1d_bm_rate_chage_1 -> s0-001  (RB25,100ms,0.01-10s,1.0)
    # mf_1d_bm_rate_change -> s0-003 (RB50,100ms,0.01-10s,1.0)
    # mf_1d_bm_rate_change_1 -> s0-004   (RB50, 20s,0.01-7500s,1.0)
    # mf_1d_bm_rate_chage -> s0-006 (RB25,100ms,0.01-10s,1.0,ONE mSeed)
    # s = SuqcStudy("/mnt/data1tb/results/mf_1d_bm_rate_change")
    # s.rename_data_root("/mnt/data1tb/results/s0-003")
    # run_map = RunMap.load_or_create(
    #     partial(get_run_map_single_run, data_root="/mnt/data1tb/results/s0-001"),
    #      "/mnt/data1tb/results/_density_map/foo/"
    # )
    matplotlib_set_latex_param()
    # logging.set_level("INFO")

    # # with mobility seed variation
    # output_dir = "/mnt/data1tb/results/_density_map/s0-001_mixedRBs/"
    output_dir = "/mnt/data1tb/results/_density_map/s0-002_25RBs/"
    run_map = RunMap.load_or_create(
        partial(
            get_run_map_single_run,
            # data_root=[# RB25/50_mixed_test
            #     "/mnt/data1tb/results/s0-001/",
            #     "/mnt/data1tb/results/s0-004/",
            # ],
            data_root=[  # RB25
                "/mnt/data1tb/results/s0-001/",
                "/mnt/data1tb/results/s0-002/",
            ],
            # data_root=[# RB50
            #     "/mnt/data1tb/results/s0-003/",
            #     "/mnt/data1tb/results/s0-004/",
            # ]
        ),
        output_dir,
    )
    plot_s0_summary_paper(run_map)
    create_variation_tbl(run_map)
    # plot_cell_knowledge_ratio(run_map)

    # plot_cell_occupation_ratio(run_map)
    # plot_msme_err_paper(run_map)
    # plot_msme_err_detail_paper(run_map)

    # plot_cell_occupation_ratio_paper(run_map)
    # plot_msme_err(run_map)
    # plot_msme_err_details(run_map)

    # # # plot_packet_loss_ratio_over_time(run_map)
    # plot_occupation_intervals(run_map)

    print("main done")
