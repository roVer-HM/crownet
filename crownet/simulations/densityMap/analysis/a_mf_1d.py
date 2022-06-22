from __future__ import annotations
from math import floor

from multiprocessing import get_context

import os
from matplotlib import pyplot as plt
import numpy as np
from roveranalyzer.analysis.common import Simulation, SuqcRun
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.analysis import adf_test
from roveranalyzer.analysis.ts_analysis import adf_summary_test
from roveranalyzer.utils.plot import matplotlib_set_latex_param
from matplotlib.ticker import MaxNLocator
import pandas as pd

from scipy.sparse import coo_array
import scipy.signal as sg
from matplotlib import cm


ts_x = "X_0.04"
ts_y = "X_0.08"

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


def read_vadere_ts(ts_x_path, ts_y_path):
    """read ground truth """
    data = pd.concat(
        [
            pd.read_csv(
                ts_x_path,
                sep=" ",
                skiprows=1,
                header=0,
                index_col="timeStep",
                names=["timeStep", ts_x],
            ),
            pd.read_csv(
                ts_y_path,
                sep=" ",
                skiprows=1,
                header=0,
                index_col="timeStep",
                names=["timeStep", ts_y],
            ),
        ],
        axis=1,
        verify_integrity=True,
    )
    # transform time steps to real time value in seconds
    data.index = data.index * 0.4
    data.index.name = "time"
    return data


def process_variation(study: SuqcRun, scenario_lbl: str, rep_ids: list) -> dict:
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
    df = OppAnalysis.merge_maps(study, scenario_lbl, rep_ids, data=("map_mean_count",))
    adf = adf_test(df.reset_index()[["simtime", "mean"]].set_index(["simtime"]))
    adf.name = "adf"
    adf = adf.to_frame()
    adf.columns = pd.Index([scenario_lbl], name="scenario")
    stat = df["mean"].describe().to_frame()  # over time
    stat.columns = pd.Index([scenario_lbl], name="scenario")
    _out = pd.concat([adf, stat], axis=0)

    print(f"done for {scenario_lbl}")
    return {"name": scenario_lbl, "stat": _out, "map": df}


def process_simulation_run(study: SuqcRun, scenario_map: dict, vadere_ts: pd.DataFrame):
    """Calcualte statistic for multiple scenarios

    Args:
        study (SuqcRun): Suqc study object (access to run definitions and output
        scenario_map (dict): Map of scenarios keys: [scenario_name[rep_ids, lbl, ts]], ts: link to ground truth
        vadere_ts (pd.DataFrame): Ground truth

    Returns:
        _type_: Map and ground truth statistics for 1D study
    """

    # average raw data over seeds and calculate statistic
    ret = []
    args = [(study, k, scenario_map[k]["rep"]) for k in scenario_map.keys()]
    with get_context("spawn").Pool(10) as p:
        ret: dict = p.starmap(process_variation, args)
    map_out = pd.concat([e["stat"] for e in ret], axis=1, verify_integrity=True).T

    # calculate ground truth
    ground_truth = []
    for col in vadere_ts.columns:
        ground_truth.append(adf_summary_test(vadere_ts, col))

    ground_truth = pd.concat(ground_truth, axis=1).T
    ground_truth.index.name = "scenario"
    return map_out, ground_truth


def next_multiple(val, off=5):
    """Next hightest multiple based on 'off' e.g off=5: 5 (5->5, 6->10, 9->10, 11->15, etc.)"""
    return floor(val / off) * off + off if val % off > 0 else 0


