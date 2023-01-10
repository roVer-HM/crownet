from __future__ import annotations
from cProfile import label
from cmath import exp
import cmath
from functools import partial
import logging
import os
from itertools import chain
import sys
from mpl_toolkits.axes_grid1 import make_axes_locatable
from mpl_toolkits import axes_grid1
from shapely.geometry import Polygon
from matplotlib import patches
import re
import matplotlib.lines as mlines
from matplotlib.transforms import Bbox
from matplotlib.colors import (
    LinearSegmentedColormap,
    TwoSlopeNorm,
    to_rgba_array,
    Normalize,
)
import seaborn as sn
from collections.abc import Sized, Sequence
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
import roveranalyzer.utils.dataframe as FrameUtl
from matplotlib.ticker import MaxNLocator
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
from scipy.stats import mannwhitneyu, kstest
from matplotlib.backends.backend_pdf import PdfPages
from omnetinireader.config_parser import OppConfigFileBase, ObjectValue
from roveranalyzer.utils.plot import check_ax, matplotlib_set_latex_param

# load global settings
matplotlib_set_latex_param()


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


def mse_combi_plot(
    data: pd.DataFrame,
    run_map: RunMap,
):
    layout = np.array(range(4 * 6)).reshape((4, 6))
    layout[:, 4:] = 99
    with plt.rc_context(_Plot.paper_rc()):
        fig, axd = plt.subplot_mosaic(layout, figsize=(20, 6))
        idx = [i for i in axd.keys() if i < 99]
        idx.sort()
        diff_ax = [axd[i] for i in idx]
        plot_mse_yDist_to_ymf(data, run_map, figure_name=None, axes=diff_ax)
        for ax in diff_ax:
            ax.xaxis.grid(False)
            ax.yaxis.grid(False)
        plot_mse_yDist_to_ymf_box_plot(run_map, data, figure_name=None, ax=axd[99])
        axd[99].set_title("")
        fig.tight_layout(w_pad=0.3)
        fig.savefig(run_map.path("msme_combi.pdf"))
    print("Hi")


def plot_mse_yDist_to_ymf(
    data: pd.DataFrame,
    run_map: RunMap,
    figure_name: str,
    axes: List[plt.Axes] | None = None,
):
    """Plot the difference between the mse error compared to the youngest measurement first (i.e. alpha=1.0)
    For each parameter variation one bar chart with each bar is one seed (run_id).

    Args:
        data (pd.DataFrame): _description_
        run_map (dict): _description_
        figure_name (str): _description_
    """

    if axes is None:
        fig, axes = plt.subplots(4, 4, figsize=(20, 7), sharey="all")
        # fig.suptitle("Cell MSE difference between ymfDist and ymf")
        axes = list(chain(*axes))
    else:
        fig = axes[0].get_figure()

    ymin = data["err_to_ymf"].min()
    ymax = data["err_to_ymf"].max()
    for idx, item in enumerate(run_map.items()):
        if idx == 16:
            continue
        if idx % 4 == 0:
            if idx >= 12:
                ax_loc = "left_low"
            else:
                ax_loc = "left"
        else:
            if idx >= 12:
                ax_loc = "low"
            else:
                ax_loc = ""

        group_name = item[0]
        sim_group: SimulationGroup = item[1]
        seed_num = len(sim_group)
        ax: plt.Axes = axes[idx]
        ax.hlines(0, -1, seed_num, colors="gray")
        ax.set_ylim(ymin, ymax)
        err = data.loc[sim_group.reps, ["err_to_ymf"]]
        ax.bar(np.arange(len(sim_group)), err.iloc[:, 0].to_numpy(), 0.55)
        if "left" in ax_loc:
            ax.set_ylabel("MSME diff", fontsize="large")
        if "low" in ax_loc:
            ax.set_xlabel("Seeds / Runs")
            ax.set_xticks(
                np.arange(seed_num),
                labels=[i if i % 2 == 0 else "" for i in np.arange(seed_num)],
            )
            ax.set_xticklabels(ax.get_xticklabels(), fontsize="x-large")
        else:
            ax.set_xticks(np.arange(seed_num), labels=np.arange(seed_num))
            ax.set_xticklabels([])
        # if not "left" in ax_loc:
        #     ax.set_yticklabels([])
        lbl = group_name.split("_")
        if idx == 0:
            ax.text(
                0.05,
                0.1,
                f"$\\alpha={lbl[0]}, D={lbl[1]}$",
                fontsize="xx-large",
                horizontalalignment="left",
                verticalalignment="bottom",
                transform=ax.transAxes,
                fontweight="bold",
                backgroundcolor="white",
                zorder=9,
            )
        else:
            ax.text(
                0.97,
                0.9,
                f"$\\alpha={lbl[0]}, D={lbl[1]}$",
                fontsize="xx-large",
                horizontalalignment="right",
                verticalalignment="top",
                transform=ax.transAxes,
                fontweight="bold",
                backgroundcolor="white",
                zorder=9,
            )

    if figure_name is not None:
        fig.tight_layout()
        with PdfPages(run_map.path(figure_name)) as pdf:
            pdf.savefig(fig)
    return fig, axes


