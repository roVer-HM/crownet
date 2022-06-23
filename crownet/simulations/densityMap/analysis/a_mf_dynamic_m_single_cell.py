from cProfile import label
import enum
import os
import itertools
from itertools import chain, product
from multiprocessing import get_context
import re
from statistics import mean
from itertools import repeat
from tokenize import group
from traceback import print_tb
from turtle import st
from typing import List
from matplotlib import markers
from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D, percentile
from roveranalyzer.simulators.opp.scave import CrownetSql
from roveranalyzer.utils.dataframe import FrameConsumer
from roveranalyzer.utils.general import Project
from roveranalyzer.utils.parallel import run_args_map, run_kwargs_map
from roveranalyzer.utils.plot import StyleMap, check_ax
from matplotlib.ticker import MaxNLocator
import seaborn as sb
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from shapely.geometry import Polygon
from matplotlib.backends.backend_pdf import PdfPages
from omnetinireader.config_parser import OppConfigFileBase, ObjectValue
import matplotlib.colors as mcolors


def min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


def get_run_map(study: SuqcRun, rep_count=5):
    def get_label(sim: Simulation):
        cfg: OppConfigFileBase = sim.run_context.oppini
        mapCfg: ObjectValue = cfg.get("*.pNode[*].app[1].app.mapCfg")
        scenario = cfg.get("**.vadereScenarioPath")
        r = re.compile(r".*_(\d+).*")
        if match := r.match(scenario):
            iat = match.groups()[0]
        else:
            iat = "??"
        return f"{mapCfg['alpha']}_{mapCfg['stepDist']}_{iat}"

    run_map = study.create_run_map(rep=rep_count, lbl_f=get_label)

    return run_map


def collect_count_error(study: SuqcRun, run_dict: dict):
    df = []
    for idx, rep in enumerate(run_dict["rep"]):
        sim: Simulation = study.get_sim(rep)
        _df = sim.get_dcdMap().map_count_measure().loc[:, ["map_mean_err"]]
        _df.columns = [f"{idx}-{run_dict['lbl']}"]
        df.append(_df)

    df = pd.concat(df, axis=1, verify_integrity=True)
    return df


def plot_position_error(study: SuqcRun):

    run_map = get_run_map(study)

    data: List[(pd.DataFrame, dict)] = run_kwargs_map(
        collect_count_error,
        [dict(study=study, run_dict=v) for v in run_map.values()],
        pool_size=10,
        append_args=True,
    )

    with PdfPages(study.path("count_error.pdf")) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle("Count Error for alpha_dist_iat")
        axes = list(chain(*axes))
        for idx, d in enumerate(data):
            df = d[0]
            run_dict = d[1]["run_dict"]
            ax: plt.Axes = axes[idx]
            ax.set_title(f"{run_dict['lbl']}")
            ax.axhline(y=0, color="black")
            for n, col in enumerate(df.columns):
                ax.plot(df.index, df[col], label=f"run_{n}", linewidth=1)
            ax.set_ylim(-10, 20)
            ax.set_ylabel("Mean count error")
            ax.set_xlabel("time in [s]")
        fig.tight_layout(rect=(0.0, 0.05, 1.0, 1.0))
        fig.legend(
            *axes[0].get_legend_handles_labels(),
            loc="lower center",
            ncol=5,
            bbox_to_anchor=(0.5, 0),
            bbox_transform=fig.transFigure,
        )
        pdf.savefig(fig)


def collect_cell_mse(
    study: SuqcRun,
    run_dict: dict,
    cell_count: int,
    consumer: FrameConsumer = FrameConsumer.EMPTY,
) -> pd.Series:
    df = []
    for idx, rep in enumerate(run_dict["rep"]):
        sim: Simulation = study.get_sim(rep)
        _df = sim.get_dcdMap().cell_count_measure(columns=["cell_mse"])
        _df = _df.groupby(by=["simtime"]).sum() / cell_count
        _df.columns = [rep]
        _df.columns.name = "run_id"
        df.append(_df)
    df = pd.concat(df, axis=1, verify_integrity=True)
    df = consumer(df)
    df = df.stack()  # series
    df.name = "cell_mse"
    return df


