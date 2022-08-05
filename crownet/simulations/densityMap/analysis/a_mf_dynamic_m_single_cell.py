from __future__ import annotations
from dataclasses import dataclass
from itertools import chain
from functools import partial
import re
from typing import Any, Callable, Dict, List, Tuple
from roveranalyzer.analysis.common import (
    RunContext,
    Simulation,
    SimulationGroup,
    SuqcStudy,
    RunMap,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.utils.plot import StyleMap, check_ax
from matplotlib.ticker import MaxNLocator
import seaborn as sb
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from omnetinireader.config_parser import OppConfigFileBase, ObjectValue


def lbl_f_alpha_dist(sim: Simulation) -> str:
    cfg: OppConfigFileBase = sim.run_context.oppini
    mapCfg: ObjectValue = cfg.get("*.misc[*].app[1].app.mapCfg")
    return f"({mapCfg['alpha']},{mapCfg['stepDist']})"


def from_sim_v(sim: Simulation) -> str:
    cfg: OppConfigFileBase = sim.run_context.oppini
    mapCfg: ObjectValue = cfg.get("*.pNode[*].app[1].app.mapCfg")
    scenario = cfg.get("**.vadereScenarioPath")
    r = re.compile(r".*_(\d+).*")
    if match := r.match(scenario):
        iat = match.groups()[0]
    else:
        iat = "??"
    return f'{mapCfg["alpha"]}_{mapCfg["stepDist"]}_{iat}'


def from_sim_bm(sim: Simulation) -> str:
    cfg: OppConfigFileBase = sim.run_context.oppini
    mapCfg: ObjectValue = cfg.get("*.misc[*].app[1].app.mapCfg")
    scenario = cfg.get("*.bonnMotionServer.traceFile")
    r = re.compile(r".*exp_(\d+).*")
    if match := r.match(scenario):
        iat = match.groups()[0]
    else:
        iat = "??"
    return f'{mapCfg["alpha"]}_{mapCfg["stepDist"]}_{iat}'


def collect_count_error_for_simulation_group(sim_group: SimulationGroup):
    df = []
    for idx, sim in enumerate(sim_group.simulations):
        _df = sim.get_dcdMap().map_count_measure().loc[:, ["map_mean_err"]]
        _df.columns = [f"{idx}-{sim_group.group_name}"]
        df.append(_df)

    df = pd.concat(df, axis=1, verify_integrity=True)
    return df


def plot_count_error(run_map: RunMap, figure_name: str):

    data: List[(pd.DataFrame, dict)] = run_kwargs_map(
        collect_count_error_for_simulation_group,
        [dict(sim_group=v) for v in run_map.get_simulation_group()],
        pool_size=10,
        append_args=True,
    )

    with PdfPages(run_map.path(figure_name)) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle("Count Error for alpha_dist_iat")
        axes = list(chain(*axes))
        for idx, d in enumerate(data):
            df = d[0]
            sim_group: SimulationGroup = d[1]["sim_group"]
            if "1_999" in sim_group.label:
                # ignore last one
                continue
            ax: plt.Axes = axes[idx]
            ax.set_title(f"{sim_group.label}")
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


def plot_mse_cell_over_time(run_map: RunMap, data: pd.DataFrame, figure_name: str):
    """Plot cell mean squared error over time for each variation. One line per seed."""

    with PdfPages(run_map.path(figure_name)) as pdf:
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
) -> Tuple[Dict[str, List[SimulationGroup]], Dict[str, List[SimulationGroup]]]:
    """Sort parameter variations based on alpha and dist. Each variation is part of both Dict[str, ...]
    returned from this function.

    Returns:
        Tuple[Dict[str, List[SimulationGroup] Dict[str, List[SimulationGroup]]]: _description_
    """
    alpha_ = {}
    dist_ = {}
    for sim_group in run_map.get_simulation_group():
        a = float(sim_group.group_name.split("_")[0])
        _data = alpha_.get(a, [])
        _data.append(sim_group)
        alpha_[a] = _data

        d = float(sim_group.group_name.split("_")[1])
        _data = dist_.get(d, [])
        _data.append(sim_group)
        dist_[d] = _data
    return alpha_, dist_


def _add_sorted_legends(ax: plt.Axes):
    h, l = ax.get_legend_handles_labels()
    h, l = zip(*sorted(zip(h, l), key=lambda t: t[1]))
    ax.legend(h, l)


