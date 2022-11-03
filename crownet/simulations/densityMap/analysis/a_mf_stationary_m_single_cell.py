from __future__ import annotations
from fileinput import filename
from math import nan
from multiprocessing import pool
import re
import itertools
from sqlite3 import converters
from matplotlib.transforms import Affine2D
from matplotlib.lines import Line2D
from shapely.geometry import Point, Polygon
from roveranalyzer.analysis.common import (
    NamedSimulationGroupFactory,
    RunMap,
    Simulation,
    RunMap,
    SimulationGroup,
    SuqcStudy,
    SimGroupAppendStrategy,
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


def _read_groups(
    study: SuqcStudy,
    run_map: RunMap,
    id_offset: int,
    group_strategy=SimGroupAppendStrategy.APPEND,
):
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
        sim_list.append((int(m["run_id"]), sim))
        group["sim_list"] = sim_list
        groups[group_name] = group

    keys = list(groups.keys())
    keys.sort()
    for g_name in keys:
        group = groups[g_name]
        sim_list = group["sim_list"]
        sim_list.sort(key=lambda x: x[0])
        sim_list = [s[1] for s in sim_list]
        run_map.append_or_add(
            SimulationGroup(g_name, sim_list, attr=group["attr"]),
            strategy=group_strategy,
        )

    # calc density
    area = {}
    sim: Simulation = run_map.any(lambda x: "full" in x.group_name)[0]
    _meta = sim.builder.global_p.get_meta_object()
    area["A1"] = (_meta.x_dim * _meta.y_dim, f"A1: {_meta.x_dim}x{_meta.y_dim}")
    area["A2"] = (area["A1"][0] / 4, f"A2: {_meta.x_dim/2}x{_meta.y_dim/2}")

    for name, g in run_map.items():
        if any([i in name for i in ["full", "position"]]):
            _k = "A1"
        else:
            _k = "A2"
        g.attr["area"] = area[_k][0]
        g.attr["density"] = g.attr["num"] / g.attr["area"]
        g.attr["area_id"] = _k
        g.attr["area_lbl_long"] = area[_k][1]
        g.attr["area_str"] = area[_k][1][4:]

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


def run_map_from_6_7(output_path: str, *args, **kwargs) -> RunMap:
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_6/")
    study2 = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_7/")
    run_map: RunMap = _read_groups(study, RunMap(output_path), id_offset=0)
    return _read_groups(
        study2,
        run_map,
        id_offset=run_map.max_id + 1,
        group_strategy=SimGroupAppendStrategy.DROP_NEW,
    )


def run_map_from_4_8(output_path: str, *args, **kwargs) -> RunMap:
    """Simulation run of ydist and ymf heuristic with multiple numbers of pedestrians as well as constant densities."""
    run_map = RunMap(output_path)
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_8/")
    run_map = _read_groups(study, run_map, id_offset=0)

    study2 = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_4/")
    run_map = _read_groups(study2, run_map, id_offset=run_map.max_id + 1)
    return run_map


def read_map_data(run_map: RunMap):
    return OppAnalysis.run_get_merge_maps(
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
    return OppAnalysis.run_collect_maps(
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


def _filter_convergence_time(run_map, convergence_error=0.05, event_time: float = 40.0):
    data = read_map_data_by_run(run_map)
    time_slice = slice(event_time, None, None)
    # calculate convergence time step for each simulation (sim, run)
    data = data.unstack("data").droplevel(0, axis=1)

    data["abs_err"] = np.abs(data["map_glb_count_n"] - data["map_count_mean_n"])
    data["conv_mask"] = data["abs_err"] < convergence_error
    data["convergence_time"] = data.index.get_level_values("simtime") - event_time
    # add attribute
    _attr = run_map.attr_df()
    data = data.join(_attr, on="sim")
    data = data.rename(columns={"area_lbl_long": "Area"})

    # Ids of runs which have at least one time point where the run has a n error of
    # less than convergence_error *before* the event time.
    runs_with_valid_values_before_event_time = (
        data[data["conv_mask"]]
        .loc[pd.IndexSlice[:, :, : (event_time - 1)]]
        .reset_index("simtime")
        .index.unique()
    )
    runs_wrong_before_event = (
        data.reset_index("simtime")
        .index.unique()
        .difference(runs_with_valid_values_before_event_time)
    )
    wrong_dict = {}
    for sim, run in runs_wrong_before_event:
        runs = wrong_dict.get(sim, [])
        runs.append(run)
        wrong_dict[sim] = runs
    print(
        f">>>{len(runs_wrong_before_event)}/{len(data.reset_index('simtime').index.unique())} wrong before time {event_time}:"
    )
    # for k, v in wrong_dict.items():
    #     print(f"{k}: {v}")
    wrong_mask = np.array(
        [i in runs_wrong_before_event for i in data.reset_index("simtime").index]
    )

    df = data[~wrong_mask].loc[:, :, time_slice].copy(deep=True)
    # convergence is reached at first time point where ["conv_mask"] is true --> idxmax()
    # Note if ["conv_mask"] is never true, i.e. convergence is not reached the first index
    # for that group is used.
    conv_idx = pd.MultiIndex.from_tuples(
        df.groupby(["sim", "run"])["conv_mask"].idxmax()
    )

    df = df.loc[conv_idx]
    runs_wrong_after_event = (
        df.loc[~df["conv_mask"]].reset_index("simtime").index.unique()
    )
    # set convergence time for runs that did not reach convergence to -1
    df.loc[~df["conv_mask"], ["convergence_time"]] = data.index.get_level_values(
        "simtime"
    ).max()

    runs_correct = (
        data.reset_index("simtime")
        .index.unique()
        .difference(runs_wrong_before_event)
        .difference(runs_wrong_after_event)
    )
    runs_correct = [(i, ii, "correct") for i, ii, in runs_correct]
    runs_wrong_before_event = [
        (i, ii, "err_before") for i, ii in runs_wrong_before_event
    ]
    runs_wrong_after_event = [(i, ii, "err_after") for i, ii in runs_wrong_after_event]
    idx = pd.MultiIndex.from_tuples(
        [*runs_correct, *runs_wrong_before_event, *runs_wrong_after_event],
        names=("sim", "run", "type"),
    )
    df_num = (
        pd.DataFrame(1, index=idx, columns=["count"])
        .join(run_map.attr_df())
        .reset_index()
    )

    return df, df_num


def _append_convergence_time_count(df, name="count"):
    idx = list(df.index.names)
    idx.append("convergence_time")
    _df = df.reset_index().set_index(idx).iloc[:, 0]
    _df = _df.groupby(idx).count()
    _df.name = name
    df = df.reset_index().join(_df, on=idx).set_index(idx[0:-1])
    return df


def plot_convergence_per_density(
    run_map: RunMap,
    convergence_error=0.05,
    event_time: float = 40.0,
    ax: plt.Axes | None = None,
):

    # data
    df, df_num = _filter_convergence_time(run_map, convergence_error, event_time)
    # add count
    df = _append_convergence_time_count(
        df.reset_index().set_index(["density", "Area"]), name="Number simulations"
    )
    # only use densities where both areas are present.
    _density_mask = (
        df_num.reset_index().set_index("density")["count"].groupby("density").count()
    )
    _density_mask = _density_mask[_density_mask == 80].index
    df = df.reset_index().set_index(["density", "Area"]).loc[_density_mask]

    # stat
    x = df.reset_index().set_index(["sim", "run"]).index
    y = pd.MultiIndex.from_product(
        [x.get_level_values(0).unique(), list(range(20))], names=["sim", "run"]
    )
    tbl = (
        pd.DataFrame(1, columns=["count"], index=y.difference(x))
        .join(run_map.attr_df(), on="sim")
        .reset_index()
        .set_index(["run", "alg", "area_id", "density"])
        .loc[:, ["count"]]
        .groupby(["alg", "area_id", "density"])
        .count()
    )  # before list
    tbl["err"] = "before"
    tbl = tbl.set_index("err", append=True)
    tbl_after = (
        pd.DataFrame(
            1,
            columns=["count"],
            index=pd.MultiIndex.from_frame(
                df[df["convergence_time"] >= 100.0][["sim", "run"]]
            ),
        )
        .reset_index()
        .join(run_map.attr_df(), on="sim")
        .set_index(["run", "alg", "area_id", "density"])
        .loc[:, ["count"]]
        .groupby(["alg", "area_id", "density"])
        .count()
    )
    tbl_after["err"] = "after"
    tbl_after = tbl_after.set_index("err", append=True)
    tbl = pd.concat([tbl, tbl_after]).sort_index()
    tbl = (
        tbl.unstack(["alg", "err"], fill_value=0)
        .swaplevel(axis=0)
        .droplevel(0, axis=1)
        .sort_index()
    )
    tbl = tbl.reset_index()
    tbl = tbl.rename(columns={"area_id": "Area", "density": "Density"})
    tbl = tbl.iloc[:, [0, 1, 3, 2, 5, 4]]  # sort columns
    FrameUtl.save_as_tex_table(
        tbl,
        path=run_map.path("convergence_time_per_run_over_density.tex"),
        str_replace=lambda x: x.replace(r"{r}{", r"{l}{"),
    )

    df = df[df["convergence_time"] < 100.0]
    # create scatter plot
    df = df.rename(
        columns=dict(
            ped_count="Number of pedestrians", convergence_time="Convergence time"
        )
    )
    # plot
    with plt.rc_context(PlotUtl.paper_rc()):
        fig, ax = plt.subplots(1, 1, figsize=(8, 9 / 16 * 8))
        ax = sn.scatterplot(
            ax=ax,
            data=df.reset_index(),
            x="density",
            y="Convergence time",
            hue="area_id",
            style="area_id",
            size="Number simulations",
            sizes=(100, 300),
            markers=["P", "X"],
            # markers=["s", "D"],
            legend="brief",
            zorder=3,
        )
        c_time_mean = sn.lineplot(
            data=df.reset_index(),
            ax=ax,
            x="density",
            color="black",
            y="Convergence time",
            zorder=1,
            label="Mean convergence time $\\pm$ std",
            legend=None,
        )
        # ax.set_title("Convergence time of simulations over density and area size", pad=10)
        ax.xaxis.set_major_locator(MaxNLocator(10))
        ax.set_xlim(0, df.reset_index()["density"].max())
        ax.set_xticklabels([f"{t*1000:.2f}" for idx, t in enumerate(ax.get_xticks())])
        # ax.text(1.02, -.005, "$10\\times^{-3}$", transform=ax.transAxes, fontsize="xx-large")
        ax.set_xlabel("Density in $10\\times^{-3}\\frac{ped}{m^2}$")

        ax.yaxis.set_major_locator(FixedLocator(np.arange(0, 31, 5)))
        ax.set_xlim(0, df.reset_index()["density"].max())
        ax.set_ylim(-1, df[df["conv_mask"]]["Convergence time"].max() + 5)
        ax.set_ylabel(
            "Convergence time in seconds",
        )
        h, l = ax.get_legend_handles_labels()
        _h = np.array(h)[[-1, 1, 2]]
        _l = np.array(l)[[-1, 1, 2]]
        # _h = h[-1]
        # _l = l[-1]
        # for i in [-1, 3, 0]:
        #     del h[i]
        #     del l[i]
        # fig.legend(h, l, loc="lower center", ncol=len(h), columnspacing=0.5)
        # [ i.set_fontsize("xx-small") for i in  fig.legends[0].texts]
        ax.legend().remove()
        ax.legend(_h, _l, loc="upper right")

        print(f">>>plot: {ax.get_title()}")
        print(
            f">>>Use N={df.shape[0]}/{df_num.reset_index().set_index('density').loc[_density_mask].shape[0]} simulations. Simulation always outside of the error band or with convergence timer > 50s are removed"
        )
        print(
            f">>> Description density > 0.003\n{df.loc[0.0003:, :, :]['Convergence time'].describe()}"
        )

        fig.tight_layout(rect=(0.0, 0.08, 1.0, 1.0))
        fig.savefig(run_map.path("convergence_time_per_run_over_density.pdf"))


def plot_convergence_per_num(
    run_map: RunMap,
    convergence_error=0.05,
    event_time: float = 40.0,
    ax: plt.Axes | None = None,
):

    df, df_num = _filter_convergence_time(run_map, convergence_error, event_time)
    # add count
    df = _append_convergence_time_count(
        df.reset_index().set_index(["num", "Area"]), name="Number simulations"
    )

    df = df.reset_index()

    fig, ax = check_ax(ax)
    ax = sn.scatterplot(
        ax=ax,
        data=df.reset_index(),
        x="num",
        y="convergence_time",
        hue="Area",
        style="Area",
        size="Number simulations",
        sizes=(50, 250),
        markers=["P", "X"],
        # markers=["s", "D"],
        legend="brief",
    )
    ax.set_ylabel("Convergence time in seconds")
    ax.set_xlabel("Number of pedestrians")
    ax.set_title(
        "Convergence time of simulations over number of pedestrians and area size"
    )

    # create x axis tick labels based on data
    l = np.sort(df["num"].unique()).astype(int)
    ax.xaxis.set_major_locator(FixedLocator(l))
    ax.set_xticklabels(l, rotation=90, ha="center")
    ax.set_ylim(-1, df[df["conv_mask"]]["convergence_time"].max() + 5)
    # fix legend layout and position
    # h, l = ax.get_legend_handles_labels()
    # _del_idx = [l.index(_l) for _l in ["alg", "count", "area"]]
    # for idx in _del_idx[::-1]:
    #     del h[idx]
    #     del l[idx]
    # fig.legend(h, l, loc="lower center", ncol=len(h), fontsize="x-small")
    # ax.get_legend().remove()

    # table
    # _max_count = df_run.set_index(["num", "alg", "type"])["count"].groupby("num").sum().iloc[0]/2
    # df_run = df_run.set_index(["num", "alg", "type"])["count"].groupby(["num", "alg", "type"]).count().unstack(["alg", "type"], fill_value=0).sort_index(axis=1)
    # _m = (df_run[("yDist", "correct")] == _max_count) & (df_run[("ymf", "correct")] == _max_count)
    # df_run = df_run[~_m]
    # tbl_cols =  [f"{c}\n{cc}" for c, cc in  df_run.columns]
    # tbl_cols.insert(0, "ped.\nnum")
    # tbl = ax.table(
    #     cellText=df_run.reset_index().values,
    #     cellLoc="center",
    #     colLabels=tbl_cols,
    #     bbox=(0.45, 0.35, 0.5, 0.5),
    # )
    # tbl.scale(1, 5)
    # tbl.set_zorder(999)  # on top
    # tbl_t = ax.text(0.45, 0.88, "Erroneous runs with N=40 simulations per ped. number and algorithm", transform=ax.transAxes, backgroundcolor="white")
    # tbl_t.set_zorder(999)

    print(f">>>plot: {ax.get_title()}")
    print(
        f">>>Use N={df_num[df_num['type']=='correct'].shape[0]}/{df_num.shape[0]} simulations. Simulation always outside of the error band or with convergence timer > 50s are removed"
    )
    fig.tight_layout(rect=(0.0, 0.05, 1.0, 1.0))
    fig.savefig(run_map.path("convergence_time_per_run_over_ped_number.pdf"))


# def plot_convergence_per_num_old(
#     run_map: RunMap,
#     convergence_error=0.05,
#     event_time: float = 40.0,
#     ax: plt.Axes | None = None,
# ):

#     df, df_run= _filter_convergence_time(run_map, convergence_error, event_time)

#     df = df.reset_index()
#     # create small x-axis offset to prevent overlapping data
#     df["ped_count"] = df["num"]
#     df = df.set_index(["num", "alg", "run"]).sort_index()
#     df.loc[pd.IndexSlice[:, "ymf", :], ["ped_count"]] += 0.3
#     df.loc[pd.IndexSlice[:, "yDist", :], ["ped_count"]] -= 0.3

#     # create scatter plot
#     df = df.rename(
#         columns=dict(
#             ped_count="Number of pedestrians", convergence_time="Convergence time"
#         )
#     )
#     fig, ax = check_ax(ax)
#     ax = sn.scatterplot(
#         ax=ax,
#         data=df.reset_index(),
#         x="Number of pedestrians",
#         y="Convergence time",
#         hue="alg",
#         style="area",
#         size="count",
#     )

#     # create x axis tick labels based on data
#     l = np.sort(df.index.get_level_values("num").unique()).astype(int)
#     ax.xaxis.set_major_locator(FixedLocator(l))
#     ax.set_xticklabels(l, rotation=90, ha="center")
#     ax.set_ylim(-1, df[df["conv_mask"]]["Convergence time"].max() + 5)
#     # fix legend layout and position
#     h, l = ax.get_legend_handles_labels()
#     _del_idx = [l.index(_l) for _l in ["alg", "count", "area"]]
#     for idx in _del_idx[::-1]:
#         del h[idx]
#         del l[idx]
#     fig.legend(h, l, loc="lower center", ncol=len(h), fontsize="x-small")
#     ax.get_legend().remove()
#     # save figure
#     ax.set_title("Convergence time for mean density map for each simulation run")

#     # table
#     _max_count = df_run.set_index(["num", "alg", "type"])["count"].groupby("num").sum().iloc[0]/2
#     df_run = df_run.set_index(["num", "alg", "type"])["count"].groupby(["num", "alg", "type"]).count().unstack(["alg", "type"], fill_value=0).sort_index(axis=1)
#     _m = (df_run[("yDist", "correct")] == _max_count) & (df_run[("ymf", "correct")] == _max_count)
#     df_run = df_run[~_m]
#     tbl_cols =  [f"{c}\n{cc}" for c, cc in  df_run.columns]
#     tbl_cols.insert(0, "ped.\nnum")
#     tbl = ax.table(
#         cellText=df_run.reset_index().values,
#         cellLoc="center",
#         colLabels=tbl_cols,
#         bbox=(0.45, 0.35, 0.5, 0.5),
#     )
#     # [c.set_height(2) for c in tbl.get_celld().values()]
#     tbl.scale(1, 5)
#     tbl.set_zorder(999)  # on top
#     tbl_t = ax.text(0.45, 0.88, "Erroneous runs with N=40 simulations per ped. number and algorithm", transform=ax.transAxes, backgroundcolor="white")
#     tbl_t.set_zorder(999)


#     fig.tight_layout(rect=(0., 0.05, 1., 1.))
#     fig.savefig(run_map.path("convergence_time_per_run_over_ped_number.pdf"))


def main(run_map: RunMap):

    map = read_map_data(run_map).sort_index()

    # get convergence time
    df_convergence = get_convergence_time(map, time_slice=slice(51, None), err=0.05)
    df_convergence.to_csv(run_map.path("convergence_time.csv"))

    # plot
    plot_all_relative_pedestrian_count_ts(map)
    plot_merged_relative_pedestrian_count_ts_by_area(map, run_map)
    # plot_positions(run_map)
    plot_all_absolute_pedestrian_count_ts(map)
    plot_convergence_per_density(run_map, event_time=51.0, convergence_error=0.05)
    plot_convergence_per_num(run_map, event_time=51.0, convergence_error=0.05)
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


def plot_single_position(run_map: RunMap):
    # get data
    sim_g: SimulationGroup = run_map.any(
        lambda x: x.attr["num"] == 64 and x.attr["area_id"] == "A2"
    )
    sim: Simulation = sim_g.simulations[0]
    _, bound = sim.sql.get_sim_offset_and_bound()
    bound_poly = sim.sql.get_bound_polygon().exterior.xy
    _b = np.array(sim.sql.get_bound_polygon().exterior.bounds)
    _b[:2] = (_b / 4)[2:]
    _b[2:] = (_b * 3 / 4)[2:]
    bound_poly_small = Polygon(
        [_b[:2], (_b[2], _b[1]), (_b[2:]), (_b[0], _b[3])]
    ).exterior.xy
    pos_df = OppAnalysis.merge_position(sim_g)
    run_id = pos_df.index.get_level_values(0)[0]
    # plot
    with plt.rc_context(PlotUtl.paper_rc(tick_labelsize="x-large")):
        ax: plt.Axes
        fig, ax = plt.subplots(1, 1, figsize=(5, 5))
        ax.set_aspect("equal", adjustable="box")
        ax.set_xlim(-10, bound[0] + 10)
        ax.set_ylim(-10, bound[1] + 10)
        _ticks = np.arange(0, 500, 50)
        ax.xaxis.set_major_locator(FixedLocator(_ticks))
        ax.yaxis.set_major_locator(FixedLocator(_ticks))

        enb = ax.scatter(415 / 2, 394 / 2, marker="s", color="black", label="eNB")
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
            marker="p",
            linestyle="none",
            color="blue",
        )
        ax.plot(bound_poly[0], bound_poly[1], color="red")
        ax.plot(bound_poly_small[0], bound_poly_small[1], color="red")
        ax.text(
            15,
            365,
            "A1: 415x394 m",
            zorder=5,
            fontsize="xx-large",
            backgroundcolor="white",
        )
        ax.text(
            110,
            70,
            "A2: 207x196 m",
            zorder=5,
            fontsize="xx-large",
            backgroundcolor="white",
        )
        ax.legend(
            handles=[enb],
            loc="lower right",
            bbox_to_anchor=(0.59, 0.02, 0.4, 0.4),
            handletextpad=0.1,
        )
        fig.tight_layout()
        fig.savefig(run_map.path("s2-topography.pdf"))


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


def merge_relative_with_error_bars(
    run_map: RunMap,
    gb_index,
    data=None,
    df_filter: FrameUtl.FrameConsumer = FrameUtl.FrameConsumer.EMPTY,
):
    data: pd.DataFrame = read_map_data(run_map).sort_index() if data is None else data
    _glb = data.loc[data.index.get_level_values("sim")[0], :, "map_glb_count_n"]
    data = data.loc[pd.IndexSlice[:, :, "map_count_mean_n"], :].copy()
    data = data.join(run_map.attr_df(), on="sim")
    # filter by density
    data = df_filter(data)
    # add group discriminator
    # data = split_sim_key(data)
    data = data.reset_index().set_index(gb_index)
    ret = data.groupby(gb_index)[0].agg(["mean", "std", "sem", "count"])
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
    for sim_size in ret.index.get_level_values(gb_index[1]).unique():
        stat_df = (
            ret.loc[pd.IndexSlice[["yDist", "ymf"], sim_size, :], ["mean"]]
            .unstack(gb_index[:2][::-1])
            .droplevel([0, 1], axis=1)
            .reset_index(drop=True)
        )
        t = mannwhitneyu(stat_df.iloc[:, 0], stat_df.iloc[:, 1])
        stat.append([t[0], t[1], f"{sim_size}-ydist/ymf"])
    stat = pd.DataFrame(np.array(stat), columns=["stat", "p-value", "name"])

    return ret, data, stat


# normalized mean only
def ax_trans(ax: plt.Axes, num=2, offset=0.2):
    """Transformations which creates distributes 'num' items 'offset' amount apparat around zero."""
    _start = 0.0 - float((num - 1) * offset) / 2
    _stop = _start + num * offset
    return [
        Affine2D().translate(x, 0.0) + ax.transData
        for x in np.arange(_start, _stop, offset)
    ]


def plot_merged_relative_pedestrian_count_ts_by_density(
    data: pd.DataFrame, run_map: RunMap
):
    def density_filter(df: pd.DataFrame, max_num=6):
        idx = df.index.names
        _df = df.reset_index().set_index("density")
        _density_mask = (
            _df.reset_index().set_index("density").iloc[:, 0].groupby("density").count()
        )
        _density_mask = _density_mask[_density_mask == _density_mask.max()].index
        if len(_density_mask) >= max_num:
            _density_mask = _density_mask.sort_values(ascending=False)[:max_num]
        return _df.loc[_density_mask].reset_index().set_index(idx)

    data: pd.DataFrame = read_map_data(run_map).sort_index() if data is None else data
    data = (
        data.loc[:, :, ["map_count_mean_n", "map_glb_count_n"], :]
        .unstack(["data"])
        .droplevel(0, axis=1)
        .join(run_map.attr_df(), on="sim")
    )
    data = data.rename(
        columns={"map_count_mean_n": "ped_count_n", "map_glb_count_n": "glb"}
    )

    # filter by density
    ret = density_filter(data, max_num=9)
    ret["alg_area"] = ret["alg"] + "-" + ret["area_id"]
    ret = ret.reset_index().set_index(["density", "alg_area", "simtime"])
    filename = "normalized_pedestrian_count_merged_by_density.pdf"

    # stat
    density_area_id = pd.MultiIndex.from_frame(
        ret.reset_index()[["density", "area_id"]]
    ).unique()
    alg = ret["alg"].unique()
    stat = []
    _d = ret.reset_index().set_index(["density", "area_id", "alg"])["ped_count_n"]
    for density, area in density_area_id:
        t = mannwhitneyu(_d.loc[density, area, alg[0]], _d.loc[density, area, alg[1]])
        stat.append(
            [
                t[0],
                t[1],
                density,
                area,
                *alg,
                "H0 (same)" if t[1] > 0.05 else "H1 (diff)",
            ]
        )
    stat = pd.DataFrame(
        stat,
        columns=["stat", "p-value", "density", "area_id", "alg1", "alg2", "h_label"],
    )
    stat.to_csv(run_map.path("normalized_pedestrian_count_merged_by_density_mwu.csv"))
    tbl = (
        stat.set_index(["density", "area_id"])[["p-value", "h_label"]]
        .unstack("area_id")
        .swaplevel(axis=1)
        .sort_index(axis=1)
        .iloc[:, [1, 0, 3, 2]]
        .reset_index()
    )

    FrameUtl.save_as_tex_table(
        tbl,
        run_map.path("normalized_pedestrian_count_merged_by_density_mwu.tex"),
        col_format=[
            (pd.IndexSlice[:, pd.IndexSlice[:, "p-value"]], FrameUtl.siunitx())
        ],
        str_replace=lambda x: x.replace("h\_label", "").replace(r"{r}{A", r"{l}{A"),
    )

    with PdfPages(run_map.path(filename)) as pdf:
        with plt.rc_context(PlotUtl.paper_rc()):
            g = sn.relplot(
                data=ret.reset_index(),
                x="simtime",
                y="ped_count_n",
                col="density",
                hue="alg_area",
                col_wrap=3,
                kind="line",
                aspect=4.0 / 2,
                height=3,
                zorder=5,
            )
            g.set_titles("")
            _loc = np.arange(30, 101, 10)
            for density, ax in g.axes_dict.items():
                print(f"for density {density}")
                ax.text(
                    0.97,
                    0.97,
                    f"density: {density:.4e} $\\frac{{ped}}{{m^2}}$\nPLOS: {1./density:.2f} $\\frac{{m^2}}{{ped}}$",
                    transform=ax.transAxes,
                    horizontalalignment="right",
                    verticalalignment="top",
                    fontsize="xx-large",
                    fontweight="bold",
                    backgroundcolor="white",
                    zorder=9,
                )
                ax.patch.set_edgecolor(np.array([204, 204, 204, 255]) / 255)
                ax.patch.set_linewidth("1")
                ax.xaxis.set_major_locator(FixedLocator(_loc))
                sn.lineplot(
                    data=ret.reset_index(),
                    x="simtime",
                    y="ped_count_n",
                    units="sim",
                    color=".7",
                    ax=ax,
                    estimator=None,
                    legend=False,
                    zorder=4,
                )
                _df = ret.loc[pd.IndexSlice[density, :, :], ["glb"]]
                _df = _df.loc[:, _df.index.get_level_values(1)[0], :].reset_index()
                ax.plot(
                    "simtime",
                    "glb",
                    data=_df,
                    drawstyle="steps-post",
                    label="Ground Truth",
                    color="black",
                    zorder=6,
                )
            g.set_xlabels("Time in seconds")
            g.set_ylabels("Norm. ped. count")
            g.tight_layout(rect=(0.0, 0.05, 1.0, 1.0))
            h = np.array(g.figure.legends[0].legendHandles)
            l = np.array(g.figure.legends[0].texts)
            _l = np.argsort(np.array([i._text[-2:] for i in g.figure.legends[0].texts]))
            l = list(l[_l])
            h = list(h[_l])
            h.insert(0, Line2D([0], [0], color="k"))
            l.insert(0, plt.Text(0, 0, "Ground Truth"))
            g.figure.legends[0].set_title(None)
            g.figure.legends[0].legendHandles = h
            g.figure.legends[0].texts = l
            sn.move_legend(
                g,
                loc="lower center",
                ncol=5,
                labelspacing=0.2,
                handletextpad=0.2,
                columnspacing=0.8,
                handlelength=1.0,
            )
            pdf.savefig(g.figure)


def plot_merged_relative_pedestrian_count_ts_by_area(
    data: pd.DataFrame, run_map: RunMap
):

    density_filter = lambda df: df[df["density"] > 0.00030]
    idx_by_area = ["alg", "area_id", "simtime"]  # by area_id
    idx_by_density = ["alg", "density", "simtime"]  # by density

    ret, data, stat = merge_relative_with_error_bars(
        run_map, gb_index=idx_by_density, df_filter=density_filter
    )

    title = (
        f"Normalized number of pedestrians over time in areas A1=[415x394] A2=[207x196]"
    )
    filename = "normalized_pedestrian_count_merged_by_area.pdf"

    p = [
        ("Norm.  pedestrian count +/- Std Dev", "std"),
        # ("Mean +/- SEM", "sem"),
        # ("Mean with 95% confiden500ce interval", ["clm_olow", "clm_ohigh"]),
    ]

    with PdfPages(run_map.path(filename)) as pdf:
        for ylabel, err in p:
            fig, ax = plt.subplots(1, 1, figsize=(16, 9))
            id_sel = list(ret.droplevel(2, axis=0).index.unique())
            _glb = ret.loc[pd.IndexSlice[id_sel[0][0], id_sel[0][1], :], "glb"]
            ax.plot(
                _glb.index.get_level_values("simtime"),
                _glb,
                drawstyle="steps-post",
                label="Ground Truth",
                color="black",
            )
            ax.set_title(title)
            ax.set_ylim(0.4, 1.1)
            ax.set_xlim(ret.index.get_level_values("simtime").min(), 100)
            ax.set_ylabel(ylabel)
            ax.set_xlabel("Time in seconds")
            ax.xaxis.set_major_locator(MaxNLocator(10))
            # data
            translate = ax_trans(ax, len(id_sel))
            for idx, (i, ii) in enumerate(id_sel):
                _df = ret.loc[i, ii, :]
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
                    label=f"S2-{i}-{ii}",
                )
                ax.legend()
            stat = stat[["name", "p-value"]]
            stat["name"] = "S2-" + stat["name"]
            stat.columns = ["Mann-Whitney U", "p-value"]
            stat = FrameUtl.format_frame(
                stat, si_func={"p-value": FrameUtl.siunitx(precision=4)}
            )

            tbl = ax.table(
                cellText=stat.values,
                cellLoc="center",
                colLabels=stat.columns,
                bbox=(0.55, 0.25, 0.4, 0.2),
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

    # run_map: RunMap = RunMap.load_or_create(
    #     create_f=run_map_from_6,
    #     output_path="/mnt/data1tb/results/_density_map/02a_stationary_output/",
    # )

    run_map: RunMap = RunMap.load_or_create(
        create_f=run_map_from_6_7,
        output_path="/mnt/data1tb/results/_density_map/02b_stationary_output/",
    )

    # plot_convergence_per_density(run_map, event_time=50.0, convergence_error=0.05)
    # plot_convergence_per_num(run_map, event_time=50.0, convergence_error=0.05)
    # main(run_map)
    # plot_relative_count_stat(run_map)
    plot_merged_relative_pedestrian_count_ts_by_density(None, run_map)
    # plot_single_position(run_map)
    print("done")
