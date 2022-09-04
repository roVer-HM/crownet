from __future__ import annotations
from multiprocessing import pool
import re
import itertools
from sqlite3 import converters
from matplotlib.transforms import Affine2D
from roveranalyzer.analysis.common import (
    NamedSimulationGroupFactory,
    RunMap,
    Simulation,
    RunMap,
    SimulationGroup,
    SuqcStudy,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.utils.parallel import run_kwargs_map
from roveranalyzer.utils.plot import check_ax, matplotlib_set_latex_param
import roveranalyzer.utils.dataframe as FrameUtl
import roveranalyzer.utils.plot as PlotUtl
from matplotlib.ticker import MaxNLocator, FixedLocator
import pandas as pd
import numpy as np
import seaborn as sn
import scipy.stats as st
from scipy.stats import mannwhitneyu, kstest
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

from omnetinireader.config_parser import ObjectValue

# load global settings
matplotlib_set_latex_param()


def _min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


def _rename_merged(df: pd.DataFrame) -> pd.DataFrame:
    if any(df.loc[pd.IndexSlice[:, :, "map_glb_count"], "std"] != 0):
        raise ValueError("WARNING: global number of nodes differs between merged runs")

    df = df.rename(columns={"p_50": "median"})
    df = df.unstack("data").swaplevel(-2, -1, axis=1)
    df = df.sort_index(axis=1)
    df = df.drop(columns=[("map_glb_count", "std"), ("map_glb_count", "median")])
    df = df.rename(columns={"map_mean_count": "map_count"})
    df.columns = [f"{l1}_{l2}" for l1, l2 in df.columns]
    df = df.rename(columns={"map_glb_count_mean": "map_glb_count"})
    return df


def _prepare_collected(df: pd.DataFrame) -> pd.DataFrame:
    df = df.unstack(["data"]).droplevel(0, axis=1)
    df = df.rename(columns={"map_mean_count": "map_count_mean"})
    return df


def _process_relative_err(df: pd.DataFrame, norm_colum="map_glb_count"):

    _max = df[norm_colum].max()
    for c in df.columns:
        df[f"{c}_n"] = _min_max_norm(df[c], min=0, max=_max)

    df.columns.name = "data"
    df = df.stack().sort_index()
    return df


def _read_groups(study: SuqcStudy, run_map: RunMap, id_offset: int):
    groups: dict = {}
    pattern = re.compile(
        r"_stationary_m_base_single_cell, (?P<base>.*?)(?P<num>\d+)_(?P<run_id>\d+)_(?P<seed>\d+)$"
    )
    for sim in study.sim_iter(id_offset=id_offset):
        extends = sim.run_context.oppini["extends"]
        m = pattern.match(extends)
        m = m.groupdict()

        alg: ObjectValue = sim.run_context.ini_get("*.misc[*].app[1].app.mapCfg")
        attr = {}
        attr["alg"] = "ymf" if float(alg["alpha"]) == 1.0 else "yDist"
        attr["num"] = int(m["num"])
        attr["base"] = m["base"][:-1]

        group_name = f"{attr['alg']}_{m['base']}{m['num']}"
        group = groups.get(group_name, {})
        group["attr"] = attr

        sim_list = group.get("sim_list", [])
        sim_list.append((m["run_id"], sim))
        group["sim_list"] = sim_list
        groups[group_name] = group

    keys = list(groups.keys())
    keys.sort()
    for g_name in keys:
        group = groups[g_name]
        sim_list = group["sim_list"]
        sim_list.sort(key=lambda x: x[0])
        sim_list = [s[1] for s in sim_list]
        run_map.append_or_add(SimulationGroup(g_name, sim_list, attr=group["attr"]))

    return run_map


def run_map_with_2_step_ramp_up(output_path: str, *args, **kwargs) -> RunMap:
    """Simulation of one variation with multiple steps to reach 100% of the set
    pedestrians to see if a drop in the count accuracy persists.
    """
    run_map = RunMap(output_path)
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_5/")
    run_map = study.update_run_map(
        run_map,
        20,
        id_offset=0,
        sim_group_factory=NamedSimulationGroupFactory(["yDist", "ymf"]),
    )
    return run_map


def run_map_from_6(output_path: str, *args, **kwargs) -> RunMap:
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_6/")
    return _read_groups(study, RunMap(output_path), id_offset=0)


def run_map_from_4_8(output_path: str, *args, **kwargs) -> RunMap:
    """Simulation run of ydist and ymf heuristic with multiple numbers of pedestrians as well as constant densities."""
    run_map = RunMap(output_path)
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_8/")
    run_map = _read_groups(study, run_map, id_offset=0)

    study2 = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_4/")
    run_map = _read_groups(study2, run_map, id_offset=run_map.max_id + 1)
    return run_map


def read_map_data(run_map: RunMap):
    return OppAnalysis.merge_maps_for_run_map(
        run_map=run_map,
        data=["map_glb_count", "map_mean_count"],
        frame_consumer=FrameUtl.FrameConsumerList.get(
            _rename_merged, _process_relative_err
        ),
        hdf_path="merged_normalized_map_measure.h5",
        hdf_key="map_measure_norm_static",
        pool_size=20,
    )


def read_map_data_by_run(run_map: RunMap) -> pd.DataFrame:
    fc_list = FrameUtl.FrameConsumerList.get(_prepare_collected, _process_relative_err)
    return OppAnalysis.collect_maps_for_run_map(
        run_map=run_map,
        data=["map_glb_count", "map_mean_count"],
        frame_consumer=fc_list,
        hdf_path="normalized_map_measure.h5",
        hdf_key="map_measure_norm_static",
        pool_size=20,
    )


def get_convergence_time(map: pd.DataFrame, time_slice, err) -> dict:

    df = (
        map.loc[
            pd.IndexSlice[:, time_slice, ("map_count_mean_n", "map_glb_count_n")],
        ]
        .unstack("data")
        .copy(deep=True)
    )
    if isinstance(df.columns, pd.MultiIndex):
        df = df.droplevel(0, axis=1)
    mask = df["map_count_mean_n"] >= (df["map_glb_count_n"] - err)
    mask = mask & (df["map_count_mean_n"] <= (df["map_glb_count_n"] + err))
    df["convergence_mask"] = mask

    ret = []
    for g, _df in df.groupby(["sim"]):
        ret.append(
            [
                g,
                _df["convergence_mask"].idxmax()[-1] - time_slice.start,
                float(
                    map.loc[
                        pd.IndexSlice[g, :, "map_glb_count"],
                    ].max()
                ),
            ]
        )

    ret = pd.DataFrame(ret, columns=["sim", "convergence_time", "ped_count"]).set_index(
        "sim"
    )

    _missing: pd.Index = df.index.get_level_values(0).unique().difference(ret.index)
    if len(_missing) > 0:
        _df = (
            map.loc[_missing, df.index.get_level_values(1).max(), "map_glb_count"]
            .reset_index(["simtime", "data"], drop=True)
            .set_axis(["ped_count"], axis=1)
        )
        _df["convergence_time"] = float("inf")
        ret = pd.concat([ret, _df], axis=0)

    return ret


def plot_convergence(
    run_map: RunMap,
    convergence_error=0.05,
    event_time: float = 40.0,
    ax: plt.Axes | None = None,
):

    df = read_map_data_by_run(run_map)
    time_slice = slice(event_time, None, None)
    # calculate convergence time step for each simulation (sim, run)
    df = df.unstack("data").droplevel(0, axis=1).loc[:, :, time_slice]
    df["abs_err"] = np.abs(df["map_glb_count_n"] - df["map_count_mean_n"])
    df["conv_mask"] = df["abs_err"] < convergence_error
    df["convergence_time"] = df.index.get_level_values("simtime") - event_time
    # convergence is reached at first time point where ["conv_mask"] is true --> idxmax()
    # Note if ["conv_mask"] is never true, i.e. convergence is not reached the first index
    # for that group is used.
    conv_idx = pd.MultiIndex.from_tuples(
        df.groupby(["sim", "run"])["conv_mask"].idxmax()
    )
    df = df.loc[conv_idx]
    # set convergence time for runs that did not reach convergence to -1
    df.loc[~df["conv_mask"], ["convergence_time"]] = -1.0
    # add attribute
    _attr = run_map.attr_df()
    _attr["area"] = _attr["base"].apply(lambda x: x.split("_")[-1])
    _attr["area"] = _attr["area"].str.replace("position", "full")
    _attr["area"] = _attr["area"].str.replace("full", "A1")
    _attr["area"] = _attr["area"].str.replace("quarter", "A2")
    df = df.join(_attr, on="sim").reset_index().sort_values("num")
    # add count of same convergence time for each group
    ct_count = df.groupby(["num", "alg", "convergence_time"]).count().iloc[:, 0]
    ct_count.name = "count"
    df = df.join(ct_count, on=["num", "alg", "convergence_time"])
    df = df.reset_index()
    # create small x-axis offset to prevent overlapping data
    df["ped_count"] = df["num"]
    df = df.set_index(["num", "alg", "run"]).sort_index()
    df.loc[pd.IndexSlice[:, "ymf", :], ["ped_count"]] += 0.3
    df.loc[pd.IndexSlice[:, "yDist", :], ["ped_count"]] -= 0.3

    # create scatter plot
    df = df.rename(
        columns=dict(
            ped_count="Number of pedestrians", convergence_time="Convergence time"
        )
    )
    fig, ax = check_ax(ax)
    ax = sn.scatterplot(
        ax=ax,
        data=df.reset_index(),
        x="Number of pedestrians",
        y="Convergence time",
        hue="alg",
        style="area",
        size="count",
    )

    # create x axis tick labels based on data
    l = np.sort(df.index.get_level_values("num").unique()).astype(int)
    ax.xaxis.set_major_locator(FixedLocator(l))
    ax.set_xticklabels(l, rotation=90, ha="center")
    # fix legend layout and position
    h, l = ax.get_legend_handles_labels()
    _del_idx = [l.index(_l) for _l in ["alg", "count", "area"]]
    for idx in _del_idx[::-1]:
        del h[idx]
        del l[idx]
    fig.legend(h, l, loc="lower center", ncol=len(h), fontsize="x-small")
    ax.get_legend().remove()
    # save figure
    ax.set_title("Convergence time for mean density map for each simulation run")
    ax.get_figure().tight_layout()
    fig.savefig(run_map.path("convergence_time_per_run.pdf"))


def main(run_map: RunMap):

    map = read_map_data(run_map).sort_index()

    # get convergence time
    df_convergence = get_convergence_time(map, time_slice=slice(51, None), err=0.05)
    df_convergence.to_csv(run_map.path("convergence_time.csv"))

    # plot
    plot_all_relative_pedestrian_count_ts(map)
    plot_merged_relative_pedestrian_count_ts(map, run_map)
    # plot_positions(run_map)
    plot_all_absolute_pedestrian_count_ts(map)
    plot_convergence(run_map, event_time=51.0, convergence_error=0.05)
    plot_relative_count_stat(run_map)


def plot_all_absolute_pedestrian_count_ts(map: pd.DataFrame):
    with PdfPages(run_map.path("absolute_pedestrian_count_merged.pdf")) as pdf:
        for group, df in map.groupby(by=["sim"]):
            fig, axes = plt.subplots(3, 1, figsize=(16, 3 * 9))
            for ax, lbl_filter in zip(axes, ["position", "full", "quarter"]):
                # ax.set_ylim(0.0, 1.1)
                ax.set_ylabel("number of pedestrians")
                ax.set_xlabel("time in [s]")
                ax.set_xlim(0, 100)
                ax.xaxis.set_major_locator(MaxNLocator(10))
                ax.yaxis.set_major_locator(MaxNLocator(11))
                if lbl_filter in ["position", "full"]:
                    ax.set_title("Number of pedestrians over time in Area [415x394]")
                else:
                    ax.set_title("Number of pedestrians over time in Area [207x196]")

                _glb = df.loc[:, :, "map_glb_count"]
                ax.plot(
                    _glb.index.get_level_values("simtime"),
                    _glb,
                    label="ground_truth",
                    color="black",
                )
                _df = df.loc[pd.IndexSlice[:, :, "map_count_mean"]]
                ax.plot(_df.index.get_level_values("simtime"), _df, label=group)
                ax.legend(loc="center right")
            pdf.savefig(fig)
            plt.close(fig)


def plot_all_relative_pedestrian_count_ts(map: pd.DataFrame):

    # create figures
    fig, axes = plt.subplots(3, 1, figsize=(16, 3 * 9))

    for ax, lbl_filter in zip(axes, ["position", "full", "quarter"]):
        ax.set_ylim(0.0, 1.1)
        ax.set_ylabel("norm. number of pedestrians")
        ax.set_xlabel("time in [s]")
        ax.set_xlim(0, 100)
        ax.xaxis.set_major_locator(MaxNLocator(10))
        ax.yaxis.set_major_locator(MaxNLocator(11))
        if lbl_filter in ["position", "full"]:
            ax.set_title("Normalized number of pedestrians over time in Area [415x394]")
        else:
            ax.set_title("Normalized number of pedestrians over time in Area [207x196]")

        _first_run = map.index.get_level_values(0)[0]
        _glb = map.loc[pd.IndexSlice[_first_run, :, "map_glb_count_n"]]
        ax.plot(
            _glb.index.get_level_values("simtime"),
            _glb,
            label="ground_truth",
            color="black",
        )
        for sim in map.index.get_level_values("sim").unique():
            if lbl_filter in sim:
                _df = map.loc[pd.IndexSlice[sim, :, "map_count_mean_n"]]
                ax.plot(_df.index, _df, label=sim)
        ax.legend(loc="center right")

    fig.tight_layout()
    fig.savefig(run_map.path("normalized_pedestrian_count.pdf"))


def plot_positions(run_map: RunMap):

    args = [dict(sim_group=val) for val in run_map.values()]
    position_data = run_kwargs_map(
        func=OppAnalysis.merge_position,
        kwargs_iter=args,
        append_args=True,
    )

    figures = []
    for pos_df, kwargs in position_data:
        sim_group: RunMap = kwargs["sim_group"]
        rep_list = sim_group.reps
        lbl = sim_group.group_name
        sim: Simulation = sim_group[0]  # for offset only. Is equal for all rep's
        _, bound = sim.sql.get_sim_offset_and_bound()
        bound_poly = sim.sql.get_bound_polygon().exterior.xy
        fig, axes = plt.subplots(4, 5, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle(f"Position for scenario {lbl} with N=20 seeds.", fontsize=16)
        axes = list(itertools.chain(*axes))
        for i in range(len(rep_list), len(axes)):
            fig.delaxes(axes[i])
        for idx, run_id in enumerate(rep_list):
            ax: plt.Axes = axes[idx]
            ax.set_title(f"Run N={idx}")
            ax.set_aspect("equal", adjustable="box")
            ax.set_xlim(-10, bound[0] + 10)
            ax.set_ylim(-10, bound[1] + 10)
            ax.plot(
                "x",
                "y",
                # fmt="",
                label="nodes stationary for total simulation",
                data=pos_df.loc[run_id, :, :, True],
                markersize=3,
                marker="o",
                linestyle="none",
                color="red",
            )
            ax.plot(
                "x",
                "y",
                label="nodes removed after t=40s",
                data=pos_df.loc[run_id, :, :, False],
                markersize=3,
                marker="o",
                linestyle="none",
                color="blue",
            )
            ax.plot(bound_poly[0], bound_poly[1], color="red")
        fig.legend(
            *axes[0].get_legend_handles_labels(),
            loc="lower center",
            ncol=2,
            bbox_to_anchor=(0.5, 0),
            bbox_transform=fig.transFigure,
        )
        fig.tight_layout(rect=(0, 0.05, 1.0, 1.0))
        figures.append(fig)
        print(f"done with {lbl}")

    with PdfPages(run_map.path("postion.pdf")) as pdf:
        for f in figures:
            pdf.savefig(f)


def split_sim_key(data: pd.DataFrame, keep_index: bool = False):
    """Split concatenated string key to separate entities"""
    mask_ydist = data.index.get_level_values("sim").str.startswith("yDist")
    mask_ymf = data.index.get_level_values("sim").str.startswith("ymf")
    mask_full = data.index.get_level_values("sim").str.contains("full")
    mask_quarter = data.index.get_level_values("sim").str.contains("quarter")
    mask_position = data.index.get_level_values("sim").str.contains("position")
    data["sim_group"] = ""
    data["sim_size"] = ""
    data.loc[mask_ydist, ["sim_group"]] = "yDist"
    data.loc[mask_ymf, ["sim_group"]] = "ymf"
    data.loc[mask_full, ["sim_size"]] = "full"
    data.loc[mask_quarter, ["sim_size"]] = "quarter"
    data.loc[mask_position, ["sim_size"]] = "position"
    if not keep_index:
        data = data.reset_index("sim", drop=True)
    return data


def merge_relative_with_error_bars(run_map: RunMap, data=None):
    data: pd.DataFrame = read_map_data(run_map).sort_index() if data is None else data
    _glb = data.loc[data.index.get_level_values("sim")[0], :, "map_glb_count_n"]
    data = data.loc[pd.IndexSlice[:, :, "map_count_mean_n"], :].copy()
    # add group discriminator
    data = split_sim_key(data)
    data = data.reset_index().set_index(["sim_group", "sim_size", "simtime"])
    ret = data.groupby(["sim_group", "sim_size", "simtime"]).agg(
        ["mean", "std", "sem", "count"]
    )
    ret.columns = ret.columns.droplevel(0)
    clm_l, clm_h = st.t.interval(
        0.95, df=ret["count"] - 1, loc=ret["mean"], scale=ret["sem"]
    )
    ret["clm_low"] = clm_l
    ret["clm_high"] = clm_h
    ret["clm_olow"] = ret["mean"] - clm_l
    ret["clm_ohigh"] = clm_h - ret["mean"]

    ret = ret.join(_glb)
    ret = ret.rename(columns={0: "glb"})

    stat = []
    for sim_size in ret.index.get_level_values("sim_size").unique():
        stat_df = (
            ret.loc[pd.IndexSlice[["yDist", "ymf"], sim_size, :], ["mean"]]
            .unstack(["sim_size", "sim_group"])
            .droplevel([0, 1], axis=1)
            .reset_index(drop=True)
        )
        t = mannwhitneyu(stat_df.iloc[:, 0], stat_df.iloc[:, 1])
        stat.append([t[0], t[1], f"{sim_size}-ydist/ymf"])
    stat = pd.DataFrame(np.array(stat), columns=["stat", "p-value", "name"])

    return ret, data, stat


def plot_merged_relative_pedestrian_count_ts(data: pd.DataFrame, run_map: RunMap):

    ret, data, stat = merge_relative_with_error_bars(run_map, data)

    p = [
        ("Normalized  pedestrian count +/- Std Dev", "std"),
        # ("Mean +/- SEM", "sem"),
        # ("Mean with 95% confidence interval", ["clm_olow", "clm_ohigh"]),
    ]
    # normalized mean only
    def ax_trans(ax: plt.Axes, num=2, offset=0.2):
        """Transformations which creates distributes 'num' items 'offset' amount apparat around zero."""
        _start = 0.0 - float((num - 1) * offset) / 2
        _stop = _start + num * offset
        return [
            Affine2D().translate(x, 0.0) + ax.transData
            for x in np.arange(_start, _stop, offset)
        ]

    with PdfPages(run_map.path("normalized_pedestrian_count_merged.pdf")) as pdf:
        for ylabel, err in p:
            fig, ax = plt.subplots(1, 1, figsize=(16, 9))
            id_sel = list(itertools.product(["full", "quarter"], ["ymf", "yDist"]))
            _glb = ret.loc[pd.IndexSlice[id_sel[0][1], id_sel[0][0], :], "glb"]
            ax.plot(
                _glb.index.get_level_values("simtime"),
                _glb,
                drawstyle="steps-post",
                label="Ground Truth",
                color="black",
            )
            ax.set_title(
                f"Normalized number of pedestrians over time in areas A1=[415x394] A2=[207x196]"
            )
            ax.set_ylim(0.4, 1.1)
            ax.set_xlim(0, 100)
            ax.set_ylabel(ylabel)
            ax.set_xlabel("Time in seconds")
            ax.xaxis.set_major_locator(MaxNLocator(10))
            # data
            translate = ax_trans(ax, len(id_sel))
            for idx, (group, sim) in enumerate(id_sel):
                _df = ret.loc[sim, group, :]
                yerr = _df[err].to_numpy().T if isinstance(err, list) else _df[err]
                ax.errorbar(
                    _df.index.get_level_values("simtime"),
                    _df["mean"],
                    transform=translate[idx],
                    yerr=yerr,
                    fmt="-" if idx < 2 else "-.",
                    linewidth=1,
                    elinewidth=1,
                    capsize=2,
                    capthick=1,
                    label=f"S2-{'A1' if group=='full' else 'A2'}-{sim}",
                )
                ax.legend()
            stat = stat[["name", "p-value"]]
            stat = stat[~stat["name"].str.contains("position")]
            stat["name"] = stat["name"].apply(
                lambda x: x.replace("full", "S2-A1")
                if "full" in x
                else x.replace("quarter", "S2-A2")
            )
            stat.columns = ["Mann-Whitney U", "p-value"]
            stat = FrameUtl.format_frame(
                stat, si_func={"p-value": FrameUtl.siunitx(precision=3)}
            )

            tbl = ax.table(
                cellText=stat.values,
                cellLoc="center",
                colLabels=stat.columns,
                bbox=(0.5, 0.25, 0.5, 0.2),
            )
            tbl.scale(1, 2)
            tbl.set_zorder(999)  # on top
            fig.tight_layout()
            pdf.savefig(fig)


def plot_count_stats(run_map: RunMap):
    maps = read_map_data(run_map)
    groups = [
        g.group_name for g in run_map.values() if g.attr["base"] == "density_full"
    ]
    groups.sort(key=lambda x: int(run_map.attr(x, "num")))
    data = maps.loc[
        pd.IndexSlice[groups, :, ["map_count_mean", "map_glb_count"]]
    ].reset_index()
    data["alg"] = data["sim"].apply(lambda x: run_map.attr(x, "alg"))
    data["num"] = data["sim"].apply(lambda x: int(run_map.attr(x, "num")))
    lbl = dict(map_count_mean="count", map_glb_count="gt")
    data["data"] = data["data"].apply(lambda x: lbl[x])
    data["data"] = data["data"] + "_" + data["alg"]
    data = data[data["data"] != "gt_ymf"]
    data["data"] = data["data"].str.replace("gt_ydist", "gt")
    data = data.drop(columns=["sim", "alg"])
    data = (
        data.set_index(["num", "simtime", "data"])
        .sort_index()
        .unstack("data")
        .droplevel(0, axis=1)
    )
    data = data.loc[:, ["gt", "count_ydist", "count_ymf"]]

    for g, _df in data.groupby("num"):
        print(g)
        OppAnalysis.plot_descriptive_comparison(
            _df.copy(deep=True).reset_index("num", drop=True),
            lbl_dict={},
            run_map=run_map,
            out_name=f"peds_{g}_dist_plots.pdf",
        )


def plot_relative_count_stat(run_map: RunMap):
    maps = read_map_data(run_map)
    # select full size simulations only
    groups = [
        g.group_name for g in run_map.values() if g.attr["base"] == "density_full"
    ]

    # transform maps frame to into long format  [simtime](gt, ....)
    gt = (
        maps.loc[pd.IndexSlice[groups[0], :, ["map_glb_count_n"]]]
        .reset_index()
        .drop(columns=["sim"])
    )
    gt["data"] = "gt"
    gt = gt.set_index(["simtime", "data"])
    data = maps.loc[pd.IndexSlice[groups, :, ["map_count_mean_n"]]].reset_index()
    data["alg"] = data["sim"].apply(lambda x: run_map.attr(x, "alg"))
    data["data"] = (
        "count_"
        + data["alg"]
        + "_"
        + data["sim"].apply(lambda x: str(run_map.attr(x, "num")))
    )
    data = data[data["data"] != "gt_ymf"]
    data["data"] = data["data"].str.replace("gt_yDist", "gt")
    data = data.drop(columns=["sim", "alg"])
    data = data.set_index(["simtime", "data"]).sort_index()
    data = pd.concat([gt, data])
    data = data.unstack("data").droplevel(0, axis=1)

    # prepare statistic combinations (only match with ground truth and with algorithms)
    nums = np.unique(np.sort([int(run_map.attr(g, "num")) for g in groups]))
    stat_combination = []
    for num in nums:
        stat_combination.append(("gt", f"count_yDist_{num}"))
        stat_combination.append(("gt", f"count_ymf_{num}"))
    alg_combi = [(f"count_yDist_{num}", f"count_ymf_{num}") for num in nums]
    stat_combination.extend(alg_combi)

    # sort columns (simtime)[gt, ydist_A, ymf_A, ydist_B, ...]
    data = data.loc[:, ["gt", *list(itertools.chain(*alg_combi))]]

    # simplify label names
    lbl_dict = {c: c.replace("count_", "") for c in data.columns[1:]}
    OppAnalysis.plot_descriptive_comparison(
        data,
        lbl_dict=lbl_dict,
        stat_col_combination=stat_combination,
        run_map=run_map,
        out_name=f"normalized_ped_count_dist_plots.pdf",
    )


def update_hdf_files():
    # study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_3/")
    # study = SuqcRun("/mnt/data1tb/results/mf_1d_1/")
    pass
    # for sim in study.get_simulations():
    #     _b = sim.builder
    #     group_version = _b.count_p.get_attribute(attr_key="group_version", group="cell_measure", default=1)
    #     if group_version < 2:
    #         print(f"update cell_measure for {sim.data_root}")
    #         new_cell_metric = sim.get_dcdMap().cell_count_measure(load_cached_version=False)
    #         _b.count_p.override_frame(group="cell_measure", frame=new_cell_metric)
    #         _b.count_p.repack_hdf(keep_old_file=False)
    #         _b.count_p.set_attribute(attr_key="group_version", value=2, group="cell_measure")
    #     else:
    #         print(f"group is up to date. No changes needed for {sim.data_root}")


if __name__ == "__main__":

    # run_map: RunMap = RunMap.load_or_create(
    #     create_f=run_map_from_4_8,
    #     output_path="/mnt/data1tb/results/_density_map/02_stationary_output/",
    # )

    # run_map: RunMap = RunMap.load_or_create(
    #     create_f=run_map_with_2_step_ramp_up,
    #     output_path="/mnt/data1tb/results/mf_stationary_m_single_cell_5/",
    # )

    run_map: RunMap = RunMap.load_or_create(
        create_f=run_map_from_6,
        output_path="/mnt/data1tb/results/_density_map/02a_stationary_output/",
    )

    plot_convergence(run_map, event_time=51.0, convergence_error=0.05)
    # main(run_map)
    # plot_relative_count_stat(run_map)
    # plot_merged_relative_pedestrian_count_ts(None, run_map)
    print("done")