def plot_mse_yDist_to_ymf_box_plot(
    run_map: RunMap, data: pd.DataFrame, figure_name: str, ax: plt.Axes | None = None
):

    # create mean cell mse for each run_id over time.
    # data = data.groupby("run_id").mean()
    # add label of the form '({alpha}, {dist}) to data
    keep_cols = ["run_index", "label", "err_to_ymf"]
    data = (
        data.reset_index()[keep_cols]
        .set_index(["run_index", "label"])
        .unstack(["label"])
    )
    sorted_index = data.mean().sort_values().index
    data = data[sorted_index]
    data.columns = data.columns.droplevel(0)
    data = data.rename({"(1,999)": "YMF"}, axis=1)

    with plt.rc_context(_Plot.paper_rc(tick_labelsize="x-large")):
        # data.reset_index().set_index(['run_id', 'label']).boxplot(by=["label"])
        fig, ax = check_ax(ax, figsize=(8, 6.5))
        # ax.set_title(
        #     f"Cell MSE difference between ymfDist and ymf (sorted by mean) N={len(data.iloc[:,0])}"
        # )
        b = data.boxplot(ax=ax, meanline=True, showmeans=True)
        ax.set_xlabel(f"Variation (alpha, cut off distance)", fontsize="xx-large")
        ax.set_ylabel("MSME diff", labelpad=-8, fontsize="large")
        ax.axhline(y=0, color="red")
        ax.axvline(x=np.argmax(data.columns == "YMF") + 1, color="red")
        ax.set_xticklabels(ax.get_xticklabels(), rotation=90, ha="center")
        ax.legend(
            handles=[
                mlines.Line2D([], [], color="#a6a6a6", linestyle="-", label="median"),
                mlines.Line2D([], [], color="#54a767", linestyle="--", label="mean"),
            ],
            loc="upper left",
            facecolor="white",
            framealpha=1,
        )

        if figure_name is not None:
            fig.tight_layout()
            fig.savefig(run_map.path(figure_name))
        else:
            return fig, ax


