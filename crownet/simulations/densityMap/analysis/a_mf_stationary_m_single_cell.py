import re
import itertools
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
from matplotlib.ticker import MaxNLocator
import pandas as pd
import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages


def _min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


def _process_relative_err(df: pd.DataFrame):
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
    df[gn] = _min_max_norm(df[g])
    df[mn] = _min_max_norm(df[m], max=df[g].max())
    df[mmn] = _min_max_norm(df[mm], max=df[g].max())

    df.columns.name = "data"
    df = df.stack().sort_index()
    return df


def _read_groups(study: SuqcStudy, run_map: RunMap, alg_name: str, id_offset: int):
    groups: dict = {}
    pattern = re.compile(
        r"_stationary_m_base_single_cell, (?P<base>.*?)(?P<num>\d+)_(?P<run_id>\d+)_(?P<seed>\d+)$"
    )
    for sim in study.sim_iter(id_offset=id_offset):
        extends = sim.run_context.oppini["extends"]
        m = pattern.match(extends)
        m = m.groupdict()
        group_name = f"{alg_name}_{m['base']}{m['num']}"
        group = groups.get(group_name, {})
        attr = group.get("attr", {})
        attr["alg"] = alg_name
        attr["num"] = m["num"]
        attr["base"] = m["base"][:-1]
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
        sim_group_factory=NamedSimulationGroupFactory(["ydist", "ymf"]),
    )
    return run_map


def run_map_from_4_8(output_path: str, *args, **kwargs) -> RunMap:
    """Simulation run of ydist and ymf heuristic with multiple numbers of pedestrians as well as constant densities."""
    run_map = RunMap(output_path)
    study = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_8/")
    run_map = _read_groups(study, run_map, "ydist", id_offset=0)

    study2 = SuqcStudy("/mnt/data1tb/results/mf_stationary_m_single_cell_4/")
    run_map = _read_groups(study2, run_map, "ymf", id_offset=run_map.max_id + 1)
    return run_map


def read_map_data(run_map: RunMap):

    return OppAnalysis.merge_maps_for_run_map(
        run_map=run_map,
        data=["map_glb_count", "map_count_mean"],
        frame_consumer=_process_relative_err,
        hdf_path="merged_normalized_map_measure.h5",
        hdf_key="map_measure_norm_static",
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
                map.loc[
                    pd.IndexSlice[g, :, "map_glb_count"],
                ].max(),
            ]
        )

    return pd.DataFrame.from_records(
        ret, columns=["sim", "convergence_time", "ped_count"]
    )


def main(run_map: RunMap):

    map = read_map_data(run_map).sort_index()

    # get convergence time
    df_convergence = get_convergence_time(map, time_slice=slice(41, None), err=0.05)
    df_convergence.to_csv(run_map.path("convergence_time.csv"))

    # plot
    plot_all_relative_pedestrian_count_ts(map)
    plot_merged_relative_pedestrian_count_ts(map, run_map)
    plot_positions(run_map)
    plot_all_absolute_pedestrian_count_ts(map)


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
                fmt="",
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


