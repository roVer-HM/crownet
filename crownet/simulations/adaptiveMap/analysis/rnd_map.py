from __future__ import annotations
from functools import partial
import io
from matplotlib.patches import Patch
from matplotlib.ticker import (
    MultipleLocator,
)
import scipy.stats as stats
import seaborn as sns
from crownetutils.analysis.common import (
    RunMap,
    SimulationGroup,
)
from crownetutils.analysis.hdf.provider import (
    BaseHdfProvider,
    HdfGroupFactory,
    LazyHdfProvider,
)
from crownetutils.utils.plot import PlotUtil_, percentile, with_axis
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib

from s1_corridor_ramp_down import (
    MemberEstPlotter,
    create_member_estimate_run_map,
    _corridor_filter_target_cells,
)

from crownetutils.utils.styles import (
    load_matplotlib_style,
    STYLE_TEX,
)

load_matplotlib_style(STYLE_TEX)


def lazy_read_dpmm_gt_from_sim_group(
    run_map: RunMap, sg: str, hdf_path: str, group: str = "root"
):
    gf = HdfGroupFactory(
        group_name=group,
        factory=partial(read_dpmm_gt_from_sim_group, run_map=run_map, sg=sg),
    )
    return LazyHdfProvider(hdf_path, group, gf)


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


def histogram_cell_occupation(data: pd.DataFrame, *, alpha=0.5, ax: plt.Axes = None):
    seeds = data["seed"].unique()
    colors = sns.color_palette("colorblind", n_colors=len(seeds))
    _range = (data["count"].min(), data["count"].max())
    _bin_count = data["count"].max() + 1
    _bins = np.histogram(data["count"], bins=int(_bin_count))[1]
    s = stats.describe(data["count"])
    s2 = data["count"].describe().iloc[1:].apply("{:.4f}".format)

    def str_pad(strings):
        m_length = max([len(str(s)) for s in strings])

        def _f(s):
            pad = m_length - len(str(s))
            pad = " " * pad
            s = f"{s}:{pad} "
            return s

        return _f

    pad_f = str_pad(s2.index)
    lbl = ""
    for l, v in list(zip(s2.index, s2.values)):
        lbl += f"{pad_f(l)}{v}\n"

    lbl = lbl.replace("%", "\%")
    ax.text(
        4,
        0.1,
        lbl,
        bbox=dict(facecolor="white", alpha=1.0),
        fontdict={"family": "monospace"},
    )

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
    ax.set_xlim(-0.5, 9.5)
    ax.set_xlabel("Number of agents in cell", fontdict={"fontsize": "medium"})
    ax.set_ylabel("Density", fontdict={"fontsize": "medium"})
    # color_legend(ax, colors=colors, alpha=0.3, loc="upper right")
    return ax


class MemberEstPlotterWithRandomComparison(MemberEstPlotter):
    """Compare error (MSCE) of member estimates (no est, neighborhood, map)
    with random map density map based on cell occupation distribution collected
    via ground truth data.

    """

    def plot_msce(self):
        _hdf = BaseHdfProvider(self.run_map.path("msce_over_tp.h5"))
        box_data = []
        for g in ["ConstantRate-", "nTable-500kbps", "map-500kbps"]:
            sg: SimulationGroup = self.run_map[g]
            _df = self._get_msce_over_tp(_hdf, sg)
            df_m = _df.groupby("run_id").agg(
                ["mean", "std", percentile(25), percentile(75)]
            )
            if isinstance(df_m.columns, pd.MultiIndex):
                df_m = df_m.droplevel(0, axis=1)
            box_data.append(df_m["mean"])

        draw = get_random_draw()
        box_data.append(draw["msme"])

        rc = {
            "axes.labelsize": "xx-large",
            "xtick.labelsize": "x-large",
            "ytick.labelsize": "x-large",
        }
        colors = self.get_default_color_cycle()[0:3]
        colors = [colors[-1], colors[0], colors[1]]
        colors.append(self.get_default_color_cycle()[4])
        with plt.rc_context(rc):
            fig, _ax = plt.subplot_mosaic("a;a;a;b;b", figsize=(8.9, 8.9))
            ax = _ax["a"]
            tbl = _ax["b"]

            ax_zoom = ax.inset_axes([0.01, 0.2, 0.8, 0.78])  # TODO
            b = ax.boxplot(box_data, positions=[10, 20, 30, 40], widths=3.5)
            ax.set_xticklabels(["Constant rate", "Neighbor est.", "Map est.", "Random"])
            ax.yaxis.set_major_locator(MultipleLocator(0.2))
            ax.yaxis.set_minor_locator(MultipleLocator(0.1))
            ax.set_ylim(0.1, 2.0)
            ax.set_xlim(5, 43)
            ax.yaxis.tick_right()
            ax.set_ylabel("Mean squared map error (MSME)")
            self.color_box_plot(b, fill_color=colors, ax=ax)
            box_data = [_df.reset_index(drop=True) for _df in box_data]

            b = ax_zoom.boxplot(box_data, positions=[10, 20, 30, 40], widths=3.5)
            self.color_box_plot(b, fill_color=colors, ax=ax_zoom)
            # ax_zoom.set_ylim(0.15, 0.324)
            ax_zoom.set_ylim(0.2125, 0.345)
            # ax_zoom.yaxis.set_major_locator(MultipleLocator(0.05))
            ax_zoom.yaxis.set_major_locator(MultipleLocator(0.05 / 2))
            ax_zoom.set_xlim(6, 35)
            ax_zoom.yaxis.tick_right()
            ax_zoom.xaxis.tick_top()
            ax_zoom.xaxis.set_ticklabels(["", "", "", ""])
            ax_zoom.tick_params(axis="y", which="both", direction="in", pad=-45)
            ax_zoom.tick_params(axis="x", which="both", direction="in")
            # Create offset transform by 5 points in x direction
            dx = 0
            dy = 11 / 72.0
            offset = matplotlib.transforms.ScaledTranslation(
                dx, dy, fig.dpi_scale_trans
            )

            # apply offset transform to all x ticklabels.
            for label in ax_zoom.yaxis.get_majorticklabels():
                label.set_transform(label.get_transform() + offset)

            # sub region of the original image
            for spine in ax_zoom.spines.values():
                spine.set_edgecolor("black")

            ax.indicate_inset_zoom(ax_zoom, edgecolor="black", linewidth=2.0)

            df = pd.concat(box_data, axis=1, ignore_index=True)
            df = df.describe().iloc[1:, :]
            df = df.applymap(lambda x: f"{x:.4f}")
            df.columns = ["Constant rate", "Neighbor est.", "Map est.", "Random"]
            df.index.name = ""
            t: plt.Table
            _, _, t = self.df_to_table(
                df.reset_index(), ax=tbl, col_width=[1 / 9, 2 / 9, 2 / 9, 2 / 9, 2 / 9]
            )
            t.set_fontsize(15)
            fig.tight_layout()
            fig.savefig(self.run_map.path("member_est_msme.pdf"))


