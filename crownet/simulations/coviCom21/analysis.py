import os

import matplotlib
from matplotlib.lines import Line2D

from roveranalyzer.simulators.rover.dcd.interactive import InteractiveAreaPlot, InteractiveValueOverDistance
from roveranalyzer.utils.path import get_or_create

matplotlib.use('TkAgg')

import matplotlib.animation as animation

from roveranalyzer.simulators.opp.utils import ScaveTool
from roveranalyzer.simulators.opp.opp_analysis import Opp, OppAccessor
from roveranalyzer.utils import PathHelper, from_pickle
from roveranalyzer.simulators.rover.dcd.dcd_map import DcdMap2DMulti
from itertools import product
import matplotlib.pyplot as plt
from roveranalyzer.utils import check_ax
import pandas as pd
import numpy as np

ROOT = "/mnt/results200gb"

runs = [{"run": "vadere00_60_20210214-21:23:26", "run_no": 0, "run_name": "mf_s60_0"},
        {"run": "vadere00_60_20210214-21:51:11", "run_no": 1, "run_name": "mf_s60_1"},
        {"run": "vadere00_60_20210214-22:24:09", "run_no": 2, "run_name": "mf_s60_2"},

        {"run": "vadere00_120_20210215-10:25:46", "run_no": 0, "run_name": "mf_s120_0"},
        {"run": "vadere00_120_20210215-12:02:03", "run_no": 1, "run_name": "mf_s120_1"},
        {"run": "vadere00_120_20210215-14:06:42", "run_no": 2, "run_name": "mf_s120_2"},
        ]


def read_data(path, *args, **kwargs):
    global_map_path = path.glob(
        "global.csv", recursive=False, expect=1
    )
    node_map_paths = path.glob("dcdMap_*.csv")
    scenario_path = path.glob("vadere.d/*.scenario", expect=1)

    dcd = DcdMap2DMulti.from_paths(
        global_data=global_map_path,
        node_data=node_map_paths,
        real_coords=True,
        load_all=False,
        scenario_plotter=scenario_path,
    )

    return dcd


def read_param(root_path, run=0, *args, **kwargs):
    scave_tool = ScaveTool()
    SCA = f"{root_path}/vars_rep_{run}.sca"
    scave_filter = scave_tool.filter_builder().t_parameter().build()
    df_parameters = scave_tool.read_parameters(SCA, scave_filter=scave_filter)
    return df_parameters


def read_app_data(root_path, run=0, *args, **kwargs):
    inputs = f"{root_path}/vars_rep_{run}.vec"
    scave = ScaveTool()
    scave_f = scave.filter_builder() \
        .gOpen().module("*.node[*].aid.densityMapApp").OR().module("*.node[*].aid.beaconApp").gClose() \
        .AND().name("rcvdPkLifetime:vector")
    _df = scave.load_df_from_scave(input_paths=inputs, scave_filter=scave_f)
    return _df


def analyse_interactive(dcd, what):
    if what == "map":

        time = 2
        id = 0
        fig, ax = dcd.area_plot(time_step=time, node_id=id,
                                pcolormesh_dic=dict(vmin=0, vmax=4))
        i = InteractiveAreaPlot(dcd, ax)
    else:

        fig, ax = dcd.plot_delay_over_distance(64, 2, "measurement_age", bins_width=5)
        i = InteractiveValueOverDistance(dcd, ax)

    i.show()


def make_density_plot(path, dcd):
    # make density_map_plots
    time = [140]
    ids = [0]
    for time, id in list(product(time, ids)):
        f, ax = dcd.area_plot(time_step=time, node_id=id, make_interactive=False,
                              pcolormesh_dic=dict(vmin=0, vmax=4))
        print(f"create out/density_map_{id}_t{time}.png")
        ax.set_title("")
        f.savefig(path.join(f"out/density_map_{id}_t{time}.png"))
        plt.close(f)


def make_count_plot(path, dcd, para):
    # make count plot
    f1, ax = dcd.plot_count_diff()
    maxAge = para.loc[para["name"] == "maxAge", ["value"]].iloc[0].value
    title = f"{ax.title.get_text()} with neighborhood table maxAge {maxAge}"
    ax.set_title(title)
    os.makedirs(path.join("out"), exist_ok=True)
    out_p = path.join("out/count.png")
    f1.savefig(out_p)


def make_delay_plot(dcd, para, delay, save_path):
    # make count plot
    f1, ax = dcd.plot_count_diff()
    font_dict = dcd.font_dict

    # ax.set_title("Node count and packet delay (beacon and map) over time", **font_dict["title"])
    ax.set_title("")
    ax.set_ylabel("Pedestrian count", **font_dict["ylabel"])

    df_all = delay.opp.filter().vector().normalize_vectors(axis=0)

    plots = [["packet delay", df_all]]
    time_per_bin = 1.0  # seconds
    for n, df in plots:
        bins = int(np.floor(df["time"].max() / time_per_bin))

        df = df.groupby(pd.cut(df["time"], bins)).mean()
        df = df.dropna()

    ax_twin = ax.twinx()
    ax_twin.plot("time", "value", data=df, color='r', label="Packet delays")
    ax_twin.set_ylim(0, 100)
    ax_twin.set_ylabel(f"Mean delay [s] ($w_z = {time_per_bin}s$)", **font_dict["ylabel"])
    handles, labels = ax.get_legend_handles_labels()
    handles.append(Line2D([0], [0], color='r', linewidth=1))
    labels.append("Packet delay")
    ax.legend().remove()
    ax_twin.legend().remove()
    plt.legend().remove()
    f1.legends.clear()
    ax.tick_params(axis="x", labelsize=dcd.font_dict["tick_size"])
    ax.tick_params(axis="y", labelsize=dcd.font_dict["tick_size"])
    f1.legend(handles, labels, prop=font_dict["legend"], bbox_to_anchor=(.895, 0.87))
    f1.savefig(save_path)
    df["value"].describe().to_csv(f"{save_path}_delay.txt")

    print(f"save {save_path}")
    plt.close(f1)