def plot_merged_relative_pedestrian_count_ts(data: pd.DataFrame, run_map: RunMap):
    # normalized mean only
    _glb = data.loc[data.index.get_level_values("sim")[0], :, "map_glb_count_n"]
    data = data.loc[:, :, "map_count_mean_n"].copy()
    # add group discriminator

    mask_ydist = data.index.get_level_values("sim").str.startswith("ydist")
    mask_ymf = data.index.get_level_values("sim").str.startswith("ymf")
    mask_full = data.index.get_level_values("sim").str.contains("full")
    mask_quarter = data.index.get_level_values("sim").str.contains("quarter")
    mask_position = data.index.get_level_values("sim").str.contains("position")
    data["sim_group"] = ""
    data["sim_size"] = ""
    data.loc[mask_ydist, ["sim_group"]] = "ydist"
    data.loc[mask_ymf, ["sim_group"]] = "ymf"
    data.loc[mask_full, ["sim_size"]] = "full"
    data.loc[mask_quarter, ["sim_size"]] = "quarter"
    data.loc[mask_position, ["sim_size"]] = "position"
    # data = data.reset_index()
    data = data.reset_index().set_index(["sim_group", "sim_size", "simtime"])

    data = data.drop(columns=["sim"])
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

    p = [
        ("Mean +/- Std Dev", "std"),
        ("Mean +/- SEM", "sem"),
        ("Mean with 95% confidence interval", ["clm_olow", "clm_ohigh"]),
    ]

    def ax_trans(ax: plt.Axes, num=2, offset=0.1):
        """Transformations which creates distributes 'num' items 'offset' amount apparat around zero."""
        _start = 0.0 - float((num - 1) * offset) / 2
        _stop = _start + num * offset
        return [
            Affine2D().translate(x, 0.0) + ax.transData
            for x in np.arange(_start, _stop, offset)
        ]

    with PdfPages(run_map.path("normalized_pedestrian_count_merged.pdf")) as pdf:
        for ylabel, err in p:
            fig, axes = plt.subplots(2, 1, figsize=(16, 2 * 9))
            for ax_index, group in enumerate(
                data.index.get_level_values("sim_size").unique().to_list()
            ):
                data_page = ret.loc[:, group, :]
                ax: plt.Axes = axes[ax_index]
                translate = ax_trans(ax, 2)
                if group in ["position", "full"]:
                    ax.set_title(
                        f"Normalized number of pedestrians over time in Area [415x394] - {group}"
                    )
                else:
                    ax.set_title(
                        f"Normalized number of pedestrians over time in Area [207x196] - {group}"
                    )
                ax.set_ylim(0.4, 1.1)
                ax.set_ylabel(ylabel)
                ax.set_xlabel("Time in seconds")

                ax.plot(
                    _glb.index.get_level_values("simtime"),
                    _glb,
                    label="ground_truth",
                    color="black",
                )
                for idx, sim in enumerate(
                    data.index.get_level_values("sim_group").unique().to_list()
                ):
                    _df = data_page.loc[sim, :]
                    yerr = _df[err].to_numpy().T if isinstance(err, list) else _df[err]
                    ax.errorbar(
                        _df.index.get_level_values("simtime"),
                        _df["mean"],
                        yerr=yerr,
                        transform=translate[idx],
                        linewidth=0.8,
                        elinewidth=0.5,
                        capsize=2,
                        capthick=0.5,
                        label=sim,
                    )
                    ax.legend()
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
        OppAnalysis.count_stat_plots(
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
        + data["sim"].apply(lambda x: run_map.attr(x, "num"))
    )
    data = data[data["data"] != "gt_ymf"]
    data["data"] = data["data"].str.replace("gt_ydist", "gt")
    data = data.drop(columns=["sim", "alg"])
    data = data.set_index(["simtime", "data"]).sort_index()
    data = pd.concat([gt, data])
    data = data.unstack("data").droplevel(0, axis=1)

    # prepare statistic combinations (only match with ground truth and with algorithms)
    nums = np.unique(np.sort([int(run_map.attr(g, "num")) for g in groups]))
    stat_combination = []
    for num in nums:
        stat_combination.append(("gt", f"count_ydist_{num}"))
        stat_combination.append(("gt", f"count_ymf_{num}"))
    alg_combi = [(f"count_ydist_{num}", f"count_ymf_{num}") for num in nums]
    stat_combination.extend(alg_combi)

    # sort columns (simtime)[gt, ydist_A, ymf_A, ydist_B, ...]
    data = data.loc[:, ["gt", *list(itertools.chain(*alg_combi))]]

    # simplify label names
    lbl_dict = {c: c.replace("count_", "") for c in data.columns[1:]}
    OppAnalysis.count_stat_plots(
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

    run_map: RunMap = RunMap.load_or_create(
        create_f=run_map_from_4_8,
        output_path="/mnt/data1tb/results/_density_map/02_stationary_output/",
    )

    # run_map: RunMap = RunMap.load_or_create(
    #     create_f=run_map_with_2_step_ramp_up,
    #     output_path="/mnt/data1tb/results/mf_stationary_m_single_cell_5/",
    # )
    # main(run_map)
    plot_relative_count_stat(run_map)
    print("done")