def plot_per_seed_stats(run_map: RunMap):
    data, data_mean_by_run_id = _get_mse_data(run_map)
    ts_per_seed = data.reset_index().merge(
        run_map.id_to_label_series(enumerate_run=True).reset_index(), on="run_id"
    )
    ts_per_seed = (
        ts_per_seed.set_index(["label", "seed", "simtime"])
        .drop(columns=["run_id", "run_index"])
        .unstack(["label"])
        .droplevel(0, axis=1)
    )

    groups = [g.group_name for g in run_map.values() if g.attr["alpha"] > 0.7]
    groups.sort(key=lambda x: run_map.attr(x, "alpha"))

    with run_map.pdf_page("mse_ts.pdf") as pdf:
        for seed in ts_per_seed.index.get_level_values("seed").unique():
            print(f"seed: {seed}")
            _df = ts_per_seed.loc[seed, ["0.9_60_25", "1_999_25"]]
            fig, ax = _Plot.check_ax()
            sns.lineplot(data=_df)
            ax.set_title(f"Time series of MSE for seed {seed}")
            pdf.savefig(fig)
            plt.close(fig)

            f, ax = _Plot.check_ax()
            sns.ecdfplot(
                _df,
                ax=ax,
            )
            ax.set_title(f"ECDF pedestrian count")
            ax.set_xlabel("Pedestrian count")
            ax.get_legend().set_title(None)
            sns.move_legend(ax, "upper left")
            pdf.savefig(f)
            plt.close(f)

            fig, ax = _Plot.check_ax()
            OppAnalysis.calculate_equality_tests(_df, ax=ax)
            ax.set_title(f"Test for seed {seed}")
            pdf.savefig(fig)
            plt.close(fig)

    print("hi")


def cells_with_biggest_error_diff(
    run_map: RunMap,
    cmp_groups: list["str"],
    seed_idx: int,
    abs_err_filter: float = 1.0,
    lbl_f=lambda x: x.group_name,
):
    """Find cells which have the most number of errors over all time steps and
    agents. Further compare two runs and return cells where the number of errors
    entries differe the most
    """
    df = []

    for g in cmp_groups:
        _i = pd.IndexSlice
        sim: Simulation = run_map[g].simulations[seed_idx]
        _df = sim.get_dcdMap().count_p[_i[:, :, :, 1:], ["err"]]
        _df["err"] = np.abs(_df["err"])
        _df.columns = ["abs_err"]
        _df = _df[_df["abs_err"] >= abs_err_filter]
        _df["lbl"] = lbl_f(run_map[g])
        df.append(_df)
    df = pd.concat(df, axis=0)
    df = df.sort_values("abs_err", ascending=False)

    # [x, y](err_count_run1, err_count_run2, err_count_abs_diff)
    # Number of errors >= abs_err_filter of each cell over all agents and time steps
    err_count_matrix = (
        df.reset_index()
        .set_index(["lbl", "x", "y"])
        .drop(columns=["simtime", "ID"])
        .groupby(["lbl", "x", "y"])
        .count()
        .unstack("lbl")
        .fillna(0)
        .droplevel(0, axis=1)
    )
    # err_count_matrix["err_count_abs_diff"] = np.abs(err_count_matrix.iloc[:, 0] - err_count_matrix.iloc[:, 1])
    err_count_matrix[
        f"[{err_count_matrix.columns[0]} - {err_count_matrix.columns[1]}]"
    ] = (err_count_matrix.iloc[:, 0] - err_count_matrix.iloc[:, 1])
    # collect grid for color mesh plots
    meta: DcdMetaData = (
        run_map[cmp_groups[0]].simulations[seed_idx].get_dcdMap().metadata
    )
    mgrid = meta.mgrid()
    # add missing cell counts (defaults to zero)
    missing_index = meta.grid_index_2d(real_coords=True).difference(
        err_count_matrix.index
    )
    err_count_matrix = pd.concat(
        [
            pd.DataFrame(0, columns=err_count_matrix.columns, index=missing_index),
            err_count_matrix,
        ],
        axis=0,
    ).sort_index()

    return err_count_matrix, mgrid


