from __future__ import annotations
from dataclasses import dataclass
from itertools import chain
import re
from typing import Any, Callable, Dict, List, Tuple
from roveranalyzer.analysis.common import (
    RunContext,
    Simulation,
    SuqcRun,
    RunMap,
    Parameter_Variation,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.utils.plot import StyleMap
from matplotlib.ticker import MaxNLocator
import seaborn as sb
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from omnetinireader.config_parser import OppConfigFileBase, ObjectValue


def get_run_map(
    study: SuqcRun, rep_count=10, map_filter: Callable[[Any], True] = lambda x: True
) -> RunMap:
    """Create RunMap labels based on alpha / cut off distance

    Args:
        study (SuqcRun): _description_
        rep_count (int, optional): Number of seeds used. Defaults to 10.
        map_filter (_type_, optional): _description_. Defaults to lambda x:True.

    Returns:
        RunMap:  A dictionary with [str, ParameterVariation]
    """

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


def collect_count_error_for_parameter_variation(
    study: SuqcRun, par_var: Parameter_Variation
):
    df = []
    for idx, rep in enumerate(par_var.reps):
        sim: Simulation = study.get_sim(rep)
        _df = sim.get_dcdMap().map_count_measure().loc[:, ["map_mean_err"]]
        _df.columns = [f"{idx}-{par_var['lbl']}"]
        df.append(_df)

    df = pd.concat(df, axis=1, verify_integrity=True)
    return df


def plot_count_error(study: SuqcRun, run_map: RunMap, figure_name: str):

    data: List[(pd.DataFrame, dict)] = run_kwargs_map(
        collect_count_error_for_parameter_variation,
        [dict(study=study, par_var=v) for v in run_map.get_parameter_variations()],
        pool_size=10,
        append_args=True,
    )

    with PdfPages(study.path(figure_name)) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle("Count Error for alpha_dist_iat")
        axes = list(chain(*axes))
        for idx, d in enumerate(data):
            df = d[0]
            par_var: Parameter_Variation = d[1]["par_var"]
            if "1_999" in par_var.label:
                # ignore last one
                continue
            ax: plt.Axes = axes[idx]
            ax.set_title(f"{par_var.label}")
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


def plot_mse_cell_over_time(
    study: SuqcRun, run_map: RunMap, data: pd.DataFrame, figure_name: str
):
    """Plot cell mean squared error over time for each variation. One line per seed."""

    with PdfPages(study.path(figure_name)) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle("Mean Squared Cell Error for alpha_dist_iat")
        axes = list(chain(*axes))

        # do not plot ymf variation (e.g. alpha = 1)
        for idx, par_var in enumerate(
            run_map.filtered_parameter_variations(lambda lbl: "1_999" not in lbl)
        ):
            ax: plt.Axes = axes[idx]
            ax.set_title(par_var.label)
            ax.axhline(y=0, color="black")
            for n, rep in enumerate(par_var.reps):
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


def _get_alpha_dist(
    run_map: RunMap,
) -> Tuple[Dict[str, Parameter_Variation], Dict[str, Parameter_Variation]]:
    """Sort parameter variations based on alpha and dist. Each variation is part of both Dict[str, ...]
    returned from this function.

    Returns:
        Tuple[Dict[str, Parameter_Variation], Dict[str, Parameter_Variation]]: _description_
    """
    alpha_ = {}
    dist_ = {}
    for par_var in run_map.get_parameter_variations():
        a = float(par_var.label.split("_")[0])
        _data = alpha_.get(a, [])
        _data.append(par_var)
        alpha_[a] = _data

        d = float(par_var.label.split("_")[1])
        _data = dist_.get(d, [])
        _data.append(par_var)
        dist_[d] = _data
    return alpha_, dist_


def _add_sorted_legends(ax: plt.Axes):
    h, l = ax.get_legend_handles_labels()
    h, l = zip(*sorted(zip(h, l), key=lambda t: t[1]))
    ax.legend(h, l)


def plot_mse_all(
    study: SuqcRun,
    data: pd.DataFrame,
    run_map: RunMap,
    styles: StyleMap,
    figure_name: str,
):

    alpha_, dist_ = _get_alpha_dist(run_map)

    fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
    ax_a: plt.Axes = ax[0]
    ax_b: plt.Axes = ax[1]

    for alpha, par_vars in alpha_.items():
        for par_var in par_vars:
            reps = par_var.reps
            ax_a.plot(
                alpha * np.ones(len(reps)),
                data.loc[reps],
                label=par_var.label,
                **styles.get_style(par_var.label),
            )
    ax_a.set_xlim(0.45, 1.0)
    ax_a.set_title("Cell MSE over alpha")
    ax_a.set_ylabel("Cell MSE")
    ax_a.set_xlabel("Alpha")
    _add_sorted_legends(ax_a)

    for dist, par_vars in dist_.items():
        for par_var in par_vars:
            reps = par_var.reps
            ax_b.plot(
                dist * np.ones(len(reps)),
                data.loc[reps],
                label=par_var.label,
                **styles.get_style(par_var.label),
            )
    ax_b.set_title("Cell MSE over distance")
    ax_b.set_xlabel("Distance threshold")
    _add_sorted_legends(ax_b)

    fig.savefig(study.path(figure_name))


def plot_mse_for_seed(
    study: SuqcRun, data: pd.DataFrame, run_map: dict, styles: StyleMap, figure_name
):
    """Plot cell mse for each axis (alpha, cut off distance) and each seed. Add comparison
    line for youngest measurement first (i.e. alpha=1.0)
    """

    alpha_, dist_ = _get_alpha_dist(run_map)

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
            _add_sorted_legends(ax_a)
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
            _add_sorted_legends(ax_b)
            ax_b.hlines(ymf_err, 0, 1000, colors="red")
            pdf.savefig(fig)


def plot_mse_yDist_to_ymf(
    study: SuqcRun,
    data: pd.DataFrame,
    run_map: dict,
    figure_name: str,
):
    """Plot the difference between the mse error compared to the youngest measurement first (i.e. alpha=1.0)
    For each parameter variation one bar chart with each bar is one seed (run_id).

    Args:
        study (SuqcRun): _description_
        data (pd.DataFrame): _description_
        run_map (dict): _description_
        figure_name (str): _description_
    """

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
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_1/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_2/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_3/")
    study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_4/")
    run_map: RunMap = get_run_map(study, map_filter=id_filter)

    data = OppAnalysis.get_mse_cell_data_for_study(
        study, run_map, hdf_path="cell_mse.h5", cell_count=84 * 79, pool_size=20
    )
    data_mean_by_run_id = data.groupby(by="run_id").mean()

    # repeat  ymf error to calculate mse_diff for each run
    ymf_err = np.tile(data_mean_by_run_id.loc[run_map["1_999_25"].reps], reps=17)
    data_mean_by_run_id = data_mean_by_run_id.to_frame()
    data_mean_by_run_id["err_to_ymf"] = data_mean_by_run_id["cell_mse"] - ymf_err

    styles = StyleMap(markersize=3, marker="o", linestyle="none")

    plot_mse_all(
        study, data_mean_by_run_id, run_map, styles, figure_name="cell_mse.pdf"
    )
    plot_mse_for_seed(
        study,
        data_mean_by_run_id["cell_mse"],
        run_map,
        styles,
        figure_name="cell_mse_for_seed.pdf",
    )
    plot_mse_yDist_to_ymf(study, data_mean_by_run_id, run_map, "mse_diff.pdf")
    # plot_mse_cell_over_time(study, run_map, data, "cell_mse_over_time.pdf")
    # plot_count_error(study, run_map, "count_error_over_time.pdf")

    print()


if __name__ == "__main__":
    main()
    print("done")
