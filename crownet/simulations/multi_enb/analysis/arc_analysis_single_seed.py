from dataclasses import dataclass
from itertools import product
from functools import partial
import os
import shutil
from typing import Any, List
from crownetutils.analysis.plot.rsd_grid_plots import (
    RsdGridPlotter,
    RunMapRsdGridPlotStyler,
)
from crownetutils.utils.styles import load_matplotlib_style
from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.offsetbox import AnnotationBbox, TextArea
from matplotlib.patches import BoxStyle
import numpy as np
from crownetutils.analysis.common import RunMap, Simulation, SimulationGroup, SuqcStudy

from crownetutils.utils.plot import (
    GridPlot,
    GridPlotIter,
    MulticolorPatch,
    MulticolorPatchHandler,
    PlotUtil,
    combine_images,
)
from crownetutils.utils.logging import logger, timing

from matplotlib.ticker import (
    FixedFormatter,
    FixedLocator,
    MultipleLocator,
    ScalarFormatter,
)

load_matplotlib_style(os.path.join(os.path.dirname(__file__), "paper_tex.mplstyle"))


class FilteredSeedGroupFactory:

    """Only select specific seeds from the provided SqucStudy """

    def __init__(self, seeds: List[int], sim_per_group=20, offset=0) -> None:
        self.seed = seeds
        self.sim_per_group = sim_per_group
        self.offset = offset

    def __call__(
        self, sim: Simulation, data: List[Simulation], **kwds: Any
    ) -> SimulationGroup:
        attr = kwds.get("attr", {})

        data_filtered: List[Simulation] = list(np.array(data)[self.seed])
        sim: Simulation = data_filtered[0]
        entropy_cfg = sim.run_context.ini_get("*.misc[*].app[2].app.mapCfg")
        ttl = int(entropy_cfg.as_dict["cellAgeTTL"].replace(".0s", "").replace("s", ""))
        order = entropy_cfg.as_dict["idStreamType"]
        sg = SimulationGroup(
            group_name=f"{order}_ttl{ttl}", data=data_filtered, attr=attr
        )
        return sg


@dataclass
class SeedLocation:

    seed_list: List[int]  # list of seeds to use for that study
    study_dir: str  # path to suqc directory
    offset: int = 0  # offset applied if multiple studies are combined
    group_size: int = 20  # number of seeds


def build_run_map(output_path, seed_location_list: List[SeedLocation]):

    r = RunMap(output_path)
    for seed_location in seed_location_list:
        f = FilteredSeedGroupFactory(seed_location.seed_list)
        study = SuqcStudy(base_path=seed_location.study_dir)
        study.update_run_map(
            run_map=r,
            sim_per_group=seed_location.group_size,
            id_offset=seed_location.offset,
            sim_group_factory=f,
        )

    return r


@timing
def plot_rsd_overview_grids(data_root, analysis_out, seed_list, all=True):
    seed_location_list = ([SeedLocation(seed_list, data_root)],)
    run_map = RunMap.load_or_create(
        create_f=partial(build_run_map, seed_location_list=seed_location_list),
        output_path=analysis_out,
    )

    # just any Simulation will do. It will only grab the enb color map
    if all:
        plotter = RsdGridPlotter.from_sim(sim=run_map[0][0])  # all
    else:
        plotter = RsdGridPlotter.from_sim(
            sim=run_map[0][0],
            rows=1,
            cols=5,
            rsd_filter=[6, 7, 8, 9, 10],
            single_fig_width=5.0,
        )

    parameters = [
        "orderByDistanceAscending_ttl15",
        "orderByDistanceDescending_ttl15",
        "insertionOrder_ttl15",
        "insertionOrder_ttl60",
        "orderByDistanceAscending_ttl60",
        "orderByDistanceDescending_ttl60",
        "orderByDistanceAscending_ttl3600",
        "orderByDistanceDescending_ttl3600",
        "insertionOrder_ttl3600",
    ]

    seed = 0
    for p in parameters:
        logger.info(f"process {p}")
        # plotter.plot_rsd_size_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_serving_enb_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_throughput_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_packet_delay_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_burst_information_ratio_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_map_age_over_distance_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_map_number_measures_over_distance_grid(
        #     run_map, parameter=p, seed=seed
        # )
        # plotter.plot_map_size_over_distance_grid(run_map, parameter=p, seed=seed)
        plotter.plot_map_size_over_time(run_map, parameter=p, seed=seed)

    combine_figures(analysis_dir=analysis_out)