class MyCm(LinearSegmentedColormap):
    def __init__(
        self,
        name: str,
        segmentdata: Mapping[str, Sequence[Tuple[float, float, float]]],
        N: int = ...,
        gamma: float = ...,
    ) -> None:
        super().__init__(name, segmentdata, N, gamma)

    def __call__(self, X, alpha=None, bytes=False):
        print(X)
        return super().__call__(X, alpha, bytes)

    def from_list(name, colors, N=256, gamma=1.0):
        """
        Create a `LinearSegmentedColormap` from a list of colors.

        Parameters
        ----------
        name : str
            The name of the colormap.
        colors : array-like of colors or array-like of (value, color)
            If only colors are given, they are equidistantly mapped from the
            range :math:`[0, 1]`; i.e. 0 maps to ``colors[0]`` and 1 maps to
            ``colors[-1]``.
            If (value, color) pairs are given, the mapping is from *value*
            to *color*. This can be used to divide the range unevenly.
        N : int
            The number of rgb quantization levels.
        gamma : float
        """
        if not np.iterable(colors):
            raise ValueError("colors must be iterable")

        if (
            isinstance(colors[0], Sized)
            and len(colors[0]) == 2
            and not isinstance(colors[0], str)
        ):
            # List of value, color pairs
            vals, colors = zip(*colors)
        else:
            vals = np.linspace(0, 1, len(colors))

        r, g, b, a = to_rgba_array(colors).T
        cdict = {
            "red": np.column_stack([vals, r, r]),
            "green": np.column_stack([vals, g, g]),
            "blue": np.column_stack([vals, b, b]),
            "alpha": np.column_stack([vals, a, a]),
        }

        return MyCm(name, cdict, N, gamma)


def get_clipped_area():
    cell_size = 5.0
    return slice(22 * cell_size, 50 * cell_size, cell_size), slice(
        8 * cell_size, 63 * cell_size, cell_size
    )


def get_legal_cells(
    scenario: VaderScenarioPlotHelper, xy_slices: Tuple[slice, slice], c=5.0
):

    _covered = []
    _free = []
    _x, _y = xy_slices
    obs: List[Polygon] = [
        scenario.scenario.shape_to_list(s["shape"], to_shapely=True)
        for s in scenario.scenario.obstacles
    ]
    for x in np.arange(_x.start, _x.stop, c):
        for y in np.arange(_y.start, _y.stop, c):
            idx = (x, y)
            cell = Polygon([[x, y], [x + c, y], [x + c, y + c], [x, y + c], [x, y]])
            for _o in obs:
                if _o.covers(cell):
                    _covered.append(idx)
                    break
            if _covered[-1] != idx:
                _free.append(idx)

    _covered = pd.MultiIndex.from_tuples(_covered, names=["x", "y"])
    _free = pd.MultiIndex.from_tuples(_free, names=["x", "y"])

    return _free, _covered


def add_cell_patches(ax: plt.axes, xy, c=5.0, **kwds):
    for x, y in xy:
        # x = x+c/2
        # y = y+c/2
        cell = Polygon([[x, y], [x + c, y], [x + c, y + c], [x, y + c], [x, y]])
        patch = patches.Polygon(
            list(cell.exterior.coords),
            edgecolor="black",
            fill=False,
            closed=True,
            **kwds,
        )
        ax.add_patch(patch)
    return ax


def write_cell_tex(run_map: RunMap):
    scenario = VaderScenarioPlotHelper(
        "../study/traces_dynamic.d/mf_dyn_exp_25.out/BASIS_mf_dyn_exp_25.out.scenario"
    )
    _free, _covered = PlotUtil.get_vadere_legal_cells(scenario, get_clipped_area())
    PlotUtil.cell_to_tex(_free, c=5.0, fd=run_map.path("cell_tikz.tex"), attr=["cell"])


