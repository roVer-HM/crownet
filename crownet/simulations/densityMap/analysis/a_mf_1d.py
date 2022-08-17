from __future__ import annotations
from copy import deepcopy
from math import floor

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
from roveranalyzer.utils.plot import matplotlib_set_latex_param
from matplotlib.ticker import MaxNLocator
import pandas as pd

from scipy.sparse import coo_array
import scipy.signal as sg
from matplotlib import cm


ts_x = "X_0.04"  # iat_50
ts_y = "X_0.08"  # iat_25

ts_ = {
    ts_x: {
        "lbl": r"$X_t$ with ped. arrival rate $\lambda_1 = 0.04\,\frac{ped}{s}$",
        "color": "red",
    },
    ts_y: {
        "lbl": r"$Y_t$ with ped. arrival rate $\lambda_2 = 0.08\,\frac{ped}{s}$",
        "color": "blue",
    },
}


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
):

    _v = ground_truth_ts.loc[:5000, [ts]]  # only first 5000 seconds
    max_y = _v[ts].max()  # get ground truth max y value for axis

    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    ax.plot(_v.index, _v, label=ts_[ts]["lbl"], marker=None, color="black")
    for sim in maps.index.get_level_values("sim").unique():
        if run_map[sim].attr["ts"] == ts:
            _df: pd.DataFrame = maps.loc[
                pd.IndexSlice[sim, :, "map_mean_count"], ["mean"]
            ]
            max_y = max(max_y, _df["mean"].max())
            ax.plot(
                _df.index.get_level_values("simtime"),
                _df["mean"],
                label=run_map[sim].attr["lbl"],
                marker=None,
            )

            ax.set_title("Number of Pedestrians in Simulation")
            ax.set_ylabel("Number of Pedestrians")
            ax.set_xlabel("Time in [s]")
            ax.set_ylim(0, next_multiple(max_y, 5))
            ax.set_xlim(0, 5000)
            ax.xaxis.set_major_locator(MaxNLocator(10))
            ax.legend(loc="lower right")

    if output_path is not None:
        fig.savefig(run_map.path(f"ped_count_ts_map_{ts}.pdf"))

    return fig, ax


def make_vader_ts_figure(data: pd.DataFrame, output_path):
    """
    Plot ground truth
    """
    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    ax.plot(
        data.index,
        data.loc[:, [ts_x]],
        label=ts_[ts_x]["lbl"],
        marker=None,
        color=ts_[ts_x]["color"],
    )
    ax.plot(
        data.index,
        data.loc[:, [ts_y]],
        label=ts_[ts_y]["lbl"],
        marker=None,
        color=ts_[ts_y]["color"],
    )
    ax.set_title("Number of Pedestrians in Simulation")
    ax.set_ylabel("Number of Pedestrians")
    ax.set_xlabel("Time in [s]")
    ax.set_ylim(0, 35)
    ax.set_xlim(0, 10000)
    ax.vlines(500, ymin=0, ymax=35, linestyles="--", label=None, color="black")
    ax.xaxis.set_major_locator(MaxNLocator(20))
    x_lbl = [
        str(i if idx % 2 == 0 else "") for idx, i in enumerate(np.arange(0, 10001, 500))
    ]
    x_lbl[1] = "500"
    ax.set_xticklabels(x_lbl)
    ax.legend(loc="lower right")
    if output_path is not None:
        fig.savefig(os.path.join(output_path, "ped_time_series.pdf"))
    return fig, ax


def get_run_map(output_dir, load_if_present=True) -> RunMap:
    if load_if_present and os.path.exists(os.path.join(output_dir, "run_map.json")):
        return RunMap.load_from_json(os.path.join(output_dir, "run_map.json"))

    # only simulations 0-79 are correct. Others have wrong map config
    run_ymfd_1 = SuqcStudy("/mnt/data1tb/results/mf_1d_bm/")
    sim_factory = SimFactory()
    run_map = run_ymfd_1.update_run_map(
        RunMap(output_dir),
        sim_per_group=20,
        id_offset=0,
        sim_group_factory=sim_factory,
        id_filter=lambda x: x[0] < 80,
        attr=dict(alg="yDist"),
    )

    # rerun of ymf variation aka (alpha = 1.0)
    run_ymf = SuqcStudy("/mnt/data1tb/results/mf_1d_bm_2/")
    run_map = run_ymf.update_run_map(
        run_map,
        sim_per_group=20,
        id_offset=run_map.max_id + 1,
        sim_group_factory=sim_factory,
        attr=dict(alg="ymf"),
    )

    run_map.save_json()
    return run_map