def get_mse_cell_data(study: SuqcRun, run_map: dict):
    if os.path.exists(study.path("cell_mse.h5")):
        data = pd.read_hdf(study.path("cell_mse.h5"), key="cell_mse")
    else:
        data: List[(pd.DataFrame, dict)] = run_kwargs_map(
            collect_cell_mse,
            [
                dict(study=study, run_dict=v, cell_count=84 * 79)
                for v in run_map.values()
            ],
            pool_size=20,
        )
        data: pd.DataFrame = pd.concat(data, axis=0, verify_integrity=True)
        data = data.sort_index()
        data.to_hdf(study.path("cell_mse.h5"), key="cell_mse", format="table")
    return data


def plot_mse_cell(study: SuqcRun):
    run_map = get_run_map(study)

    data = get_mse_cell_data(study, run_map)

    with PdfPages(study.path("cell_mse.pdf")) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle("Mean Squared Cell Error for alpha_dist_iat")
        axes = list(chain(*axes))

        for idx, run_dict in enumerate(run_map.values()):
            ax: plt.Axes = axes[idx]
            ax.set_title(f"{run_dict['lbl']}")
            ax.axhline(y=0, color="black")
            for n, rep in enumerate(run_dict["rep"]):
                df = data.loc[pd.IndexSlice[:, rep]]
                ax.plot(df.index, df, label=f"run_{n}", linewidth=1)
            ax.set_ylabel("cell mse")
            ax.set_xlabel("time in [s]")
        fig.tight_layout(rect=(0.0, 0.05, 1.0, 1.0))
        fig.legend(
            *axes[0].get_legend_handles_labels(),
            loc="lower center",
            ncol=5,
            bbox_to_anchor=(0.5, 0),
            bbox_transform=fig.transFigure,
        )
        pdf.savefig(fig)


def main(study: SuqcRun):
    run_map = get_run_map(study)
    data = get_mse_cell_data(study, run_map)
    data = data.groupby(by="run_id").mean()

    styles = StyleMap(markersize=3, marker="o", linestyle="none")
    alpha_ = {}
    dist_ = {}
    for lbl, run in run_map.items():
        a = float(lbl.split("_")[0])
        _data = alpha_.get(a, [])
        _data.append(run)
        alpha_[a] = _data

        d = float(lbl.split("_")[1])
        _data = dist_.get(d, [])
        _data.append(run)
        dist_[d] = _data

    fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
    ax_a: plt.Axes = ax[0]
    ax_b: plt.Axes = ax[1]

    ax_a.set_xlim(0.45, 1.0)
    for a, runs in alpha_.items():
        for r in runs:
            reps = r["rep"]
            ax_a.plot(
                a * np.ones(len(reps)),
                data.loc[reps],
                label=r["lbl"],
                **styles.get_style(r["lbl"]),
            )
    ax_a.set_title("Cell MSE over alpha")
    ax_a.set_ylabel("Cell MSE")
    ax_a.set_xlabel("Alpha")
    h, l = ax_a.get_legend_handles_labels()
    h, l = zip(*sorted(zip(h, l), key=lambda t: t[1]))
    ax_a.legend(h, l)

    for a, runs in dist_.items():
        for r in runs:
            reps = r["rep"]
            ax_b.plot(
                a * np.ones(len(reps)),
                data.loc[reps],
                label=r["lbl"],
                **styles.get_style(r["lbl"]),
            )
    ax_b.set_title("Cell MSE over distance")
    ax_b.set_xlabel("Distance threshold")
    h, l = ax_b.get_legend_handles_labels()
    h, l = zip(*sorted(zip(h, l), key=lambda t: t[1]))
    ax_b.legend(h, l)

    fig.savefig(study.path("cell_mse.pdf"))

    print()


if __name__ == "__main__":

    study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_3/")
    main(study)
    print("done")