def create_od_matrix(run_map: RunMap):
    scenario = VaderScenarioPlotHelper(
        "../study/traces_dynamic.d/mf_dyn_exp_25.out/BASIS_mf_dyn_exp_25.out.scenario"
    )
    od_target_changer = pd.read_csv(
        os.path.join(os.path.dirname(__file__), "./S3_targetChanger_OD.csv"),
        delimiter=";",
    )
    od_target_changer["next Targets"] = od_target_changer["next Targets"].apply(
        lambda x: [int(i.strip()[1:]) for i in x.split(",")]
    )
    od_target_changer["Source"] = od_target_changer["Source"].apply(
        lambda x: int(str(x)[1:])
    )
    od = (
        od_target_changer.iloc[:, [0, -1]]
        .set_axis(["origin", "destination"], axis=1)
        .to_dict("records")
    )
    od = {i["origin"]: i["destination"] for i in od}
    od_2 = {s["id"] - 1000: s["targetIds"] for s in scenario.scenario.sources}
    for k, val in od_2.items():
        val = [v - 2000 for v in val]
        if k not in od:
            od[k] = val
    p = []
    col = {}
    for idx, k in enumerate(np.sort(list(od.keys()))):
        for v in od[k]:
            col[k] = idx
            p.append(dict(origin=idx, destination=v))

    p = pd.DataFrame.from_records(p)
    p["c"] = 1
    p = p.pivot_table(columns="destination", index="origin", fill_value=0)
    p = p.div(p.sum(axis=1), axis=0).droplevel(0, axis=1)
    p = p.rename(columns=col)
    col_format = [([c], FrameUtl.siunitx(precision=2)) for c in p.columns]
    FrameUtl.save_as_tex_table(
        p.reset_index(),
        run_map.path("s3_od_matrix.tex"),
        selected_only=False,
        col_format=col_format,
        str_replace=lambda x: x.replace(
            r"\num[round-precision=2]{0.0}", r"$\cdot$"
        ).replace("origin", ""),
    )
    print("hi")


def plot_test(run_map: RunMap):

    seeds = run_map["1_999_25"].mobility_seeds()
    scenario = VaderScenarioPlotHelper(
        "../study/traces_dynamic.d/mf_dyn_exp_25.out/BASIS_mf_dyn_exp_25.out.scenario"
    )
    # scenario = VaderScenarioPlotHelper("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
    _free, _covered = get_legal_cells(scenario, get_clipped_area())
    w_reds = plt.get_cmap("Reds")(np.linspace(0, 1, 256))
    w_reds[0] = [1.0, 1.0, 1.0, 1.0]
    colors = [
        (
            LinearSegmentedColormap.from_list("wReds", w_reds),
            Normalize(vmin=0, vmax=6000),
        ),
        (
            LinearSegmentedColormap.from_list("wReds", w_reds),
            Normalize(vmin=0, vmax=6000),
        ),
        (("bwr"), TwoSlopeNorm(vmin=-700, vcenter=0, vmax=700)),
    ]

    x_s, y_s = get_clipped_area()

    with run_map.pdf_page("error_location.pdf") as pdf:
        for run in ["0.9_60_25"]:  # [ k for k in run_map.keys() if k != "1_999_25"]:
            with empty_fig(
                f"Comparing number of erros for run {run}(yDist) with 1_999_25(ymf)"
            ) as f:
                pdf.savefig(f)

            for idx, seed in enumerate(seeds):
                df, mgrid = cells_with_biggest_error_diff(
                    run_map,
                    [run, "1_999_25"],
                    seed_idx=idx,
                    abs_err_filter=1.0,
                )
                print(f"run: {run}\n{df.max()}")
                gx, gy = mgrid
                col_names = df.columns
                fig, axes = plt.subplots(1, 3, figsize=(16, 9))
                h = scenario.scenario.bound["height"]
                h = np.floor(h / 5.0) * 5.0
                for idx, ax in enumerate(axes):
                    z = (
                        df.loc[pd.IndexSlice[:, :], [df.columns[idx]]]
                        .unstack("y")
                        .to_numpy()
                        .T
                    )
                    extent = (0.0, z.shape[1] * 5.0, 0.0, z.shape[0] * 5.0)
                    im = ax.imshow(
                        z,
                        origin="lower",
                        extent=extent,
                        norm=colors[idx][1],
                        cmap=colors[idx][0],
                    )
                    ax.grid(False)
                    ax.set_title(col_names[idx])
                    add_cell_patches(ax, _free)
                    scenario.add_patches(ax)
                    ax.set_xlim(x_s.start, x_s.stop)
                    ax.set_ylim(y_s.start, y_s.stop)
                    ax.set_aspect(1)
                    ax.format_coord = partial(xyz_fmt, col=df.columns[idx], df=df)
                    add_colorbar(im)
                    # print("hi")

                fig.suptitle(
                    f"Comparing locations and number of reported errors for seed '{seed}'"
                )
                fig.tight_layout()
                pdf.savefig(fig)
                plt.close(fig)