def create_map_ts_figure(
    study: SuqcRun,
    scenario_map: dict,
    ground_truth_ts: pd.DataFrame,
    ts: str,
    output_path: str | None = None,
):

    # get average density map from scenario map
    run_args = [(study, k, scenario_map[k]["rep"]) for k in scenario_map.keys()]
    with get_context("spawn").Pool(1) as pool:
        maps = pool.starmap(OppAnalysis.merge_maps, run_args)
    maps = pd.concat(maps, axis=0, verify_integrity=True)

    _v = ground_truth_ts.loc[:5000, [ts]]  # only first 5000 seconds
    max_y = _v[ts].max()  # get ground truth max y value for axis

    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    ax.plot(_v.index, _v, label=ts_[ts]["lbl"], marker=None, color="black")
    for sim in maps.index.get_level_values("sim").unique():
        if scenario_map[sim]["ts"] == ts:
            _df: pd.DataFrame = maps.loc[
                pd.IndexSlice[sim, :, "map_mean_count"], ["mean"]
            ]
            max_y = max(max_y, _df["mean"].max())
            ax.plot(
                _df.index.get_level_values("simtime"),
                _df["mean"],
                label=scenario_map[sim]["lbl"],
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
        fig.savefig(os.path.join(output_path, f"ped_count_ts_map_{ts}.pdf"))

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


def process_1d_scenario():
    matplotlib_set_latex_param()
    # out_path =   "/mnt/data1tb/results/mf_1d_8/"
    out_path = "/mnt/data1tb/results/mf_1d_1/"
    vadere_output = os.path.join(
        os.environ["HOME"],
        "repos/crownet/crownet/simulations/densityMap/vadere/output/",
    )
    ts_x_path = os.path.join(
        vadere_output, "mf_1d_m_const_2x5m_d20m_2022-05-31_14-15-54.174/numAgents.csv"
    )
    ts_y_path = os.path.join(
        vadere_output,
        "mf_1d_m_const_2x5m_d20m_25_2022-05-31_17-07-44.599/numAgents.csv",
    )

    run = SuqcRun(out_path)
    run_map = {
        "1d_0": dict(
            rep=list(range(0, 5)),
            ts=ts_x,
            lbl=r"1d\_0: Beacon $\vert$ Map $\Delta t = 300\vert 1000\,ms$",
        ),  # ts_x
        "1d_1": dict(
            rep=list(range(5, 10)),
            ts=ts_x,
            lbl=r"1d\_1: Beacon $\vert$ Map $\Delta t = 1000\vert 4000\,ms$",
        ),  # ts_x
        "1d_2": dict(
            rep=list(range(10, 15)),
            ts=ts_y,
            lbl=r"1d\_2: Beacon $\vert$ Map $\Delta t = 300\vert 1000\,ms$",
        ),  # ts_y
        "1d_3": dict(
            rep=list(range(15, 20)),
            ts=ts_y,
            lbl=r"1d\_3: Beacon $\vert$ Map $\Delta t = 1000\vert 4000\,ms$",
        ),  # ts_y
    }

    # figure (ground truth)
    v_ts = read_vadere_ts(ts_x_path, ts_y_path)
    # make_vader_ts_figure(v_ts, out_path)
    # figure (maps over ground truth)
    create_map_ts_figure(run, run_map, v_ts, ts_x, out_path)
    create_map_ts_figure(run, run_map, v_ts, ts_y, out_path)
    # statistics for map and ground truth data
    stat_sim, stat_vadere = process_simulation_run(run, run_map, v_ts)
    stat_sim.to_csv(os.path.join(out_path, "mapStat.csv"))
    stat_vadere.to_csv(os.path.join(out_path, "vadereStat.csv"))
    print("done")


def conv_err():
    matplotlib_set_latex_param()
    out_path = "/mnt/data1tb/results/mf_1d_8/"
    run = SuqcRun(out_path)
    run_map = {
        "1d_0": dict(
            rep=list(range(0, 5)),
            ts=ts_x,
            lbl=r"1d\_0: Beacon $\vert$ Map $\Delta t = 300\vert 1000\,ms$",
        ),  # ts_x
        "1d_1": dict(
            rep=list(range(5, 10)),
            ts=ts_x,
            lbl=r"1d\_1: Beacon $\vert$ Map $\Delta t = 1000\vert 4000\,ms$",
        ),  # ts_x
        "1d_2": dict(
            rep=list(range(10, 15)),
            ts=ts_y,
            lbl=r"1d\_2: Beacon $\vert$ Map $\Delta t = 300\vert 1000\,ms$",
        ),  # ts_y
        "1d_3": dict(
            rep=list(range(15, 20)),
            ts=ts_y,
            lbl=r"1d\_3: Beacon $\vert$ Map $\Delta t = 1000\vert 4000\,ms$",
        ),  # ts_y
    }

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
