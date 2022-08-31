from __future__ import annotations
from functools import partial
from itertools import chain
import re
from matplotlib.colors import LinearSegmentedColormap
from typing import Callable, Dict, List, Tuple, Any
from roveranalyzer.analysis.common import (
    Simulation,
    RunMap,
    SimulationGroup,
    SimGroupFilter,
    SuqcStudy,
    RunMap,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.simulators.crownet.common.dcd_metadata import DcdMetaData
from roveranalyzer.simulators.vadere.plots.scenario import (
    Scenario,
    VaderScenarioPlotHelper,
)
from roveranalyzer.utils import dataframe
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.utils.plot import PlotUtil, StyleMap, check_ax, empty_fig
import roveranalyzer.utils.plot as _Plot
from matplotlib.ticker import MaxNLocator
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from scipy.stats import mannwhitneyu, kstest
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


def sim_group_bonnmotion(sim: Simulation, **kwds) -> SimulationGroup:
    cfg: OppConfigFileBase = sim.run_context.oppini
    mapCfg: ObjectValue = cfg.get("*.misc[*].app[1].app.mapCfg")
    scenario = cfg.get("*.bonnMotionServer.traceFile")
    r = re.compile(r".*exp_(\d+).*")
    if match := r.match(scenario):
        iat = match.groups()[0]
    else:
        iat = "??"
    kwargs = dict(**kwds)
    kwargs["group_name"] = f'{mapCfg["alpha"]}_{mapCfg["stepDist"]}_{iat}'
    attr = kwargs.get("attr", {})
    attr["alpha"] = mapCfg["alpha"]
    attr["stepDist"] = mapCfg["stepDist"]
    attr["iat"] = iat
    kwargs["attr"] = attr
    return SimulationGroup(**kwargs)


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


def describtive_two_way_comparison_msce(
    run_map: RunMap,
    filter_run: SimGroupFilter = lambda _: True,
    compare_with="1_999_25",
    output_path="mce_stats_for_alg.pdf",
    value_axes_label="Mean Squared Cell Error (MSCE)",
):
    data, data_mean_by_run_id = _get_mse_data(run_map)
    data = data.reset_index().merge(
        run_map.id_to_label_series(enumerate_run=True).reset_index(), on="run_id"
    )

    groups = [g.group_name for g in run_map.values() if filter_run(g)]
    groups.sort(
        key=lambda x: (int(run_map.attr(x, "alpha")), int(run_map.attr(x, "stepDist")))
    )
    if compare_with not in groups:
        groups.append(compare_with)

    # MSCE mean cell error for one timestep created from MSE of each cell
    data = (
        data.set_index(["label", "simtime", "seed"])
        .loc[:, ["cell_mse"]]
        .groupby(["label", "simtime"])
        .mean()
        .unstack(["label"])
        .droplevel(0, axis=1)
    )
    data = data[groups]
    # split groups into triplets containging ground truth 'compare_with' and one of the other runs
    col_combination = [(c, compare_with) for c in groups if c != compare_with]

    # cleanup lables and fix color palette
    lbl_dict = {c: c.replace("_", "-") for c in data.columns}
    palette = sns.color_palette("colorblind", n_colors=len(col_combination[0]))
    with run_map.pdf_page(output_path) as pdf_file:
        for cols in col_combination:
            title = f"Comparision {' - '.join(cols)}"
            print(f"build {title}")

            OppAnalysis.plot_descriptive_comparison(
                data.loc[:, cols],
                lbl_dict=lbl_dict,
                run_map=run_map,
                out_name=output_path,
                pdf_file=pdf_file,
                palette=palette,
                value_axes_label=value_axes_label,
            )


def describtive_two_way_comparison_count(
    run_map: RunMap,
    filter_run: SimGroupFilter = lambda _: True,
    compare_with="1_999_25",
    output_path="count_stats_for_alg.pdf",
    value_axes_label="pedestrian count",
):

    maps = OppAnalysis.merge_maps_for_run_map(
        run_map, pool_size=20, hdf_path=run_map.path("maps.h5")
    )

    # select runs to compare with
    groups = [g.group_name for g in run_map.values() if filter_run(g)]
    groups.sort(key=lambda x: run_map.attr(x, "alpha"))
    if compare_with not in groups:
        groups.append(compare_with)

    sim_lbls = maps.index.get_level_values(0).unique()
    if not all(g in sim_lbls for g in groups):
        raise ValueError(
            f"Not all selecetd simulation runs exist. selected groups: '{groups}' groups in data: '{sim_lbls}' "
        )

    # choose selecetd groups + ground truth and create a long format dataframe
    data = (
        maps.loc[pd.IndexSlice[groups, :, "map_mean_count"], ["mean"]]
        .droplevel("data", axis=0)
        .unstack("sim")
        .droplevel(0, axis=1)
    )
    data_gt = (
        maps.loc[pd.IndexSlice[groups[0], :, "map_glb_count"], ["mean"]]
        .droplevel(["sim", "data"], axis=0)
        .set_axis(["gt"], axis=1)
    )
    data = pd.concat([data_gt, data], axis=1)

    # split groups into triplets containging ground truth 'compare_with' and one of the other runs
    col_combination = [("gt", c, compare_with) for c in groups if c != compare_with]

    # clearup lables and fix color palette
    lbl_dict = {c: c.replace("_", "-") for c in data.columns}
    palette = sns.color_palette("colorblind", n_colors=len(col_combination[0]))
    with run_map.pdf_page(output_path) as pdf_file:
        for cols in col_combination:
            title = f"Comparision {' - '.join(cols)}"
            print(f"build {title}")

            OppAnalysis.plot_descriptive_comparison(
                data.loc[:, cols],
                lbl_dict=lbl_dict,
                run_map=run_map,
                out_name=output_path,
                pdf_file=pdf_file,
                palette=palette,
                value_axes_label=value_axes_label,
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


def get_run_map_N20(output_path: str, *args, **kwargs) -> RunMap:
    """One simulation run with 20 seeds"""
    study1 = SuqcStudy(kwargs["src_path"])
    run_map: RunMap = RunMap(output_dir=output_path)

    run_map = study1.update_run_map(
        run_map,
        sim_per_group=20,
        id_offset=0,
        sim_group_factory=sim_group_bonnmotion,
        allow_new_groups=True,
    )
    return run_map


def get_run_map_N20_split(output_path: str, *args, **kwargs) -> RunMap:
    """Combine two simulation runs with 10 seeds each"""
    study1 = SuqcStudy("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_4/")
    study2 = SuqcStudy("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_test/")

    run_map: RunMap = RunMap(output_dir=output_path)

    run_map = study1.update_run_map(
        run_map,
        sim_per_group=10,
        id_offset=0,
        sim_group_factory=sim_group_bonnmotion,
        allow_new_groups=True,
    )

    # append extra seeds to existing groups.
    run_map = study2.update_run_map(
        run_map,
        sim_per_group=10,
        id_offset=len(study1),
        sim_group_factory=sim_group_bonnmotion,
        allow_new_groups=False,
    )

    return run_map


def main(run_map: RunMap):
    styles = StyleMap(markersize=3, marker="o", linestyle="none")
    data, data_mean_by_run_id = _get_mse_data(run_map)

    # plot_mse_yDist_to_ymf_box_plot(
    #     study, run_map, data_mean_by_run_id, "cell_mse_box_plot.pdf"
    # )
    plot_all(run_map, styles, data, data_mean_by_run_id)


if __name__ == "__main__":
    # run_map = RunMap.load_or_create(get_run_map_N20_split, output_path="/mnt/data1tb/results/_density_map/03_dynamic_output/")
    # run_map = RunMap.load_or_create(
    #     partial(get_run_map_N20, src_path="/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_5/"),
    #     output_path="/mnt/data1tb/results/_density_map/03a_dynamic_output/",
    # )
    run_map = RunMap.load_or_create(
        partial(
            get_run_map_N20,
            src_path="/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_6/",
        ),
        output_path="/mnt/data1tb/results/_density_map/03b_dynamic_output/",
    )

    main(run_map)
    plot_per_seed_stats(run_map)
    describtive_two_way_comparison_msce(run_map)
    describtive_two_way_comparison_count(run_map)
    print("done")