def add_colorbar(im, aspect=20, pad_fraction=0.5, **kwargs):
    """Add a vertical color bar to an image plot."""
    divider = axes_grid1.make_axes_locatable(im.axes)
    width = axes_grid1.axes_size.AxesY(im.axes, aspect=1.0 / aspect)
    pad = axes_grid1.axes_size.Fraction(pad_fraction, width)
    current_ax = plt.gca()
    cax = divider.append_axes("right", size=width, pad=pad)
    plt.sca(current_ax)
    return im.axes.figure.colorbar(im, cax=cax, **kwargs)


def xyz_fmt(x, y, col, df: pd.DataFrame):
    try:
        z = df.loc[pd.IndexSlice[np.floor(x / 5) * 5, np.floor(y / 5) * 5], col]
    except KeyError:
        z = np.Infinity

    return f"x={x:.5f} y={y:.5f} z={z:.5f}"


def plot_cell_count_err_stats2(run_map: RunMap):
    seeds = run_map["1_999_25"].seeds()

    with run_map.pdf_page("count_comparison_per_seed.pdf") as pdf:
        for idx, seed in enumerate(seeds):
            df = []
            df = cells_with_biggest_error_diff(
                run_map,
                ["0.95_80_25", "1_999_25"],
                seed_idx=idx,
                abs_err_filter=1.0,
            )

            for g in ["0.95_80_25", "1_999_25"]:
                sim: Simulation = run_map[g].simulations[idx]
                _df = sim.get_dcdMap().cell_count_measure()
                _df["lbl"] = g
                df.append(_df)

            # df = pd.concat(df, axis=0, ignore_index=True)
            cmap = plt.get_cmap("YlOrRd")
            ydist = df[0]
            ydist["lbl"] = "ydist"
            ydist = ydist[ydist["abserr"] >= 1.0]
            ymf = df[1]
            ymf["lbl"] = "ymf"
            ymf = ymf[ymf["abserr"] >= 1.0]
            fig, ax = plt.subplots(1, 2, figsize=(16, 9))
            a1: plt.Axes = ax[0]
            h = a1.hist2d("x", "y", data=ydist, bins=[84, 79], cmap=cmap, vmax=600)
            fig.colorbar(h[3], ax=a1)
            a2: plt.Axes = ax[1]
            h = a2.hist2d("x", "y", data=ymf, bins=[84, 79], cmap=cmap, vmax=600)
            fig.colorbar(h[3], ax=a2)
            adf = pd.concat(df, axis=0)
            adf["coord"] = adf["x"].astype(str) + "-" + adf["y"].astype(str)
            adf = adf.set_index(["lbl", "coord"])
            adf_c = (
                adf.groupby(["lbl", "coord"])
                .count()
                .iloc[:, [0]]
                .set_axis(["count"], axis=1)
                .unstack("lbl")
                .fillna(0)
                .droplevel(0, axis=1)
            )
            adf_c["diff_abs"] = np.abs(adf_c["ydist"] - adf_c["ymf"])
            adf_c = adf_c.sort_values("diff_abs", ascending=False)
            print("hi")