def plot_mse_all(
    data: pd.DataFrame,
    run_map: RunMap,
    styles: StyleMap,
    figure_name: str,
):

    alpha_, dist_ = _get_alpha_dist(run_map)

    fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
    ax_a: plt.Axes = ax[0]
    ax_b: plt.Axes = ax[1]

    for alpha, groups in alpha_.items():
        for sim_group in groups:
            reps = sim_group.reps
            ax_a.plot(
                alpha * np.ones(len(reps)),
                data.loc[reps, ["cell_mse"]],
                label=sim_group.group_name,
                **styles.get_style(sim_group.group_name),
            )
    ax_a.set_xlim(0.45, 1.0)
    ax_a.set_title("Cell MSE over alpha")
    ax_a.set_ylabel("Cell MSE")
    ax_a.set_xlabel("Alpha")
    _add_sorted_legends(ax_a)

    for dist, groups in dist_.items():
        for sim_group in groups:
            reps = sim_group.reps
            ax_b.plot(
                dist * np.ones(len(reps)),
                data.loc[reps, ["cell_mse"]],
                label=sim_group.group_name,
                **styles.get_style(sim_group.group_name),
            )
    ax_b.set_title("Cell MSE over distance")
    ax_b.set_xlabel("Distance threshold")
    _add_sorted_legends(ax_b)

    fig.savefig(run_map.path(figure_name))


def plot_mse_for_seed(
    data: pd.DataFrame, run_map: RunMap, styles: StyleMap, figure_name
):
    """Plot cell mse for each axis (alpha, cut off distance) and each seed. Add comparison
    line for youngest measurement first (i.e. alpha=1.0)
    """

    alpha_, dist_ = _get_alpha_dist(run_map)

    with PdfPages(run_map.path(figure_name)) as pdf:
        for _seed in range(10):
            ymf_err = data.loc[run_map["1_999_25"][_seed].global_id()]
            fig, ax = plt.subplots(1, 2, figsize=(16, 9), sharey="all")
            fig.suptitle(f"Seed {_seed}")
            ax_a: plt.Axes = ax[0]
            ax_b: plt.Axes = ax[1]

            # ax_a.set_ylim(0.0015, 0.0025)
            ax_a.set_xlim(0.45, 1.0)
            for a, groups in alpha_.items():
                for sim_group in groups:
                    reps = sim_group.reps
                    ax_a.plot(
                        a,
                        data.loc[reps[_seed]],
                        label=sim_group.group_name,
                        **styles.get_style(sim_group.group_name),
                    )
            ax_a.set_title("Cell MSE over alpha")
            ax_a.set_ylabel("Cell MSE")
            ax_a.set_xlabel("Alpha")
            _add_sorted_legends(ax_a)
            ax_a.hlines(ymf_err, 0.45, 1.0, colors="red")

            for a, groups in dist_.items():
                for sim_group in groups:
                    reps = sim_group.reps
                    ax_b.plot(
                        a,
                        data.loc[reps[_seed]],
                        label=sim_group.group_name,
                        **styles.get_style(sim_group.group_name),
                    )

            # ax_b.set_ylim(0.0015, 0.0025)
            ax_b.set_title("Cell MSE over distance")
            ax_b.set_xlabel("Distance threshold")
            _add_sorted_legends(ax_b)
            ax_b.hlines(ymf_err, 0, 1000, colors="red")
            pdf.savefig(fig)


def plot_mse_yDist_to_ymf(
    data: pd.DataFrame,
    run_map: RunMap,
    figure_name: str,
):
    """Plot the difference between the mse error compared to the youngest measurement first (i.e. alpha=1.0)
    For each parameter variation one bar chart with each bar is one seed (run_id).

    Args:
        data (pd.DataFrame): _description_
        run_map (dict): _description_
        figure_name (str): _description_
    """

    with PdfPages(run_map.path(figure_name)) as pdf:
        fig, axes = plt.subplots(4, 4, figsize=(16, 9), sharey="all")
        fig.suptitle("Cell MSE difference between ymfDist and ymf")
        axes = list(chain(*axes))

        ymin = data["err_to_ymf"].min()
        ymax = data["err_to_ymf"].max()
        for idx, item in enumerate(run_map.items()):
            if idx == 16:
                continue
            group_name = item[0]
            sim_group: SimulationGroup = item[1]
            seed_num = len(sim_group)
            ax: plt.Axes = axes[idx]
            ax.hlines(0, -1, seed_num, colors="gray")
            ax.set_xticks(np.arange(seed_num), labels=np.arange(seed_num))
            ax.set_ylim(ymin, ymax)
            err = data.loc[sim_group.reps, ["err_to_ymf"]]
            ax.bar(np.arange(len(sim_group)), err.iloc[:, 0].to_numpy(), 0.35)
            ax.set_ylabel("Cell MSE diff ymfDist-ymf")
            ax.set_xlabel("Seeds / Runs")
            ax.set_title(group_name)
        fig.tight_layout()
        pdf.savefig(fig)
    print("done")


