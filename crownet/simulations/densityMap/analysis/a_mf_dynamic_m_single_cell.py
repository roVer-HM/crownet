from __future__ import annotations
from cProfile import label
from dataclasses import dataclass
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
from typing import Any, Callable, Dict, List
from unittest.util import strclass
from matplotlib import markers
from pyparsing import alphanums
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


def get_run_map(
    study: SuqcRun, rep_count=10, map_filter: Callable[[Any], True] = lambda x: True
):
    # def get_label(sim: Simulation):
    #     cfg: OppConfigFileBase = sim.run_context.oppini
    #     mapCfg: ObjectValue = cfg.get("*.pNode[*].app[1].app.mapCfg")
    #     scenario = cfg.get("**.vadereScenarioPath")
    #     r = re.compile(r".*_(\d+).*")
    #     if match := r.match(scenario):
    #         iat = match.groups()[0]
    #     else:
    #         iat = "??"
    #     return f"{mapCfg['alpha']}_{mapCfg['stepDist']}_{iat}"

    @dataclass(frozen=True)
    class _lbl_class:
        alpha: float = -1.0
        dist: float = -1.0
        iat: float = -1.0

        @classmethod
        def from_sim_v(cls, sim: Simulation, *args: Any, **kwds: Any) -> _lbl_class:
            cfg: OppConfigFileBase = sim.run_context.oppini
            mapCfg: ObjectValue = cfg.get("*.pNode[*].app[1].app.mapCfg")
            scenario = cfg.get("**.vadereScenarioPath")
            r = re.compile(r".*_(\d+).*")
            if match := r.match(scenario):
                iat = match.groups()[0]
            else:
                iat = "??"
            return str(cls(alpha=mapCfg["alpha"], dist=mapCfg["stepDist"], iat=iat))

        @classmethod
        def from_sim_bm(cls, sim: Simulation) -> _lbl_class:
            cfg: OppConfigFileBase = sim.run_context.oppini
            mapCfg: ObjectValue = cfg.get("*.misc[*].app[1].app.mapCfg")
            scenario = cfg.get("*.bonnMotionServer.traceFile")
            r = re.compile(r".*exp_(\d+).*")
            if match := r.match(scenario):
                iat = match.groups()[0]
            else:
                iat = "??"
            return str(cls(alpha=mapCfg["alpha"], dist=mapCfg["stepDist"], iat=iat))

        def __call__(self, *args: Any, **kwds: Any) -> Any:
            return self.__str__()

        def __str__(self) -> str:
            return self.__repr__()

        def __repr__(self) -> str:
            return f"{self.alpha}_{self.dist}_{self.iat}"

    run_map = study.create_run_map(
        rep=rep_count, lbl_f=_lbl_class.from_sim_bm, id_filter=map_filter
    )
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


def get_mse_cell_data(study: SuqcRun, run_map: dict, hdf_path: str):
    if os.path.exists(study.path(hdf_path)):
        data = pd.read_hdf(study.path(hdf_path), key="cell_mse")
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
        data.to_hdf(study.path(hdf_path), key="cell_mse", format="table")
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


def get_alpha_dist(run_map: dict):
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
    return alpha_, dist_


def add_sorted_legends(ax: plt.Axes):
    h, l = ax.get_legend_handles_labels()
    h, l = zip(*sorted(zip(h, l), key=lambda t: t[1]))
    ax.legend(h, l)


def plot_mse_all(
    study: SuqcRun,
    data: pd.DataFrame,
    run_map: dict,
    styles: StyleMap,
    figure_name: str,
):

    alpha_, dist_ = get_alpha_dist(run_map)

    fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
    ax_a: plt.Axes = ax[0]
    ax_b: plt.Axes = ax[1]

    for a, runs in alpha_.items():
        for r in runs:
            reps = r["rep"]
            ax_a.plot(
                a * np.ones(len(reps)),
                data.loc[reps],
                label=r["lbl"],
                **styles.get_style(r["lbl"]),
            )
    ax_a.set_xlim(0.45, 1.0)
    ax_a.set_title("Cell MSE over alpha")
    ax_a.set_ylabel("Cell MSE")
    ax_a.set_xlabel("Alpha")
    add_sorted_legends(ax_a)

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
    add_sorted_legends(ax_b)

    fig.savefig(study.path(figure_name))


