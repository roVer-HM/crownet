from __future__ import annotations
import os
from typing import List
from roveranalyzer.analysis.common import RunMap, Simulation, SimulationGroup, SuqcStudy
from roveranalyzer.analysis.omnetpp import (
    CellOccupancy,
    OppAnalysis,
)
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.utils import logging
from roveranalyzer.utils.plot import (
    PlotUtil,
    check_ax,
    mult_locator,
    plt_rc_same,
    matplotlib_set_latex_param,
)
from a_mf_1d import SimFactory, ts_x, ts_y
from functools import partial
from copy import deepcopy
from omnetinireader.config_parser import ObjectValue
import pandas as pd
import numpy as np
import seaborn as sn
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
        attr["transmission_interval_ms"] = int(map_t[0:-2])  # remove 'ms' unit

        attr[
            "lbl"
        ] = f"S4-{self.group_num}: Map $\Delta t = {map_t}$ Change rate= {attr['gt_change_rate']}"
        attr["lbl_short"] = f"S1-{self.group_num}-{map_t}-{attr['gt_change_rate']} "
        attr["ts"] = ts
        kwds["attr"] = attr
        ret = SimulationGroup(group_name=f"S4-{self.group_num}", **kwds)
        self.group_num += 1
        return ret


def get_run_map_single_run(output_dir, data_root: str | List[str]) -> RunMap:
    if isinstance(data_root, str):
        data_root = [data_root]

    run_map = RunMap(output_dir)
    for r in data_root:
        run1 = SuqcStudy(r)
        sim_factory = SimF()
        run_map = run1.update_run_map(
            RunMap(output_dir),
            sim_per_group=20,
            id_offset=run_map.max_id + 1,
            sim_group_factory=sim_factory,
        )
    return run_map