def plot_count_stats2(run_map: RunMap):
    """check difference between ydist and ymf for ONE seed
    count value is aggregated over the whole map (one value per map)
    """

    seeds = run_map["1_999_25"].seeds()

    with run_map.pdf_page("count_comparison_per_seed.pdf") as pdf:
        for idx, seed in enumerate(seeds):
            df = []
            for g in ["0.95_80_25", "1_999_25"]:
                sim: Simulation = run_map[g].simulations[idx]
                _df = (
                    sim.get_dcdMap()
                    .map_count_measure()
                    .loc[:, ["map_glb_count", "map_mean_count"]]
                )
                _df["lbl"] = g
                df.append(_df)

            df = pd.concat(df, axis=0)
            df = df.reset_index().set_index(["lbl", "simtime"]).sort_index()
            # map_glb_count is equal
            # make long form
            df = df.unstack("lbl")
            col = ["gt_del", "gt", list(df.columns)[-2][1], list(df.columns)[-1][1]]
            df = df.set_axis(col, axis=1).drop(columns=col[0]).dropna()

            fig, ax = _Plot.check_ax()
            OppAnalysis.calculate_equality_tests(df, ax=ax)
            ax.set_title(f"Stat test for seed {seed}")
            pdf.savefig(fig)
            plt.close(fig)

            fig, ax = _Plot.check_ax()
            ax.set_title("Summary Statistics for seed {seed}")
            df_desc = df.describe().applymap("{:.6f}".format).reset_index()
            ax.axis("off")
            tbl = ax.table(
                cellText=df_desc.values, colLabels=df_desc.columns, loc="center"
            )
            tbl.scale(1, 2)
            fig.tight_layout()
            pdf.savefig(fig)
            plt.close(fig)

            fig, ax = _Plot.check_ax()
            sns.lineplot(data=df)
            ax.set_title(f"Time series of count for seed {seed}")
            pdf.savefig(fig)
            plt.close(fig)

            f, ax = _Plot.check_ax()
            sns.ecdfplot(
                df,
                ax=ax,
            )
            ax.set_title(f"Count ECDF for seed {seed}")
            ax.set_xlabel("Pedestrian count")
            ax.get_legend().set_title(None)
            sns.move_legend(ax, "upper left")
            pdf.savefig(f)
            plt.close(f)

            print("hi")


def msce_comparision_paper(
    run_map: RunMap,
    compare_with="1_999_25",
    output_path="msme_example.pdf",
):
    data, _ = _get_mse_data_per_variation_ts(run_map)

    palette = sns.color_palette("colorblind", n_colors=2)
    _lbl = {
        "0.9_60_25": dict(label="S3:8 (0.9, 60)", color=palette[0], zorder=5),
        compare_with: dict(label="S3:17 (1.0, 999)", color=palette[1], zorder=5),
        "default": dict(label=None, color=(0.7, 0.7, 0.7, 1.0), zorder=3),
    }

    with plt.rc_context(_Plot.paper_rc()):
        # fig, ax1, ax2 = plt.subplot_mosaic("ab", figsize=(20,6))

        # fig, ax = plt.subplot_mosaic("aabc", figsize=(20,6))
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(20, 6))
        bbox = ax2.bbox._bbox
        x_length = bbox.xmax - bbox.x0
        bbox_new = Bbox(
            [[bbox.x0, bbox.y0 + 0.036], [bbox.x0 + 0.6 * x_length, bbox.ymax + 0.036]]
        )
        ax2.bbox._bbox = bbox_new
        for c in ["0.9_60_25", compare_with]:
            ax1.plot(data.index, data[c], **_lbl[c])
        ax1.set_xlabel("Time in seconds")
        ax1.set_ylabel("Mean Suqared Map Error (MSME)")
        ax1.legend()

        # zoom axes
        x1, x2, y1, y2 = 0.018, 0.025, 0.5, 1.0
        ax_zoom = ax2.inset_axes([1.05, 0.0, 0.7, 1.0], transform=ax2.transAxes)

        for c in data.columns:
            x = data[c].sort_values()
            y = np.linspace(0.0, 1.0, len(x))
            _kw = _lbl[c] if c in _lbl else _lbl["default"]
            ax2.plot(x, y, **_kw)
            ax_zoom.plot(x, y, **_kw)
        ax2.set_xlabel("Mean Suqared Map Error (MSME)")
        ax2.set_ylabel("ECDF")
        ax2.set_xlim(0.0, 0.025)
        ax2.legend(loc="upper left")
        ax_zoom.set_xlim(x1, x2)
        ax_zoom.set_ylim(y1, y2)
        ax_zoom.yaxis.tick_right()
        ax_zoom.tick_params(axis="y", direction="in")
        for spine in ax_zoom.spines.values():
            spine.set_edgecolor("black")
        ax2.indicate_inset_zoom(ax_zoom, edgecolor="black", alpha=1.0, linewidth=2.0)

        if output_path is not None:
            if not os.path.isabs(output_path):
                output_path = run_map.path(output_path)
            fig.tight_layout()
            fig.savefig(output_path)