def plot_mse_for_seed(
    study: SuqcRun, data: pd.DataFrame, run_map: dict, styles: StyleMap, figure_name
):

    alpha_, dist_ = get_alpha_dist(run_map)

    with PdfPages(study.path(figure_name)) as pdf:
        for _seed in range(10):
            ymf_err = data.loc[run_map["1_999_25"]["rep"][_seed]]
            fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
            fig.suptitle(f"Seed {_seed}")
            ax_a: plt.Axes = ax[0]
            ax_b: plt.Axes = ax[1]

            # ax_a.set_ylim(0.0015, 0.0025)
            ax_a.set_xlim(0.45, 1.0)
            for a, runs in alpha_.items():
                for r in runs:
                    reps = r["rep"]
                    ax_a.plot(
                        a,
                        data.loc[reps[_seed]],
                        label=r["lbl"],
                        **styles.get_style(r["lbl"]),
                    )
            ax_a.set_title("Cell MSE over alpha")
            ax_a.set_ylabel("Cell MSE")
            ax_a.set_xlabel("Alpha")
            add_sorted_legends(ax_a)
            ax_a.hlines(ymf_err, 0.45, 1.0, colors="red")

            for a, runs in dist_.items():
                for r in runs:
                    reps = r["rep"]
                    ax_b.plot(
                        a,
                        data.loc[reps[_seed]],
                        label=r["lbl"],
                        **styles.get_style(r["lbl"]),
                    )

            # ax_b.set_ylim(0.0015, 0.0025)
            ax_b.set_title("Cell MSE over distance")
            ax_b.set_xlabel("Distance threshold")
            add_sorted_legends(ax_b)
            ax_b.hlines(ymf_err, 0, 1000, colors="red")
            pdf.savefig(fig)


def plot_mse_yDist_to_ymf(
    study: SuqcRun,
    data: pd.DataFrame,
    run_map: dict,
    styles: StyleMap,
    figure_name: str,
):

    with PdfPages(study.path(figure_name)) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharey="all")
        fig.suptitle("Cell MSE difference between ymfDist and ymf")
        axes = list(chain(*axes))

        ymin = data["err_to_ymf"].min()
        ymax = data["err_to_ymf"].max()
        for idx, run_dict in enumerate(run_map.items()):
            if idx == 16:
                continue
            ax: plt.Axes = axes[idx]
            ax.hlines(0, -1, 10, colors="gray")
            ax.set_xticks(np.arange(10), labels=np.arange(10))
            ax.set_ylim(ymin, ymax)
            rep = run_dict[1]["rep"]
            lbl = run_dict[0]
            err = data.loc[rep, ["err_to_ymf"]]
            ax.bar(np.arange(10), err.iloc[:, 0].to_numpy(), 0.35)
            ax.set_ylabel("Cell MSE diff ymfDist-ymf")
            ax.set_xlabel("Seeds / Runs")
            ax.set_title(lbl)
        fig.tight_layout()
        pdf.savefig(fig)
    print("done")


def main():
    # id_filter = lambda x: all([i >= 170 for i in x])
    # id_filter = lambda x : all([i < 170 for i in x])
    id_filter = lambda x: True
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25/")
    study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_1/")
    run_map = get_run_map(study, map_filter=id_filter)

    data = get_mse_cell_data(study, run_map, hdf_path="cell_mse.h5")
    data = data.groupby(by="run_id").mean()

    ymf_err = np.tile(data.loc[run_map["1_999_25"]["rep"]], reps=17)
    data = data.to_frame()
    data["err_to_ymf"] = data["cell_mse"] - ymf_err

    styles = StyleMap(markersize=3, marker="o", linestyle="none")

    plot_mse_all(study, data, run_map, styles, figure_name="cell_mse.pdf")
    plot_mse_for_seed(
        study,
        data["cell_mse"],
        run_map,
        styles,
        figure_name="cell_mse_for_seed.pdf",
    )
    plot_mse_yDist_to_ymf(study, data, run_map, styles, "mse_diff.pdf")

    print()


if __name__ == "__main__":
    main()
    print("done")