def plot_packet_loss_ratio_over_time(run_map: RunMap):
    # loss = OppAnalysis.run_get_packet_loss(run_map)
    # loss = pd.read_hdf(run_map.path("loss2.h5"))
    # fig, ax = check_ax()
    # sn.lineplot(data=loss.reset_index(), x="time", y="lost_relative", hue="rep")
    # ax.set_title("Relative packet loss over time for each simulation")
    # fig.savefig(run_map.path("packet_loss_time_series.pdf"))

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
        a.xaxis.set_major_locator(MaxNLocator(10))
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
        0.01: [lambda ax: ax.set_ylim(0.0, 0.40)],
        0.1: [lambda ax: ax.set_ylim(0.0, 40.0)],
        0.5: [],
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
        for rate in rates:
            df = data.loc[_i[:, :, rate], :]
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
                    for f in ax_param[rate]:
                        f(ax)
                    for run_idx, d in _df.groupby("run_index"):
                        ax.plot(d["simtime"], d["cell_mse"], label=run_idx)

                axes[-1].legend(ncol=7, loc="lower center", bbox_to_anchor=(0.5, -0.7))
                fig.suptitle(
                    f"MSME details over all runs (seed combinations) with change rate {rate}"
                )
                print("foo")
                fig.tight_layout(rect=(0.0, 0.05, 1.0, 0.99))
                pdf.savefig(fig)
                plt.close(fig)


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

    ax_param = {
        0.01: [lambda ax: ax.set_ylim(0.0, 0.20)],
        0.1: [lambda ax: ax.set_ylim(0.0, 20.0)],
        0.5: [],
        1.0: [],
        # 0.5: [lambda ax: ax.set_ylim(0.0, 250)],
        # 1.0: [lambda ax: ax.set_ylim(0.0, 1000)],
    }

    with plt.rc_context(plt_rc_same(rc={"legend.fontsize": "small"})):
        fig, axes = plt.subplots(rates.shape[0], 1, figsize=(16, 16))

        for ax, rate in zip(axes, rates):
            df = err_bars.loc[_i[:, :, rate]].copy().reset_index()
            # zorder = rates[0] + 10
            x = 0
            for idx, (_, g) in enumerate(
                run_map.iter(lambda x: x.attr["gt_change_rate"] == rate)
            ):
                _m = df["label"] == g.group_name
                _d = df.loc[_m]
                lbl = f"{g.group_name}: Map $\\Delta t = {g.attr['transmission_interval_ms']}ms$"
                ax.plot(
                    _d["simtime"],
                    _d["mean"],
                    label=lbl,
                    color=PlotUtil.plot_colors[idx],
                )
                ax.fill_between(
                    _d["simtime"],
                    _d["mean"] - _d["std"],
                    _d["mean"] + _d["std"],
                    alpha=0.2,
                    color=PlotUtil.plot_colors[idx],
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
            ax.set_xticks(np.arange(0, 1600, 200))
            if rate == rates[-1]:
                ax.set_xlabel("Simtime in seconds")

        fig.suptitle("Mean squared map error over time")
        fig.tight_layout(pad=1.05, rect=(0, 0, 0.98, 0.98))
        for ax in axes:
            ax.legend(loc="center left", fontsize="large", bbox_to_anchor=(1.03, 0.4))
        fig.savefig(run_map.path("msme_over_time.pdf"))
        # fig.savefig(run_map.path("msme_over_time_zoom.pdf"))


def _get_cell_occupation_ratio(run_map: RunMap, same_mobility_seed: bool = False):
    return CellOccupancy.run_create_cell_occupation_info(
        run_map,
        hdf_path="cell_occupation.h5",
        interval_bin_size=100.0,
        same_mobility_seed=same_mobility_seed,
        frame_c=_remove_target_cells,
        pool_size=30,
    )


def plot_cell_occupation_ratio(run_map: RunMap, same_mobility_seed: bool = False):
    ret = _get_cell_occupation_ratio(run_map, same_mobility_seed)
    CellOccupancy.plot_cell_occupation_info(ret, run_map, "cell_occupation_info.pdf")


def plot_occupation_intervals(run_map: RunMap):
    """Mean occupation intervals over all mobility seeds"""
    data = _get_cell_occupation_ratio(run_map)
    # todo


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


def plot_cell_knowledge_ratio(run_map: RunMap):
    h5, change_rate = _get_create_cell_knowledge_ratio(run_map)
    fig, ax_k_ratio = check_ax()
    fig2, ax_k_ratio_zoom = check_ax()

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
        # fill_between(ax_k_ratio, data=df, label=lbl)
        ax_k_ratio.plot(df.index, df["mean"], label=lbl)

        # fill_between(ax_k_ratio_zoom, data=df, label=lbl )
        ax_k_ratio_zoom.plot(df.index, df["mean"], label=lbl)

    ax_k_ratio.legend()
    ax_k_ratio.set_title(f"Cell knowledge over time")
    ax_k_ratio_zoom.legend()
    ax_k_ratio_zoom.set_title(f"Cell knowledge over time (zoomed)")
    ax_k_ratio_zoom.set_ylim(0.9, 1.0)
    ax_k_ratio_zoom.set_xlim(400, 525)

    fig.tight_layout()
    fig.savefig(run_map.path("cell_knowledge_ratio.pdf"))
    fig2.savefig(run_map.path("cell_knowledge_ratio_zoom.pdf"))


if __name__ == "__main__":
    matplotlib_set_latex_param()
    logging.set_level("INFO")

    # with mobility seed variation
    # output_dir = "/mnt/data1tb/results/_density_map/04d_rate_output/"
    output_dir = "/mnt/data1tb/results/_density_map/04b_rate_output/"
    run_map = RunMap.load_or_create(
        partial(
            get_run_map_single_run,
            # data_root="/mnt/data1tb/results/mf_1d_bm_rate_change/",
            data_root="/mnt/data1tb/results/mf_1d_bm_rate_chage_1/",
        ),
        output_dir,
    )
    plot_cell_knowledge_ratio(run_map)
    plot_cell_occupation_ratio(run_map, same_mobility_seed=False)
    plot_msme_err(run_map)
    plot_msme_err_details(run_map)
    plot_packet_loss_ratio_over_time(run_map)
    plot_occupation_intervals(run_map)

    # save_msce_interval_info(run_map)
    print("main done")