def get_average_density_maps(
    run_map: RunMap, hdf_path: str | None = "maps.h5", hdf_key: str = "maps"
) -> pd.DateFrame:
    if hdf_path is not None and run_map.path_exists(hdf_path):
        return pd.read_hdf(run_map.path(hdf_path), key=hdf_key)
    # get average density map from scenario map

    kwargs = [dict(sim_group=sim_group) for sim_group in run_map.values()]
    maps = run_kwargs_map(OppAnalysis.merge_maps, kwargs, pool_size=10)
    maps = pd.concat(maps, axis=0, verify_integrity=True)

    if hdf_path is not None:
        maps.to_hdf(run_map.path(hdf_path), key=hdf_key, format="table")

    return maps


def process_1d_scenario():
    matplotlib_set_latex_param()
    output_dir = "/mnt/data1tb/results/_density_map/01_1d_output/"
    run_map = get_run_map(output_dir)

    trace_paths = {}
    for name, group in run_map.items():
        run_ctx: RunContext = group[0].run_context
        trace = run_ctx.ini_get(
            "*.bonnMotionServer.traceFile", r'"trace/trace_(.*)\.bonnMotion"'
        )
        trace_paths[
            group.attr["ts"]
        ] = f"../study/traces_mf_1d_bm.d/numAgents_{trace}.csv"

    print("d")

    # figure (ground truth)
    v_ts = [read_trace_ts(path, 0, lbl) for lbl, path in trace_paths.items()]
    v_ts = pd.concat(v_ts, axis=1, verify_integrity=True)
    v_ts.columns = v_ts.columns.droplevel(["run_id"])

    # get average density map from scenario map
    maps = get_average_density_maps(run_map)

    # make_vader_ts_figure(v_ts, out_path)
    # figure (maps over ground truth)
    create_map_ts_figure(run_map, v_ts, maps, ts_x, output_dir)
    create_map_ts_figure(run_map, v_ts, maps, ts_y, output_dir)

    # statistics for map and ground truth data
    stat_df = process_simulation_run(maps, run_map, v_ts)
    stat_df.to_csv(run_map.path("stat.csv"))
    print("done")


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
        attr = deepcopy(kwds.get("attr", {}))
        alg = attr.get("alg", "")
        attr[
            "lbl"
        ] = f"1d\\_{self.group_num}: Beacon $\\vert$ Map $\Delta t = {beacon_t}\\vert {map_t} {alg}$"
        attr["ts"] = ts
        kwds["attr"] = attr
        ret = SimulationGroup(group_name=f"1d_{self.group_num}", **kwds)
        self.group_num += 1
        return ret


def conv_err():
    matplotlib_set_latex_param()
    out_path = "/mnt/data1tb/results/mf_1d_8/"
    run = SuqcStudy(out_path)

    factory = SimFactory()
    run_map = run.update_run_map(
        RunMap(out_path), sim_per_group=20, id_offset=0, sim_group_factory=factory
    )
    print("d")
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
    process_1d_scenario()
    # conv_err()
    # read_vadere_ts(
    #     "../study/traces_mf_1d_bm.d/omnetSeedManager.json",
    #     [
    #     ("../study/traces_mf_1d_bm.d/trace_list_mf_1d_m_const_2x5m_d20m_iat_50.json", ts_x),
    #     ("../study/traces_mf_1d_bm.d/trace_list_mf_1d_m_const_2x5m_d20m_iat_25.json", ts_y),
    #     ]
    # )