def plot_mse_yDist_to_ymf_box_plot(
    run_map: RunMap,
    data: pd.DataFrame,
    figure_name: str,
):

    # create mean cell mse for each run_id over time.
    # data = data.groupby("run_id").mean()
    # add label of the form '({alpha}, {dist}) to data
    data = (
        data.reset_index()
        .drop(columns=["seed", "run_id", "run_id_ymf", "cell_mse", "ymf_cell_mse"])
        .set_index(["run_index", "label"])
        .unstack(["label"])
    )
    sorted_index = data.mean().sort_values().index
    data = data[sorted_index]
    data.columns = data.columns.droplevel(0)

    # data.reset_index().set_index(['run_id', 'label']).boxplot(by=["label"])
    fig, ax = check_ax()
    ax.set_title(
        f"Cell MSE difference between ymfDist and ymf (sorted by mean) N={len(data.iloc[:,0])}"
    )
    data.boxplot(ax=ax)
    ax.set_xlabel(f"Variation (alpha, cut off distance)")
    ax.set_ylabel("cell mse difference")
    ax.axhline(y=0, color="red")
    ax.axvline(x=np.argmax(data.columns == "(1,999)") + 1, color="red")

    fig.savefig(run_map.path(figure_name))


def plot_all(
    run_map: RunMap,
    styles: StyleMap,
    data: pd.DataFrame,
    data_mean_by_run_id: pd.DataFrame,
):

    plot_mse_all(data_mean_by_run_id, run_map, styles, figure_name="cell_mse.pdf")
    plot_mse_for_seed(
        data_mean_by_run_id["cell_mse"],
        run_map,
        styles,
        figure_name="cell_mse_for_seed.pdf",
    )
    plot_mse_yDist_to_ymf(data_mean_by_run_id, run_map, "mse_diff.pdf")
    # plot_mse_cell_over_time(run_map, data, "cell_mse_over_time.pdf")
    # plot_count_error(run_map, "count_error_over_time.pdf")
    plot_mse_yDist_to_ymf_box_plot(
        run_map, data_mean_by_run_id, "cell_mse_box_plot.pdf"
    )


def _get_mse_data(run_map: RunMap):

    data = OppAnalysis.get_mse_cell_data_for_study(
        run_map, hdf_path="cell_mse.h5", cell_count=84 * 79, pool_size=20
    )
    data_mean_by_run_id = data.groupby(by="run_id").mean().to_frame()
    seeds = run_map.id_to_label_series(lbl_f=lbl_f_alpha_dist, enumerate_run=True)
    data_mean_by_run_id = data_mean_by_run_id.join(seeds)

    # add ymf error column to all calculate difference
    ymf_group = run_map["1_999_25"]
    ymf_err = data_mean_by_run_id.loc[
        ymf_group.reps, ["cell_mse", "seed"]
    ].reset_index()
    ymf_err.columns = ["run_id_ymf", "ymf_cell_mse", "seed"]
    data_mean_by_run_id = (
        data_mean_by_run_id.reset_index()
        .merge(ymf_err, on="seed")
        .set_index("run_id")
        .sort_index()
    )

    data_mean_by_run_id["err_to_ymf"] = (
        data_mean_by_run_id["cell_mse"] - data_mean_by_run_id["ymf_cell_mse"]
    )
    return data, data_mean_by_run_id


def main():
    # id_filter = lambda x: all([i >= 170 for i in x])
    # id_filter = lambda x : all([i < 170 for i in x])
    id_filter = lambda x: True
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_1/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_2/")
    # study = SuqcRun("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_3/")
    study1 = SuqcStudy("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_4/")
    study2 = SuqcStudy("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_test/")

    run_map: RunMap = RunMap(output_dir="/mnt/data1tb/results/mf_dyn_20_seeds")

    run_map = study1.update_run_map(
        run_map, sim_per_group=10, id_offset=0, lbl_f=from_sim_bm, allow_new_groups=True
    )
    run_map = study2.update_run_map(
        run_map,
        sim_per_group=10,
        id_offset=len(study1),
        lbl_f=from_sim_bm,
        allow_new_groups=False,
    )
    styles = StyleMap(markersize=3, marker="o", linestyle="none")
    data, data_mean_by_run_id = _get_mse_data(run_map)

    # plot_mse_yDist_to_ymf_box_plot(
    #     study, run_map, data_mean_by_run_id, "cell_mse_box_plot.pdf"
    # )
    plot_all(run_map, styles, data, data_mean_by_run_id)


if __name__ == "__main__":
    main()
    print("done")
