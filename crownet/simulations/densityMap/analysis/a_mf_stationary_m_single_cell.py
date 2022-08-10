from cProfile import label
import os
import re
import itertools
from itertools import product
from multiprocessing import get_context
from statistics import mean
from itertools import repeat
from tokenize import group
from typing import List
from matplotlib import markers
from roveranalyzer.analysis.common import (
    RunContext,
    RunMap,
    Simulation,
    RunMap,
    SuqcStudy,
)
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D, percentile
from roveranalyzer.simulators.opp.scave import CrownetSql
from roveranalyzer.utils.general import Project
from roveranalyzer.utils.parallel import run_args_map, run_kwargs_map
from roveranalyzer.utils.plot import check_ax
from matplotlib.ticker import MaxNLocator
import seaborn as sb
import pandas as pd
import matplotlib.pyplot as plt
from shapely.geometry import Polygon
from matplotlib.backends.backend_pdf import PdfPages


def min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


def process_relative_err(df: pd.DataFrame):
    if any(df.loc[pd.IndexSlice[:, :, "map_glb_count"], "std"] != 0):
        raise ValueError("WARNING: global number of nodes differs between merged runs")

    df = df.rename(columns={"p_50": "median"})
    df = df.unstack("data").swaplevel(-2, -1, axis=1)
    df = df.sort_index(axis=1)
    df = df.drop(columns=[("map_glb_count", "std"), ("map_glb_count", "median")])
    df = df.rename(columns={"map_mean_count": "map_count"})
    df.columns = [f"{l1}_{l2}" for l1, l2 in df.columns]
    df = df.rename(columns={"map_glb_count_mean": "map_glb_count"})

    g = "map_glb_count"
    m = "map_count_mean"
    gn = "map_glb_count_n"
    mn = "map_count_mean_n"
    mm = "map_count_median"
    mmn = "map_count_median_n"
    df[gn] = min_max_norm(df[g])
    df[mn] = min_max_norm(df[m], max=df[g].max())
    df[mmn] = min_max_norm(df[mm], max=df[g].max())

    df.columns.name = "data"
    df = df.stack().sort_index()
    return df


def get_run_map(study: SuqcStudy, out_dir: str, load_if_present=True) -> RunMap:
    if load_if_present and os.path.exists(os.path.join(out_dir, "run_map.json")):
        return RunMap.load_from_json(os.path.join(out_dir, "run_map.json"))

    groups: dict = {}
    pattern = re.compile(
        r"_stationary_m_base_single_cell, (?P<base>.*?)(?P<num>\d+)_(?P<run_id>\d+)_(?P<seed>\d+)$"
    )
    for sim in study.sim_iter():
        extends = sim.run_context.oppini["extends"]
        m = pattern.match(extends)
        m = m.groupdict()
        group_name = f"{m['base']}{m['num']}"
        group = groups.get(group_name, [])
        group.append((m["run_id"], sim))
        groups[group_name] = group

    run_map: RunMap = RunMap(output_dir=out_dir)
    # sort group and append to RunMap
    keys = list(groups.keys())
    keys.sort()
    for g_name in keys:
        group = groups[g_name]
        group.sort(key=lambda x: x[0])
        run_map.append_or_add(RunMap(g_name, [sim[1] for sim in group]))

    # 415x394  -> 207x196
    run_map.save_json()
    return run_map


def read_data(run_map: RunMap):

    merged_norm_path = run_map.path("merged_normalized_map_measure.h5")
    if os.path.exists(merged_norm_path):
        map = pd.DataFrame(
            pd.HDFStore(merged_norm_path, mode="r").get("map_measure_norm_static")
        )
    else:

        data = ["map_glb_count", "map_mean_count"]
        kwargs_iter = [
            dict(
                sim_group=sim,
                data=data,
                frame_consumer=process_relative_err,
            )
            for sim in run_map.values()
        ]
        maps = run_kwargs_map(
            func=OppAnalysis.merge_maps, kwargs_iter=kwargs_iter, pool_size=10
        )
        map: pd.DataFrame = pd.concat(maps, axis=0, verify_integrity=True)
        map.to_hdf(merged_norm_path, key="map_measure_norm_static", mode="a")

    return map


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
                map.loc[
                    pd.IndexSlice[g, :, "map_glb_count"],
                ].max(),
            ]
        )

    return pd.DataFrame.from_records(
        ret, columns=["sim", "convergence_time", "ped_count"]
    )


def main(run_map: RunMap):

    map = read_data(run_map).sort_index()

    # get convergence time
    df_convergence = get_convergence_time(map, time_slice=slice(41, None), err=0.05)
    df_convergence.to_csv(run_map.path("convergence_time.csv"))

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
        fig, axes = plt.subplots(3, 4, figsize=(16, 9), sharex="all", sharey="all")
        fig.suptitle(f"Position for scenario {lbl} with N=10 seeds.", fontsize=16)
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

    # study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_1/")
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_8/")
    run_map: RunMap = get_run_map(
        study, out_dir="/mnt/data1tb/results/mf_stationary_m_single_cell_8/"
    )
    main(run_map)
    # plot_positions(study, get_run_map(study))
    # update_hdf_files()
    print("done")