def get_random_draw():
    run_map: RunMap = get_random_draw_run_map()
    hdf = run_map.get_hdf("dpmm_ground_truth.h5", "gt")
    data = get_data()
    seed_dist = data.groupby(["seed", "count"]).count().iloc[:, 0]
    seed_dist = seed_dist / seed_dist.groupby("seed").sum()
    if hdf.contains_group("msme_draw"):
        draw = hdf.get_dataframe("msme_draw")
    else:
        draw = build_draw_dpmm(hdf=hdf, run_map=run_map, data=data, seed_dist=seed_dist)

    draw = draw.reset_index()
    return draw


def get_data():
    run_map: RunMap = get_random_draw_run_map()

    return lazy_read_dpmm_gt_from_sim_group(
        run_map=run_map,
        sg="map-500kbps",
        hdf_path=run_map.path("dpmm_ground_truth.h5"),
        group="gt",
    ).frame()


def main(run_map: RunMap):
    print("hi")

    data = get_data()

    fig, ax_m = plt.subplot_mosaic("111;222", figsize=(5, 6.5))
    ax = ax_m["1"]
    histogram_cell_occupation(data, ax=ax)

    draw = get_random_draw()
    # fig, ax = plt.subplots(1,1, figsize=(16,9))
    ax = ax_m["2"]
    seeds = draw["seed"].unique()
    colors = sns.color_palette("colorblind", n_colors=len(seeds))
    box_data = []
    for idx, (g, _df) in enumerate(draw.groupby("seed")):
        box_data.append(_df["msme"])

    ax.hlines(
        y=np.percentile(draw["msme"], 10),
        colors="red",
        xmin=-0.5,
        xmax=19.5,
        linestyles="dashdot",
    )
    ax.hlines(
        y=np.percentile(draw["msme"], 90),
        colors="red",
        xmin=-0.5,
        xmax=19.5,
        linestyles="dashdot",
    )
    ax.hlines(
        y=draw["msme"].mean(), colors="red", xmin=-0.5, xmax=19.5, linestyles="solid"
    )
    print(draw["msme"].describe())

    b = ax.boxplot(box_data, positions=range(20), flierprops={"marker": "."})
    # ax.xaxis.set_major_locator(MultipleLocator(2))
    lbls = [i if i % 2 == 0 else "" for i in range(20)]
    lbls[0] = "0"
    lbls[-1] = "19"
    ax.set_xticklabels(lbls)
    ax.set_xlim(-0.5, 19.5)
    ax.yaxis.set_major_locator(MultipleLocator(0.025))
    ax.yaxis.set_minor_locator(MultipleLocator(0.025 / 2))
    ax.set_ylim(1.725, 1.925)
    ax.set_ylabel("MSME", fontdict={"fontsize": "medium"})
    ax.set_xlabel("Simulation Seeds", fontdict={"fontsize": "medium"})

    fig.tight_layout(h_pad=2.9)
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


def get_random_draw_run_map():
    r2 = RunMap.load_or_create(
        create_f=create_member_estimate_run_map,
        # output_path="/mnt/data1tb/results/_dyn_tx/s1_corridor_member_estimate_cmp",
        output_path="/mnt/data1tb/results/arc-dsa_single_cell/study_out/s0",
        load_if_present=True,
    )
    return r2


if __name__ == "__main__":
    r2 = get_random_draw_run_map()
    # main(r2)

    r2_plotter = MemberEstPlotterWithRandomComparison(r2)
    r2_plotter.plot_msce()