def describtive_two_way_comparison_msce(
    run_map: RunMap,
    filter_run: SimGroupFilter = lambda _: True,
    compare_with="1_999_25",
    output_path="mce_stats_for_alg.pdf",
    value_axes_label="Mean Squared Cell Error (MSCE)",
):
    data, groups = _get_mse_data_per_variation_ts(run_map, filter_run, compare_with)
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
    output_path="count_stats_for_alg_x.pdf",
    value_axes_label="Pedestrian count",
):

    maps = OppAnalysis.run_get_merge_maps(
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
            with plt.rc_context(_Plot.plt_rc_same(rc={"font.size": 20})):
                OppAnalysis.plot_descriptive_comparison(
                    data.loc[:, cols],
                    lbl_dict=lbl_dict,
                    run_map=run_map,
                    out_name=output_path,
                    pdf_file=pdf_file,
                    palette=palette,
                    value_axes_label=value_axes_label,
                )


def _get_mse_data_per_variation_ts(
    run_map: RunMap,
    filter_run: SimGroupFilter = lambda _: True,
    compare_with="1_999_25",
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
    return data, groups


def _get_mse_data(run_map: RunMap):

    scenario = VaderScenarioPlotHelper(
        "../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario"
    )
    _free, _covered = get_legal_cells(scenario, get_clipped_area())

    data = OppAnalysis.run_get_msce_data(
        # run_map, hdf_path="cell_mse.h5", cell_count=84 * 79, pool_size=20
        # run_map, hdf_path="cell_mse.h5", cell_count=(52-20) * (70-17), cell_slice=get_clipped_area(), pool_size=20
        run_map,
        hdf_path="cell_mse.h5",
        cell_count=-1,
        cell_slice=_free,
        pool_size=20,
        # pool_size=1,
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

    with plt.rc_context(
        _Plot.plt_rc_same(
            rc={
                "font.size": 14,
            }
        )
    ):
        plot_mse_all(data_mean_by_run_id, run_map, styles, figure_name="cell_mse.pdf")
        plot_mse_for_seed(
            data_mean_by_run_id["cell_mse"],
            run_map,
            styles,
            figure_name="cell_mse_for_seed.pdf",
        )
        plot_mse_yDist_to_ymf(data_mean_by_run_id, run_map, "mse_diff.pdf")
        plot_mse_cell_over_time(run_map, data, "cell_mse_over_time.pdf")
        plot_count_error(run_map, "count_error_over_time.pdf")
        plot_mse_yDist_to_ymf_box_plot(
            run_map, data_mean_by_run_id, "cell_mse_box_plot.pdf"
        )
        plot_test(run_map)
        write_cell_tex(run_map)
        describtive_two_way_comparison_count(
            run_map, filter_run=lambda x: x.lbl == "0.9_60_25"
        )


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


if __name__ == "__main__":

    #  mf_dynamic_m_single_cell_iat25_6 ->  s3-001 (all seeds, fixed targetChanger, map ttl=15.0 )
    # s = SuqcStudy("/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_6")
    # s.rename_data_root("/mnt/data1tb/results/s3-001", revert=False)
    # sys.exit(0)

    run_map = RunMap.load_or_create(
        partial(
            get_run_map_N20,
            # src_path="/mnt/data1tb/results/mf_dynamic_m_single_cell_iat25_6/",
            src_path="/mnt/data1tb/results/s3-001/",
        ),
        output_path="/mnt/data1tb/results/_density_map/s3-001_dynamic/",
    )

    # create_od_matrix(run_map)
    main(run_map)
    msce_comparision_paper(run_map, output_path="msme_example2.pdf")
    print("done")