def get_env(run):
    root = f"{ROOT}/{run['run']}"
    ext_root = f"/mnt/data/{run['run']}"
    p = PathHelper(root)
    return p, ext_root


def make_figures():
    for run in runs:
        p, ext_root = get_env(run)
        dcd = get_or_create(pickle_path=p.join("analysis.p"), generator_fn=read_data, override=False, path=p)
        para = get_or_create(pickle_path=p.join("paramters.p"), generator_fn=read_param, override=False,
                             root_path=ext_root, run=run["run_no"])
        app_delay = get_or_create(pickle_path=p.join("rcvdPkLifetimeVec.p"), generator_fn=read_app_data, override=False,
                                  root_path=ext_root, run=run["run_no"])

        # plots
        make_delay_plot(dcd, para, app_delay, save_path=f"{ROOT}/{run['run_name']}_count_delay.pdf")


def make_sample_map():
    run = runs[3]
    p, ext_root = get_env(run)
    dcd = get_or_create(pickle_path=p.join("analysis.p"), generator_fn=read_data, override=False)
    fig, ax = dcd.area_plot(time_step=96, node_id=0, title="Decentralized Crowed Density Map",
                            pcolormesh_dic=dict(vmin=0, vmax=4))
    fig.savefig(f"{ROOT}/{run['run_name']}_sample_map.pdf")


def make_grid_map():
    run = runs[3]
    p, ext_root = get_env(run)
    dcd = get_or_create(pickle_path=p.join("analysis.p"), generator_fn=read_data, override=False)
    fig, ax = dcd.area_plot(time_step=96, node_id=0, title="Decentralized Crowed Density Map",
                            pcolormesh_dic=dict(vmin=0, vmax=4))
    ax.set_title("")
    fig.delaxes(fig.axes[1])
    ax.set_xticks(dcd.meta.X)
    ax.set_yticks(dcd.meta.Y)
    fig.savefig(f"{ROOT}/{run['run_name']}_grid_map.pdf")


def delay_distance():
    run_60 = runs[0]
    p60, ext_root60 = get_env(run_60)
    run_120 = runs[3]
    p120, ext_root120 = get_env(run_120)
    dcd_60 = get_or_create(pickle_path=p60.join("analysis.p"), generator_fn=read_data, override=False, path=p60)
    dcd_120 = get_or_create(pickle_path=p120.join("analysis.p"), generator_fn=read_data, override=False, path=p120)

    time = 66
    delay_t = "measurement_age"
    fig, ax = check_ax()
    fig, ax = dcd_120.plot_delay_over_distance(time, -1, delay_t, remove_null=True, label="S2 (overload situation)", ax=ax)
    fig, ax = dcd_60.plot_delay_over_distance(time, -1, delay_t, remove_null=True,
                                              label="S1", ax=ax,
                                              ax_prop=dict(title="Measurement Age over Distance for $t=66\,s$"))
    fig.legend(prop=dcd_60.font_dict["legend"], bbox_to_anchor=(.41, 0.87))
    ax.tick_params(axis="x", labelsize=dcd_60.font_dict["tick_size"])
    ax.tick_params(axis="y", labelsize=dcd_60.font_dict["tick_size"])
    ax.lines[0].set_linestyle("None")
    ax.lines[1].set_linestyle("None")
    ax.lines[0].set_marker("o")
    ax.lines[1].set_marker("v")
    ax.set_xlim(right=200)
    ax.set_ylabel(f"Age in [s] ", **dcd_120.font_dict["ylabel"])
    ax.set_title("")

    fig2, ax2 = check_ax(figsize=(4, 9))
    # ax2.set_title("Distance distribution of \n NT entries to owner", fontsize=14)
    ax2.set_title("")
    local_ = dcd_120.map.index.get_level_values("ID") == dcd_120.map["source"]
    df_hist_120 = dcd_120.map.loc[(dcd_120.map["count"] != 0) & local_]
    ax2.hist(df_hist_120["owner_dist"], density=True, histtype="step", label="S2 (overload situation)")
    ax2.set_xlabel(f"Cell distance (euklid) to owners location [m]", **dcd_120.font_dict["xlabel"])

    local_ = dcd_60.map.index.get_level_values("ID") == dcd_60.map["source"]
    df_hist_60 = dcd_60.map.loc[(dcd_60.map["count"] != 0) & local_]
    ax2.hist(df_hist_60["owner_dist"], density=True, histtype="step", label="S2")
    ax2.set_xlabel(f"distance [m]", **dcd_60.font_dict["ylabel"])

    ax2.legend(prop={"size": 14})
    ax2.tick_params(axis="x", labelsize=dcd_60.font_dict["tick_size"])
    ax2.tick_params(axis="y", labelsize=dcd_60.font_dict["tick_size"])
    fig.savefig(f"{ROOT}/60_120_delayOverDistance.pdf")


def main():
    # make_figures()
    # make_sample_map()
    delay_distance()
    # interactive: select section
    # make_grid_map()


if __name__ == "__main__":
    main()
