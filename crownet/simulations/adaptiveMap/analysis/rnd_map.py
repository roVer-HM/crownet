import io
from matplotlib.patches import Patch
import pandas as pd
import numpy as np
import scipy as sp
import seaborn as sns
from scipy import stats


import matplotlib.pyplot as plt

from typing import List, Tuple
from matplotlib.ticker import (
    MultipleLocator,
    FixedLocator,
    FixedFormatter,
    LogFormatter,
)
from roveranalyzer.analysis.common import (
    RunContext,
    RunMap,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from itertools import repeat
from roveranalyzer.simulators.crownet.common.dcd_metadata import DcdMetaData
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.simulators.vadere.plots.scenario import VaderScenarioPlotHelper
from roveranalyzer.utils.dataframe import numeric_formatter, save_as_tex_table
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.utils.plot import PlotUtil, percentile, with_axis

from s1_corridor_ramp_down import (
    create_member_estimate_run_map,
    _corridor_filter_target_cells,
)


def read_dpmm_gt_from_sim_group(run_map: RunMap, sg: str):

    df = []
    for sim in run_map[sg]:
        # select ground truth data from simulation run (i.e. seed)
        _df = (
            sim.get_dcdMap()
            .count_p[pd.IndexSlice[:, :, :, 0], ["count"]]
            .reset_index(["ID"], drop=True)
        )
        _df = _df.reorder_levels(["simtime", "x", "y"])
        # build missing (time, x, y) entries containing a count of zero.
        times = _df.index.get_level_values("simtime").sort_values().unique()
        full_idx = sim.get_dcdMap().metadata.create_full_index(
            times=times, real_coords=True
        )
        diff_idx = full_idx.difference(_df.index)
        _zero = pd.DataFrame(0, index=diff_idx, columns=["count"])
        _df = pd.concat([_df, _zero])
        # add seed column and add to list
        _df["seed"] = sim.run_context.opp_seed
        _df = _df.reset_index("simtime")
        _df = _corridor_filter_target_cells(_df).reset_index()
        df.append(_df)
    return pd.concat(df, axis=0)


def histogram_cell_occupation(data: pd.DataFrame, *, alpha=0.5, ax: plt.Axes = None):
    seeds = data["seed"].unique()
    colors = sns.color_palette("colorblind", n_colors=len(seeds))
    _range = (data["count"].min(), data["count"].max())
    _bin_count = data["count"].max() + 1
    _bins = np.histogram(data["count"], bins=int(_bin_count))[1]
    for idx, (g, _df) in enumerate(data.groupby("seed")):
        ax.hist(
            _df["count"],
            bins=10,
            range=(-0.5, 9.5),
            density=True,
            histtype="step",
            color=colors[idx],
        )
        ax.vlines(_df["count"].mean(), 0, 0.6, colors=colors[idx], linestyles="dashed")
    ax.hist(
        data["count"],
        bins=10,
        range=(-0.5, 9.5),
        density=True,
        histtype="step",
        color="black",
    )
    ax.vlines(data["count"].mean(), 0, 0.6, colors="black", linestyles="dashed")

    ax.xaxis.set_major_locator(MultipleLocator(1))
    ax.yaxis.set_major_locator(MultipleLocator(0.1))
    ax.yaxis.set_minor_locator(MultipleLocator(0.1 / 2))
    ax.set_ylim(0, 0.6)
    ax.set_xlim(-0.5, 10.0)
    ax.set_xlabel("Number of agents in cell")
    ax.set_ylabel("Density")
    # color_legend(ax, colors=colors, alpha=0.3, loc="upper right")
    return ax


def build_draw_dpmm(
    run_map: RunMap, hdf: BaseHdfProvider, data: pd.DataFrame, seed_dist: pd.Series
):
    buf = io.StringIO()

    def _print(_str):
        buf.write(str(_str))
        buf.write("\n")
        print(_str)

    seeds = data["seed"].unique()
    draw = []
    for seed in seeds:
        _print(f"for mobility seed: {seed}")
        _print(data[data["seed"] == seed]["count"].describe())
        _print(stats.describe(data[data["seed"] == seed]["count"]))

        _d = data[data["seed"] == seed]
        _d = _d.set_index(["simtime", "x", "y"]).sort_index()
        seed_rv = stats.rv_discrete(
            values=(seed_dist.loc[seed].index, seed_dist.loc[seed].values)
        )
        _df_draw = draw_dpmm(seed_rv, _d, N=1000, _print=_print)
        draw.append(_df_draw)

    draw = pd.concat(draw, axis=0)
    hdf.write_frame(frame=draw, group="msme_draw")
    with open(run_map.path("draw_msme.txt"), "w") as fd:
        buf.seek(0)
        fd.write(buf.read())
    return draw


def main(run_map: RunMap):
    print("hi")

    hdf = run_map.get_hdf("dpmm_ground_truth.h5", "gt")
    if hdf.contains_group("gt"):
        data = hdf.get_dataframe("gt")
    else:
        data = read_dpmm_gt_from_sim_group(run_map, "map-500kbps")
        hdf.write_frame("gt", data)

    # global distribution over "all seeds, times and cells" -> one dist
    # glb_dist = data.groupby("count").count().iloc[:,0]
    # glb_dist = glb_dist / glb_dist.sum()
    # glb_rv = stats.rv_discrete(values=(glb_dist.index, glb_dist.values))

    # by seed: over "all times and cells" -> 20 dist

    fig, ax_m = plt.subplot_mosaic("111;222", figsize=(5, 6))
    # fig, ax = plt.subplots(1,1, figsize=(16,9))
    ax = ax_m["1"]
    histogram_cell_occupation(data, ax=ax)
    # fig.tight_layout()
    # fig.savefig(run_map.path("dpmm_occupation_histogram.pdf"))
    # plt.close(fig)
    seed_dist = data.groupby(["seed", "count"]).count().iloc[:, 0]
    seed_dist = seed_dist / seed_dist.groupby("seed").sum()

    if hdf.contains_group("msme_draw"):
        draw = hdf.get_dataframe("msme_draw")
    else:
        draw = build_draw_dpmm(hdf=hdf, run_map=run_map, data=data, seed_dist=seed_dist)

    draw = draw.reset_index()
    # fig, ax = plt.subplots(1,1, figsize=(16,9))
    ax = ax_m["2"]
    seeds = draw["seed"].unique()
    colors = sns.color_palette("colorblind", n_colors=len(seeds))
    box_data = []
    for idx, (g, _df) in enumerate(draw.groupby("seed")):
        box_data.append(_df["msme"])

    ax.hlines(
        y=np.percentile(draw["msme"], 10),
        colors="black",
        xmin=0,
        xmax=21,
        linestyles="dashdot",
    )
    ax.hlines(
        y=np.percentile(draw["msme"], 90),
        colors="black",
        xmin=0,
        xmax=21,
        linestyles="dashdot",
    )
    ax.hlines(
        y=draw["msme"].mean(), colors="black", xmin=0, xmax=21, linestyles="dashed"
    )

    b = ax.boxplot(box_data)
    PlotUtil.color_box_plot(b, fill_color=colors, ax=ax)
    ax.set_xlim(0.5, 20.5)

    # _range = (draw["msme"].min(), draw["msme"].max())
    # _bin_count = np.ceil(draw.groupby("seed").count()["msme"].mean() ** 0.5)
    # _bins = np.histogram(draw["msme"], bins=int(_bin_count))[1]
    # for idx, (g, _df) in enumerate(draw.groupby("seed")):
    #     ax.hist(
    #         _df["msme"],
    #         bins=_bins,
    #         range=_range,
    #         histtype="step",
    #         density=True,
    #         color=colors[idx],
    #         # alpha=0.3
    #     )
    #     ax.vlines(_df["msme"].mean(), 0, 65., colors=colors[idx], linestyles="dashed")
    # ax.hist(
    #     draw["msme"],
    #     bins=_bins,
    #     range=_range,
    #     density=True,
    #     histtype="step",
    #     color="black",
    # )
    # ax.vlines(draw["msme"].mean(), 0, 70., colors="black", linestyles="dashed")
    # ax.yaxis.set_major_locator(MultipleLocator(10))
    # ax.yaxis.set_minor_locator(MultipleLocator(10/2))
    # ax.set_ylabel("Density")
    # ax.set_ylim(0, 70.)

    # ax.xaxis.set_major_locator(MultipleLocator(0.025))
    # ax.xaxis.set_minor_locator(MultipleLocator(0.025/2))
    # ax.set_xlim(1.745, 1.925)
    # ax.set_xlabel("Mean squared map error (MSME)")
    # color_legend(fig, colors=colors, alpha=0.3, bbox_to_anchor=(0.4, .1), loc="upper center")

    # df = draw["msme"].describe().iloc[1:]
    # df = df.apply(lambda x: f"{x:.4f}")
    # df.columns = ["random draw"]
    # df.index.name = ""
    # t: plt.Table
    # _, _, t = PlotUtil.df_to_table(df.reset_index(), ax=ax_m["3"])
    # t.set_fontsize(14)

    fig.tight_layout()
    # fig.savefig(run_map.path("dpmm_random_msme_hist.pdf"))
    fig.savefig(run_map.path("rnd_msme.pdf"))
    print("hi")


def color_legend(ax, colors, alpha, **kwargs):
    _transparent = (1, 1, 1, 0.0)
    patches1 = [Patch(facecolor=(*c, alpha), edgecolor=(*c, alpha)) for c in colors]
    patches2 = [Patch(facecolor=_transparent, edgecolor=_transparent) for _ in colors]
    patches2[-1] = Patch(facecolor="white", edgecolor="black")
    patches = [val for pair in zip(patches1, patches2) for val in pair]
    lables = ["" for _ in patches]
    lables[-2] = "Seed 0..19"
    lables[-1] = "Over all seeds"
    ax.legend(
        handles=patches,
        labels=lables,
        # ncol=len(patches),
        ncol=len(colors) + 2,
        facecolor=_transparent,
        framealpha=0.0,
        handletextpad=0.5,
        handlelength=1.0,
        columnspacing=-0.5,
        **kwargs,
    )


def draw_dpmm(seed_rv: stats.rv_discrete, gt: pd.DataFrame, N=200, _print=print):
    msme = []
    msd = []
    for i in np.arange(N):
        _df = gt.rename(columns={"count": "gt"}).copy(deep=True)
        # draw from distribution
        _df["count"] = seed_rv.rvs(size=_df.shape[0])
        # _df["count"] = _df["gt"].mean()
        # calculate Mean squared cell error for ONE 'random measurement' thus the MSCE is
        # calcuated using N=1 --> MSCE = Suqared error
        _df["err"] = _df["count"] - _df["gt"]
        _df["sqerr"] = _df["err"] ** 2
        # MSME over time and the for the hole simulation
        _msme = _df.loc[:, ["sqerr"]].groupby(["simtime"]).mean().mean()
        _msd = _df.loc[:, ["err"]].groupby(["simtime"]).mean().mean()
        msme.append(_msme.values[0])
        msd.append(_msd.values[0])

    _print(f"MSME over N={N} meta seeds")
    msme = pd.Series(msme, name="msme", index=pd.Index(np.arange(N), name="meta_seed"))
    _print(msme.describe())
    _print(stats.describe(msme))
    msd = pd.Series(msd, name="msd", index=pd.Index(np.arange(N), name="meta_seed"))
    _print(msd.describe())
    _print(stats.describe(msd))

    df = pd.concat([msme, msd], axis=1)
    df["seed"] = gt["seed"].unique()[0]

    return df


if __name__ == "__main__":
    r2 = RunMap.load_or_create(
        create_f=create_member_estimate_run_map,
        output_path="/mnt/data1tb/results/_dyn_tx/s1_corridor_member_estimate_cmp",
        load_if_present=True,
    )
    main(r2)
