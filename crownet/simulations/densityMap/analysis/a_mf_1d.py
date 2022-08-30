from __future__ import annotations
from copy import deepcopy
from math import floor

from itertools import combinations
import json
import os
from typing import Tuple, List
from matplotlib import pyplot as plt
import numpy as np
from roveranalyzer.analysis.common import (
    RunContext,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.analysis import adf_test
from roveranalyzer.utils.parallel import run_kwargs_map
import roveranalyzer.utils.plot as PlotUtl
import roveranalyzer.utils.dataframe as FrameUtl
from matplotlib.ticker import MaxNLocator
import pandas as pd
import seaborn as sns

sns.set(font_scale=1.1, rc={"text.usetex": True})
sns.set_style("whitegrid")

from scipy.stats import mannwhitneyu, kstest

from scipy.sparse import coo_array
import scipy.signal as sg
from matplotlib import cm
from matplotlib.colors import to_rgb

from omnetinireader.config_parser import ObjectValue


ts_x = "X_0.04"  # iat_50
ts_y = "X_0.08"  # iat_25
_c = [
    to_rgb(i)
    for i in [
        "#DD5422",
        "#4EDD22",
        "#F0E532",
        "#71472C",
        "#299BA3",
        "#999999",
        "#CA79A8",
        "#339955",
    ]
]

ts_ = {
    ts_x: {
        "lbl": r"$X_t$ with ped. arrival rate $\lambda_1 = 0.04\,\frac{ped}{s}$",
        "lbl_short": r"$X_t$",
        # "color": "red",
        "color": to_rgb("#4B72B0"),
    },
    ts_y: {
        "lbl": r"$Y_t$ with ped. arrival rate $\lambda_2 = 0.08\,\frac{ped}{s}$",
        "lbl_short": r"$Y_t$",
        # "color": "blue",
        "color": to_rgb("#C44E51"),
    },
}
_c_map = {f"S1-{i}": _c[i] for i in range(8)}
_c_map["Ground Truth"] = "black"
_c_map[ts_[ts_x]["lbl_short"]] = ts_[ts_x]["color"]
_c_map[ts_[ts_y]["lbl_short"]] = ts_[ts_y]["color"]


class Cpallet:
    def __init__(self, c) -> None:
        self.c = c

    def __getitem__(self, key):
        if isinstance(key, int):
            return list(self.c.values())[key]
        if key in self.c:
            return self.c[key]
        for k, v in self.c.items():
            if k.startswith(key):
                return v
        raise ValueError(f"key '{key}' not found.")


def read_trace_ts(path, run_id, lbl):
    df = pd.read_csv(
        path,
        sep=" ",
        skiprows=1,
        header=0,
        index_col="timeStep",
        names=["timeStep", run_id],
    )
    df.index = df.index * 0.4
    df.index.name = "time"
    df.columns = pd.MultiIndex.from_tuples(
        [(lbl, c) for c in df.columns], names=("lbl", "run_id")
    )
    return df


def process_variation(data: pd.Series, scenario_lbl: str) -> dict:
    """collect results for one run based on multiple repetitions

    Args:
        study(SuqcRun): Suqc study object (access to run definitions and output)
        scenario_lbl (str): Label for scenario
        rep_ids (list): List of runs over which to average. len(rep_ids) := N (number of seeds)

    Returns:
        dict: {
            "name": scenario name,
            "stat": pd.DataFrame (N,1) with N number of statistics containing ADF-Test, Summary Stats
            "df": pd.DataFrame (N,M) index["simtime]
        }
    """
    adf = adf_test(data)
    adf.name = "adf"
    adf = adf.to_frame()
    adf.columns = pd.Index([scenario_lbl], name="scenario")
    stat = data.describe().to_frame()  # over time
    stat.columns = pd.Index([scenario_lbl], name="scenario")
    _out = pd.concat([adf, stat], axis=0)

    print(f"done for {scenario_lbl}")
    return _out


def process_simulation_run(
    maps: pd.DataFrame, run_map: RunMap, vadere_ts: pd.DataFrame
):
    """Calculate statistic for multiple scenarios

    Args:
        study (SuqcRun): Suqc study object (access to run definitions and output
        scenario_map (dict): Map of scenarios keys: [scenario_name[rep_ids, lbl, ts]], ts: link to ground truth
        vadere_ts (pd.DataFrame): Ground truth

    Returns:
        _type_: Map and ground truth statistics for 1D study
    """

    # average raw data over seeds and calculate statistic
    ret = []
    kwds = [
        dict(
            data=maps.loc[g.group_name, :, "map_mean_count"]["mean"].copy(),
            scenario_lbl=g.group_name,
        )
        for g in run_map.values()
    ]
    for col in vadere_ts.columns:
        kwds.append(dict(data=vadere_ts[col].copy(), scenario_lbl=col))

    ret = run_kwargs_map(process_variation, kwds, pool_size=1)

    stat_df = pd.concat(ret, axis=1, verify_integrity=True).T

    return stat_df


def next_multiple(val, off=5):
    """Next hightest multiple based on 'off' e.g off=5: 5 (5->5, 6->10, 9->10, 11->15, etc.)"""
    return floor(val / off) * off + off if val % off > 0 else 0


def create_map_ts_figure(
    run_map: RunMap,
    ground_truth_ts: pd.DataFrame,
    maps: pd.DataFrame,
    ts: str,
    output_path: str | None = None,
    _ax: plt.Axes | None = None,
    lbl_key: str = "lbl",
):

    _v = ground_truth_ts.loc[:5000, [ts]]  # only first 5000 seconds
    max_y = _v[ts].max()  # get ground truth max y value for axis

    fig, ax = PlotUtl.check_ax(_ax)
    ax.plot(_v.index, _v, label=ts_[ts][lbl_key], marker=None, color=ts_[ts]["color"])
    for sim in maps.index.get_level_values("sim").unique():
        if run_map[sim].attr["ts"] == ts:
            _df: pd.DataFrame = maps.loc[
                pd.IndexSlice[sim, :, "map_mean_count"], ["mean"]
            ]
            max_y = max(max_y, _df["mean"].max())
            ax.plot(
                _df.index.get_level_values("simtime"),
                _df["mean"],
                color=_c_map[sim],
                label=run_map[sim].attr["lbl_short"],
                marker=None,
            )

            ax.set_title("Number of Pedestrians in Simulation")
            ax.set_ylabel("Number of Pedestrians")
            ax.set_xlabel("Time in [s]")
            ax.set_ylim(0, next_multiple(max_y, 5))
            ax.set_xlim(0, 5000)
            ax.xaxis.set_major_locator(MaxNLocator(10))
            ax.legend(loc="lower right")

    if output_path is not None and _ax is None:
        # only save stand alone figures
        fig.savefig(run_map.path(f"ped_count_ts_map_{ts}.pdf"))

    return fig, ax


def make_vader_ts_figure(
    data: pd.DataFrame, output_path, _ax: plt.Axes | None = None, lbl_key="lbl"
):
    """
    Plot ground truth
    """
    fig, ax = PlotUtl.check_ax(_ax)
    ax.plot(
        data.index,
        data.loc[:, [ts_x]],
        label=ts_[ts_x][lbl_key],
        marker=None,
        color=ts_[ts_x]["color"],
    )
    ax.plot(
        data.index,
        data.loc[:, [ts_y]],
        label=ts_[ts_y][lbl_key],
        marker=None,
        color=ts_[ts_y]["color"],
    )
    ax.set_title("Number of Pedestrians in Simulation")
    ax.set_ylabel("Number of Pedestrians")
    ax.set_xlabel("Time in [s]")
    ax.set_ylim(0, 35)
    ax.set_xlim(0, 5000)
    ax.vlines(500, ymin=0, ymax=35, linestyles="--", label=None, color="black")
    ax.xaxis.set_major_locator(MaxNLocator(10))
    x_lbl = [
        str(i if idx % 2 == 0 else i) for idx, i in enumerate(np.arange(0, 5001, 500))
    ]
    x_lbl[1] = "500"
    ax.set_xticklabels(x_lbl)
    ax.legend(loc="lower right")
    # save figure only in stand alone mode
    if _ax is None:
        fig.savefig(output_path)
    return fig, ax


def get_run_map_single_run(output_dir) -> RunMap:
    run_ymfd_1 = SuqcStudy("/mnt/data1tb/results/mf_1d_bm_1/")
    sim_factory = SimFactory()
    run_map = run_ymfd_1.update_run_map(
        RunMap(output_dir),
        sim_per_group=20,
        id_offset=0,
        sim_group_factory=sim_factory,
    )
    return run_map


def get_run_map_split(output_dir) -> RunMap:
    # only simulations 0-79 are correct. Others have wrong map config
    run_ymfd_1 = SuqcStudy("/mnt/data1tb/results/mf_1d_bm/")
    sim_factory = SimFactory()
    run_map = run_ymfd_1.update_run_map(
        RunMap(output_dir),
        sim_per_group=20,
        id_offset=0,
        sim_group_factory=sim_factory,
        id_filter=lambda x: x[0] < 80,
    )

    # rerun of ymf variation aka (alpha = 1.0)
    run_ymf = SuqcStudy("/mnt/data1tb/results/mf_1d_bm_2/")
    run_map = run_ymf.update_run_map(
        run_map,
        sim_per_group=20,
        id_offset=run_map.max_id + 1,
        sim_group_factory=sim_factory,
    )
    return run_map


def get_average_density_maps(
    run_map: RunMap, hdf_path: str | None = "maps.h5", hdf_key: str = "maps"
) -> pd.DateFrame:
    """Density maps (aggregated over seeds) for all simulation groups in RunMap

    Returns:
        pd.DateFrame: (sim, simtime, data)[mean, std, p_50]
    """
    if hdf_path is not None and run_map.path_exists(hdf_path):
        return pd.read_hdf(run_map.path(hdf_path), key=hdf_key)
    # get average density map from scenario map

    kwargs = [dict(sim_group=sim_group) for sim_group in run_map.values()]
    maps = run_kwargs_map(OppAnalysis.merge_maps, kwargs, pool_size=10)
    maps = pd.concat(maps, axis=0, verify_integrity=True)

    if hdf_path is not None:
        maps.to_hdf(run_map.path(hdf_path), key=hdf_key, format="table")

    return maps


def plot_default_stats(run_map: RunMap):
    maps = OppAnalysis.merge_maps_for_run_map(run_map)

    # all X_0.08
    for ped_f in ["X_0.08", "X_0.04"]:
        g1 = [g.group_name for g in run_map.values() if g.attr["ts"] == ped_f]
        g1.sort(key=lambda x: run_map.attr(x, "alg"))

        # map to long form
        g1_gt = (
            maps.loc[pd.IndexSlice[g1[0], :, ["map_glb_count"]], ["mean"]]
            .set_axis(["gt"], axis=1)
            .reset_index(["sim", "data"], drop=True)
        )
        g1_maps = (
            maps.loc[pd.IndexSlice[g1, :, ["map_mean_count"]], ["mean"]]
            .unstack("sim")
            .droplevel(0, axis=1)
            .droplevel("data", axis=0)
        )
        g1_maps = pd.concat([g1_gt, g1_maps], axis=1)

        def lbl(g_name, run_map: RunMap):
            return run_map.attr(g_name, "lbl_short")

        lbl_dict = {g: lbl(g, run_map) for g in run_map.keys()}
        lbl_dict["gt"] = "Ground Truth"

        print(f"create stat plots for {ped_f}")
        OppAnalysis.plot_descriptive_comparison(
            g1_maps,
            lbl_dict,
            run_map,
            f"{ped_f}_dist_plots.pdf",
            palette=Cpallet(_c_map),
        )


def process_1d_scenario(run_map: RunMap):
    PlotUtl.matplotlib_set_latex_param()

    trace_paths = {}
    for name, group in run_map.items():
        run_ctx: RunContext = group[0].run_context
        trace = run_ctx.ini_get(
            "*.bonnMotionServer.traceFile", r'"trace/trace_(.*)\.bonnMotion"'
        )
        trace_paths[
            group.attr["ts"]
        ] = f"../study/traces_mf_1d_bm.d/numAgents_{trace}.csv"

    # get average density map from scenario map
    maps = get_average_density_maps(run_map)
    # figure (ground truth)
    v_ts = [read_trace_ts(path, 0, lbl) for lbl, path in trace_paths.items()]
    v_ts = pd.concat(v_ts, axis=1, verify_integrity=True)
    v_ts.columns = v_ts.columns.droplevel(["run_id"])
    # use the same time frame as simulated.
    v_ts = v_ts[v_ts.index <= maps.index.get_level_values("simtime").max()]

    paper_plot(run_map, v_ts, maps)

    make_vader_ts_figure(v_ts, run_map.path("ped_time_series.pdf"))
    # figure (maps over ground truth)
    create_map_ts_figure(run_map, v_ts, maps, ts_x, output_dir)
    create_map_ts_figure(run_map, v_ts, maps, ts_y, output_dir)

    # statistics for map and ground truth data
    stat_df = process_simulation_run(maps, run_map, v_ts)
    stat_df.to_csv(run_map.path("stat.csv"))
    extract_tex_tables(stat_df, run_map)

    print("done")


def extract_tex_tables(df: pd.DataFrame | str, run_map: RunMap):

    _rename = {
        "Critical Value (1%)": "Crit. (1%)",
        "Test Statistic": "Stat.",
        "scenario": "Sim.",
        "mean": "Mean",
        "std": "Std",
    }
    _format = {
        "Stat.": "\\num{{{:.3f}}}".format,
        "p-value": "\\num{{{:.4e}}}".format,
        "Crit. (1%)": "\\num{{{:.3f}}}".format,
        "Mean": "{:.2f}".format,
        "Std": "{:.2f}".format,
    }

    if isinstance(df, str):
        df: pd.DataFrame = pd.read_csv(df)
        df = df.set_index("scenario")

    for _ts in ["X_0.08", "X_0.04"]:
        g = [n for n, g in run_map.items() if g.attr["ts"] == _ts]
        g.sort()
        g.append(_ts)
        FrameUtl.save_as_tex_table(
            df.loc[g],
            run_map.path(f"stat_{_ts}.tex"),
            rename=_rename,
            col_format=_format,
            str_replace=lambda x: x.replace("X\_0.04", "$X_t$").replace(
                "X\_0.08", "$Y_t$"
            ),
        )


class SimFactory:
    def __init__(self) -> None:
        self.group_num = 0

    def __call__(self, sim: Simulation, **kwds) -> SimulationGroup:
        iat = sim.run_context.ini_get(
            "*.bonnMotionServer.traceFile", regex=r".*iat_(\d+)_.*", apply=int
        )
        if iat == 50:
            ts = ts_x
        elif iat == 25:
            ts = ts_y
        beacon_t = sim.run_context.ini_get(
            "*.misc[*].app[0].scheduler.generationInterval", r"^(\d+[a-z]*).*"
        )
        map_t = sim.run_context.ini_get(
            "*.misc[*].app[1].scheduler.generationInterval", r"^(\d+[a-z]*).*"
        )
        alg: ObjectValue = sim.run_context.ini_get("*.misc[*].app[1].app.mapCfg")
        attr = deepcopy(kwds.get("attr", {}))
        attr["alg"] = "ymf" if float(alg["alpha"]) == 1.0 else "yDist"
        alg = attr["alg"]
        attr[
            "lbl"
        ] = f"S1-{self.group_num} ({alg}): Beacon $\\vert$ Map $\Delta t = {beacon_t}\\vert {map_t}$"
        attr["lbl_short"] = f"S1-{self.group_num} ({alg})"
        attr["ts"] = ts
        kwds["attr"] = attr
        ret = SimulationGroup(group_name=f"S1-{self.group_num}", **kwds)
        self.group_num += 1
        return ret


def paper_plot(run_map: RunMap, gt_ts: pd.DataFrame, maps: pd.DataFrame):

    fig, (ax_ts, sim_ts, err_cdf) = plt.subplots(1, 3, figsize=(20, 6))

    global ts_x
    global ts_y

    make_vader_ts_figure(gt_ts, "no_save", _ax=ax_ts, lbl_key="lbl_short")
    create_map_ts_figure(
        run_map, gt_ts, maps, ts_x, "no_save", sim_ts, lbl_key="lbl_short"
    )
    create_map_ts_figure(
        run_map, gt_ts, maps, ts_y, "no_save", sim_ts, lbl_key="lbl_short"
    )

    _ts_y = [gn for gn, g in run_map.items() if g.attr["ts"] == "X_0.08"]
    _ts_x = [gn for gn, g in run_map.items() if g.attr["ts"] == "X_0.04"]
    _df = []
    data = (
        maps.loc[pd.IndexSlice[:, :, "map_mean_count"], ["mean"]]
        .unstack(["data", "sim"])
        .droplevel([0, 1], axis=1)
    )
    data_glb = (
        maps.loc[pd.IndexSlice[:, :, "map_glb_count"], ["mean"]]
        .unstack(["data", "sim"])
        .droplevel([0, 1], axis=1)
    )
    data = pd.concat(
        [data_glb.loc[:, [_ts_y[0]]].set_axis([ts_[ts_y]["lbl_short"]], axis=1), data],
        axis=1,
    )
    data = pd.concat(
        [data_glb.loc[:, [_ts_x[0]]].set_axis([ts_[ts_x]["lbl_short"]], axis=1), data],
        axis=1,
    )

    _ts_y.insert(0, ts_[ts_y]["lbl_short"])
    _ts_x.insert(0, ts_[ts_x]["lbl_short"])
    col_order = [*_ts_y, *_ts_x]

    sns.ecdfplot(data[col_order], palette=_c_map, ax=err_cdf, legend=False)

    ax_ts.set_title("Time Series Ground Truth")
    err_cdf.set_title("Empirical Cumulative Density Function")
    err_cdf.set_xlabel("Number of Pedestrian")

    _h, _l = sim_ts.get_legend_handles_labels()
    ax_ts.get_legend().remove()
    sim_ts.get_legend().remove()
    fig.legend(
        np.array(_h).reshape(2, 5).T.reshape((1, 10))[0],
        np.array(_l).reshape(2, 5).T.reshape((1, 10))[0],
        loc="lower center",
        ncol=5,
        fontsize="x-small",
    )
    fig.tight_layout(rect=(0.0, 0.07, 1.0, 1.0))

    fig.savefig(run_map.path("1d_plot_descriptive.pdf"))


def conv_err():
    PlotUtl.matplotlib_set_latex_param()
    out_path = "/mnt/data1tb/results/mf_1d_8/"
    run = SuqcStudy(out_path)

    factory = SimFactory()
    run_map = run.update_run_map(
        RunMap(out_path), sim_per_group=20, id_offset=0, sim_group_factory=factory
    )
    virdis = cm.get_cmap("viridis", 256)

    sim: Simulation = run.get_sim(0)
    map = sim.get_dcdMap()
    df = map._count_p[pd.IndexSlice[2000, :, :, 8239], :]
    d = df.reset_index()
    m = map.metadata
    d["x"] /= m.cell_size
    d["y"] /= m.cell_size
    d["err"] = np.abs(d["err"])
    spa = coo_array((d["err"], (d["x"], d["y"])), shape=m.cell_count)
    spa = spa.transpose()
    kernel = np.full((3, 3), fill_value=1 / 9)
    spa_conv = sg.convolve2d(spa.toarray(), kernel, mode="same")
    fig, ax = plt.subplots(1, 2, figsize=(16, 9))
    p = ax[0].pcolormesh(spa.toarray(), cmap=virdis, vmin=-1, vmax=1)
    fig.colorbar(p, ax=ax[0])
    ax[0].set_aspect("equal", adjustable="box")
    ax[0].set_title("Error in 1D-Scenario")
    p = ax[1].pcolormesh(spa_conv, cmap=virdis, vmin=-1, vmax=1)
    fig.colorbar(p, ax=ax[1])
    ax[1].set_title("Error in 1D-Scenario with 3x3(1/9) kernel")
    ax[1].set_aspect("equal", adjustable="box")
    fig.show()

    print("np.power(spa.toarray(), 2).sum()/(84*79)")
    print(np.power(spa.toarray(), 2).sum() / (84 * 79))

    print("np.power(spa_conv, 2).sum()/(84*79)")
    print(np.power(spa_conv, 2).sum() / (84 * 79))
    print("")


if __name__ == "__main__":

    # output_dir = "/mnt/data1tb/results/_density_map/01_1d_output/"
    # run_map = RunMap.load_or_create(get_run_map_split, output_dir)

    output_dir = "/mnt/data1tb/results/_density_map/01a_1d_output/"
    run_map = RunMap.load_or_create(get_run_map_single_run, output_dir)
    plot_default_stats(run_map)
    # process_1d_scenario(run_map)
    extract_tex_tables(run_map.path("stat.csv"), run_map)