@timing
def combine_figures(analysis_dir):

    order_var = [
        "orderByDistanceAscending",
        "orderByDistanceDescending",
        "insertionOrder",
    ]
    ttl = ["ttl15", "ttl60", "ttl3600"]

    figure_template = [
        "{base}/{order}/rsd_size_grid_{order}_{ttl}_0.png",
        "{base}/{order}/enb_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_throughput_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_delay_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_information_transfer_grid_{order}_{ttl}_0.png",
        "{base}/{order}/age_of_distance_{order}_{ttl}_0.png",
        "{base}/{order}/num_measures_over_distance_{order}_{ttl}_0.png",
        "{base}/{order}/num_cells_over_distance_{order}_{ttl}_0.png",
        "{base}/{order}/map_size_over_distance_{order}_{ttl}_0.png",
    ]

    for o1, ttl in product(order_var, ttl):
        figures = [
            f.format(base=analysis_dir, order=o1, ttl=ttl) for f in figure_template
        ]
        out = os.path.join(analysis_dir, f"combined_{o1}_{ttl}.png")
        combine_images(
            images=[{"input": figures, "output_path": out}], direction="vertical"
        )


def figure_builder(colors):
    # fig_size = (single_fig_width * cols, 9 / 16 * single_fig_width * rows)
    fig_size = PlotUtil.fig_size_mm(width=178, height=70)
    # get list of colors based on list of rsd's
    grid_figure = GridPlot(
        rows=3, columns=5, colors=colors, figsize=fig_size, sharex=True, sharey=True
    )
    grid_figure_iter: GridPlotIter = grid_figure.iter_lowerLeftOrig()
    return grid_figure_iter


def rsd_name(txt, color, xy, xybox, bbox_align, coord_transform):
    text = TextArea(
        txt,
        textprops=dict(
            size="x-small",
            horizontalalignment="center",
            verticalalignment="baseline",
            color=color,
        ),
    )
    abox = AnnotationBbox(
        offsetbox=text,
        xy=xy,
        xybox=xybox,
        boxcoords=coord_transform,
        box_alignment=bbox_align,
        frameon=False,
        bboxprops=dict(boxstyle=BoxStyle("Round", pad=0.06)),
        zorder=10,
    )
    return abox


def set_time_axis(fig: plt.Figure, ax_iter: GridPlotIter):

    for i, (fig, ax, color) in enumerate(ax_iter):
        ax: plt.Axes
        ax.xaxis.set_major_locator(MultipleLocator(400))
        ax.xaxis.set_minor_locator(MultipleLocator(50))
        ax.set_xlim(0, 1000)

    for ax in ax_iter.lower_axes():
        PlotUtil.move_last_x_ticklabel_left(ax, "1000")
        ax.set_xlabel("")

    # use any ax coordinates are base on figure
    ax_iter[0].text(
        x=0.5,
        y=0.01,
        s="Simulaton time in seconds",
        transform=fig.transFigure,
        fontdict=dict(size="medium"),
        ha="center",
    )


def save_old(fig, path):
    """Save figure but keep old version"""

    os.makedirs(os.path.dirname(path), exist_ok=True)
    try:
        shutil.copyfile(path, f"{path}.old")
    except:
        pass
    fig.savefig(path)


def plot_rsd_size_grid_for_paper(fig: plt.Figure, ax_iter: GridPlotIter, path):

    set_time_axis(fig, ax_iter)

    colors = []
    for i, (fig, ax, color) in enumerate(ax_iter):
        colors.append(color)
        ax: plt.Axes
        ax.get_legend().remove()
        ax.set_title(None)
        ax.set_ylim((0, 200))
        ax.yaxis.set_major_locator(MultipleLocator(100))
        ax.yaxis.set_minor_locator(MultipleLocator(20))

        ax.vlines(120, 0, 160, color="darkred", linestyles="dashed")

        a = rsd_name(
            f"$e\mathit{{NB}}_{{{i+1}}}$",
            color=color,
            xy=(0.03, 0.96),
            xybox=(0.03, 0.96),
            bbox_align=(0, 1),
            coord_transform=ax.transAxes,
        )
        ax.add_artist(a)

    for idx, ax in enumerate(ax_iter.left_axes()):
        PlotUtil.move_last_y_ticklabel_down(ax, "200")
        t = "Number of nodes" if idx == 1 else ""
        ax.set_ylabel(t)

    fig.subplots_adjust(
        left=0.09, bottom=0.185, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )
    fig.legend(
        [MulticolorPatch(colors)],
        ["Mean node count\nwith Q1/3"],
        handler_map={MulticolorPatch: MulticolorPatchHandler()},
        loc="lower right",
        bbox_to_anchor=(1.015, -0.0515),
        frameon=False,
        fontsize="x-small",
        handlelength=3.0,
        labelspacing=0.1,
    )
    l2 = plt.legend(
        [
            Line2D([0], [0], color="darkred", linestyle="dashed"),
            Line2D([0], [0], color="black"),
        ],
        ["Stop spawn after 120 seconds", "Total node count"],
        loc="lower left",
        bbox_to_anchor=(0.0, -0.05),
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
    )
    # l2.set_transform(fig.transFigure)
    fig.add_artist(l2)
    save_old(fig, path)


class EnbTxtPos:
    @classmethod
    def top_left(cls):
        return cls((0.03, 0.96), (0, 1))

    @classmethod
    def bottom_right(cls):
        return cls((0.985, 0.03), (1, 0))

    @classmethod
    def right(cls, y):
        return cls((0.985, y), (1, 0))

    def __init__(self, xy, box_algin) -> None:
        self.xy = xy
        self.box_algin = box_algin


def plot_throughput_grid_for_paper(fig: plt.Figure, ax_iter: GridPlotIter, path):

    set_time_axis(fig, ax_iter)

    _r = EnbTxtPos.right(0.6)
    colors = []
    enb_txt_loc: List[EnbTxtPos] = [
        _r,
        _r,
        EnbTxtPos.bottom_right(),
        _r,
        _r,
        EnbTxtPos.bottom_right(),
        EnbTxtPos.bottom_right(),
        EnbTxtPos.bottom_right(),
        EnbTxtPos.bottom_right(),
        EnbTxtPos.bottom_right(),
        _r,
        EnbTxtPos.bottom_right(),
        _r,
        _r,
        _r,
    ]
    for i, (fig, ax, color) in enumerate(ax_iter):
        colors.append(color)
        ax: plt.Axes
        if ax.get_legend() is not None:
            ax.get_legend().remove()
        ax.set_title(None)
        ax.set_ylim((0, 150))
        ax.yaxis.set_major_locator(MultipleLocator(50))
        ax.yaxis.set_minor_locator(MultipleLocator(10))

        a = rsd_name(
            f"$e\mathit{{NB}}{{{i+1}}}$",
            color="black",
            xy=enb_txt_loc[i].xy,
            xybox=enb_txt_loc[i].xy,
            bbox_align=enb_txt_loc[i].box_algin,
            coord_transform=ax.transAxes,
        )
        ax.add_artist(a)

    for idx, ax in enumerate(ax_iter.left_axes()):
        PlotUtil.move_last_y_ticklabel_down(ax, "150")
        t = "Throughput in kB/s" if idx == 1 else ""
        ax.set_ylabel(t)

    l2 = plt.legend(
        [Line2D([0], [0], color="darkred", linestyle="dashed")],
        [
            "Configured shared\nbandwidth of 125 kB/s",
        ],
        loc="lower left",
        bbox_to_anchor=(0.0, -0.05),
        bbox_transform=fig.transFigure,
        frameon=False,
        fontsize="x-small",
        labelspacing=0.1,
    )
    # l2.set_transform(fig.transFigure)
    fig.add_artist(l2)

    fig.subplots_adjust(
        left=0.09, bottom=0.185, right=0.995, top=0.99, wspace=0.08, hspace=0.15
    )
    save_old(fig, path)


def plot_single_seed_data_for_paper():

    seed_location_list = [
        SeedLocation(
            [1], "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/"
        ),
        SeedLocation([5], "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/"),
    ]
    output_path = (
        "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/paper_plots"
    )
    run_map = RunMap.load_or_create(
        create_f=partial(build_run_map, seed_location_list=seed_location_list),
        output_path=output_path,
    )
    plotter = RsdGridPlotter.from_sim(sim=run_map[0][0])  # all
    color_list = [plotter.color_map[c] for c in plotter.get_rsd_list()]
    plotter.create_figure = partial(figure_builder, colors=color_list)
    sim_group: SimulationGroup = run_map[0]

    ax_iter: GridPlotIter
    fig: plt.Figure

    fig, ax_iter = plotter.plot_throughput_grid(
        **RunMapRsdGridPlotStyler.plot_kwargs(run_map, sim_group.group_name, 1)
    )
    plot_throughput_grid_for_paper(
        fig, ax_iter, run_map.path("throughput_over_time.pdf")
    )

    fig, ax_iter = plotter.plot_rsd_size_grid(
        **RunMapRsdGridPlotStyler.plot_kwargs(run_map, sim_group.group_name, 1)
    )
    plot_rsd_size_grid_for_paper(fig, ax_iter, run_map.path("map_size_over_time.pdf"))


if __name__ == "__main__":
    plot_single_seed_data_for_paper()
