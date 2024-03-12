import os
import glob
import re
from typing import Any, List, Tuple
import shutil
from crownetutils.analysis.dpmm import MapType
from crownetutils.analysis.dpmm.dpmm import percentile
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.omnetpp.scave import CrownetSql
from crownetutils.utils.plot import enb_with_hex
from matplotlib.colors import Normalize, Colormap, ListedColormap

import matplotlib.pyplot as plt
import matplotlib as mpl

from crownetutils.analysis.common import Simulation
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg
from crownetutils.analysis.dpmm.hdf.dpmm_provider import DpmmProvider
from matplotlib.ticker import MultipleLocator
import numpy as np
import pandas as pd

from crownetutils.utils.styles import STYLE_SIMPLE_169, load_matplotlib_style

from run_script import SimulationRun

load_matplotlib_style(STYLE_SIMPLE_169)


def get_density_cfg(base_dir):
    density_cfg = DpmmCfg(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        global_map_csv_name="global.csv",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def get_entropy_cfg(base_dir):
    density_cfg = DpmmCfg(
        base_dir=base_dir,
        hdf_file="entropy_data.h5",
        map_type=MapType.ENTROPY,
        global_map_csv_name="global_entropyMap.csv",
        node_map_csv_glob="entropyMap_*.csv",
        node_map_csv_id_regex=r"entropyMap_(?P<node>\d+)\.csv",
        beacon_app_path=None,
        map_app_path="app[2]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


"""
* read entropy map from first and last two nodes base on csv file
"""


def get_pnode_csv(base_dir) -> List[Tuple[int, str]]:
    cfg = get_entropy_cfg("/dev/null")
    files = glob.glob(os.path.join(base_dir, cfg.node_map_csv_glob))
    _p = re.compile(cfg.node_map_csv_id_regex)
    node_ids = [int(_p.match(os.path.basename(i)).groupdict()["node"]) for i in files]
    files = list(zip(node_ids, files))
    files.sort(key=lambda x: x[0])
    return files


class NormalizedColors:
    @classmethod
    def num_colors(cls, num_colors, vmin, vmax, cm="tab20"):
        m = mpl.colormaps[cm](np.linspace(0, 1, num_colors))
        return cls(ListedColormap(m), vmin, vmax)

    def __init__(self, cm, vmin, vmax) -> None:
        self.cm: Colormap = cm
        self.norm: Normalize = Normalize(vmin, vmax)

    def __call__(self, val) -> Any:
        return self.cm(self.norm(val))


def load_map(file_path, node_id):
    provider = DpmmProvider(os.path.join("/dev/null", "out.h5"))
    df = provider.build_dcd_dataframe(file_path, node_id).reset_index()
    return df


def plot_area(df, ax: plt.Axes, enb_pos, run, ttl, order):
    enb_patch, hex_patch = enb_with_hex(
        origin=enb_pos[["x", "y"]].values, inner_r=650, scale=120
    )

    ax.add_collection(hex_patch)

    own_pos = df[df["own_cell"] != 0]
    ax.scatter(df["x"], df["y"], marker=".", label=f"cells")
    ax.scatter(own_pos["x"], own_pos["y"], color="red", marker=".", label="node path")
    ax.set_aspect("equal")
    ax.set_ylim(0, 6000)
    ax.set_xlim(0, 6000)
    ax.xaxis.set_major_locator(MultipleLocator(500))
    ax.yaxis.set_major_locator(MultipleLocator(500))
    ax.legend(loc="lower right", ncol=5)
    rsd_list = ",".join(np.sort(df["rsd_id"].unique()).astype(str))
    box = np.array([df[["x", "y"]].min(), df[["x", "y"]].max()])
    max_bound = box[1, :] - box[0, :]
    ax.text(
        x=130,
        y=150,
        s=f"map size: [ {max_bound[0]}m x {max_bound[1]}m ]",
        transform=ax.transData,
        bbox=dict(boxstyle="square,pad=.3", facecolor="white", edgecolor="black"),
    )

    ax.add_collection(enb_patch)
    for _, row in enb_pos.iterrows():
        ax.annotate(
            f"enb {int(row['hostId'])+1}",
            [row.x, row.y],
            xytext=[row.x - 200, row.y - 190],
            textcoords=ax.transData,
        )
    ax.set_title(
        f"run {run}, with ttl: {ttl} order: {order} rsd:{rsd_list} until {df['simtime'].max()} seconds"
    )


def read_csv_files_from_study(study_out_glob: str, save_path_csv: str):
    if os.path.exists(save_path_csv):
        return pd.read_csv(save_path_csv, delimiter=";")

    runs = glob.glob(study_out_glob)
    run_id_pattern = re.compile(r".*Sample_(?P<run>\d+).*")

    out = []
    for run in runs:
        run_id = int(run_id_pattern.match(run).groupdict()["run"])
        files = get_pnode_csv(os.path.join(run, "final_multi_enb_out"))
        out.append((run_id, files))

    with open(save_path_csv, "w", encoding="utf-8") as fd:
        fd.write("run_id;node_id;no;path\n")
        for run_id, files in out:
            for no, (node_id, file) in enumerate(files):
                fd.write(";".join([str(run_id), str(node_id), str(no), file]))
                fd.write("\n")

    return read_csv_files_from_study(study_out_glob, save_path_csv)


def copy_example_maps(base_dir):
    """Copy first and last two pedestrian csv entropy maps to new location"""

    df = read_csv_files_from_study(
        study_out_glob="/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream/simulation_runs/outputs/Sample*",
        save_path_csv="/mnt/data1tb/results/multi_enb/arc-dsa_multi_cell_data/sorted_csv.csv",
    ).sort_values(["run_id", "no"])

    for run_id, _df in df.groupby("run_id"):
        for i in [15, 16, -2, -1]:
            src = _df.iloc[i]["path"]
            dst = os.path.join(
                base_dir,
                f"Sample_{run_id}_0",
                "final_multi_enb_out",
                os.path.basename(src),
            )
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            print(f"copy {src} -> {dst}")
            shutil.copy(src=src, dst=dst)


def data_by_node(data):

    for node_id, node_data in data.items():
        for seed, by_seed in node_data.items():
            min_time = min([v["time_max"] for v in by_seed.values()])
            runs = list(by_seed.keys())
            runs.sort()
            yield node_id, seed, min_time, runs, by_seed


def create_cell_map():
    # base_dir = "/mnt/data1tb/results/multi_enb/arc-dsa_multi_cell_data"
    base_dir = "/mnt/data1tb/results/multi_enb/final_multi_enb_dev_20231018-09:23:55"
    # copy_example_maps(base_dir)

    study_out_glob = os.path.join(base_dir, "Sample*/**/entropyMap*.csv")
    map_csv_paths = glob.glob(study_out_glob)
    run_id_pattern = re.compile(r".*Sample_(?P<run>\d+).*")
    node_id_pattern = re.compile(r".*entropyMap_(?P<node>\d+)\.csv")

    colors = NormalizedColors.num_colors(16, 0, 15)

    sim_dir = "/mnt/data1tb/results/multi_enb/arc-dsa_multi_cell_data/Sample_0_0/final_multi_enb_out"
    cfg = get_entropy_cfg(sim_dir)
    sim: Simulation = Simulation.from_dpmm_cfg(cfg)
    sql: CrownetSql = sim.sql
    enb = sql.enb_position(
        apply_offset=False,
    )

    data = {}

    node_ids = [3500]
    for map_path in map_csv_paths:
        run_id = int(run_id_pattern.match(map_path).groupdict()["run"])
        node_id = int(node_id_pattern.match(map_path).groupdict()["node"])
        if node_id not in node_ids:
            continue
        node_dict = data.get(node_id, {})
        seed = run_id % 2

        by_seed = node_dict.get(f"seed_{seed}", {})
        df = load_map(map_path, node_id)
        time_max = df["simtime"].max()
        by_seed[run_id] = {"map_path": map_path, "df": df, "time_max": time_max}
        node_dict[f"seed_{seed}"] = by_seed
        data[node_id] = node_dict

    node_ids = list(data.keys())

    ttl_list = [15, 15, 15, 60, 60, 60, 3600, 3600, 3600]
    order_list = [
        "insert",
        "DistAsc",
        "DistDesc",
        "insert",
        "DistAsc",
        "DistDesc",
        "insert",
        "DistAsc",
        "DistDesc",
    ]

    # age over distance 1by3

    for node_id, seed, min_time, runs, node_seed_data in data_by_node(data):
        fig, axes = plt.subplots(ncols=1, nrows=3, figsize=(16, 9 * 2), sharex=True)
        axes = axes.flatten()
        for idx, (run, ttl, order) in enumerate(zip(runs, ttl_list, order_list)):
            ax: plt.Axes = axes[int(idx / 3)]
            df = node_seed_data[run]["df"]
            df = df[df["simtime"] <= min_time]
            df = get_measurement_age_over_distance(
                df, distance_bin=25.0, dist_col="owner_dist"
            )
            # df = get_measurement_age_over_distance(df, distance_bin=25.0, dist_col="tri_dist")
            ax.plot("mid", "mean", data=df, label=f"{run}/{ttl}/{order}")
            # ax.fill_between(
            #     df["mid"],
            #     df["p_25"],
            #     df["p_75"],
            #     alpha=0.35,
            #     interpolate=True
            # )
            ax.set_ylabel("Age of Information in seconds")
            ax.legend()
        ax.set_xlabel("Distance of cell to node in meter")
        fig.tight_layout()
        fig_path = os.path.join(base_dir, f"age_over_dist_{node_id}_{seed}.png")
        # fig_path =os.path.join(base_dir, f"age_over_tri_dist_{node_id}_{seed}.png")
        print(f"save {fig_path}")
        fig.savefig(fig_path)
        plt.close(fig)

    # map Plot 3by3
    for node_id, seed, min_time, runs, node_seed_data in data_by_node(data):
        fig, axes = plt.subplots(3, 3, figsize=(9 * 3, 9 * 3), sharex=True, sharey=True)
        axes = axes.flatten()
        for idx, (run, ttl, order) in enumerate(zip(runs, ttl_list, order_list)):
            ax: plt.Axes = axes[idx]
            df = node_seed_data[run]["df"]
            df = df[df["simtime"] <= min_time]
            plot_area(df=df, ax=ax, enb_pos=enb, run=run, ttl=ttl, order=order)
        fig_path = os.path.join(base_dir, f"node_{node_id}_{seed}.png")
        print(f"save {fig_path}")
        fig.tight_layout()
        fig.savefig(fig_path)
        plt.close(fig)


def get_measurement_age_over_distance(df, distance_bin=25.0, dist_col="owner_dist"):
    if dist_col == "tri_dist":
        tri_dist = df[["x", "y", "x_owner", "y_owner"]].values.reshape((-1, 2, 2))
        tri_dist = np.abs(tri_dist[:, 0] - tri_dist[:, 1]).sum(axis=1)
        df = df.copy()
        df["tri_dist"] = tri_dist
    bins = pd.interval_range(
        start=0, end=df[dist_col].max(), freq=distance_bin, closed="left"
    )
    d = df[[dist_col, "measurement_age"]].copy()
    d["dist_bin"] = pd.cut(df[dist_col], bins=bins)
    d = d.groupby("dist_bin")["measurement_age"].agg(
        ["count", "mean", "std", percentile(0.25), percentile(0.75)]
    )
    d = d.dropna()
    d["mid"] = [i.mid for i in d.index]
    d["left"] = [i.left for i in d.index]
    return d


def plot_map_and_pkt_count(pkt_count_ts, map_size_ts, pkt_bytes_ts, title, path):

    prop_cycle = plt.rcParams["axes.prop_cycle"]
    colors = prop_cycle.by_key()["color"]

    fig, axes = plt.subplot_mosaic("a;a;b;c;c", sharex=True, figsize=(9, 16))

    map_ax = axes["a"]
    map_ax.plot(
        "simtime", "count", data=map_size_ts, label="num cells in map", color=colors[0]
    )
    for i in range(1, 3, 1):
        map_ax.hlines(
            i * 114,
            map_size_ts["simtime"].min(),
            map_size_ts["simtime"].max(),
            colors="red",
        )
        map_ax.text(10, i * 114 + 2, f"packet {i}", color="red")

    pkt_ax = axes["b"]
    pkt_ax.plot(
        "time",
        "pkt_sent",
        data=pkt_count_ts,
        label="num pkts sent",
        linestyle="none",
        marker="3",
        color=colors[1],
    )
    # pkt_ax.legend(loc="upper left")
    pkt_ax.set_ylabel("packet count")
    map_ax.set_title(title)

    bytes_ax = axes["c"]
    bytes_ax.plot(
        "time", "consumed", data=pkt_bytes_ts, label="consumend bytes", color=colors[2]
    )
    bytes_ax.set_ylabel("Consumned bytes")
    bytes_ax.set_xlabel("simulation time in s")
    bytes_ax.xaxis.set_major_locator(MultipleLocator(100))
    bytes_ax.xaxis.set_minor_locator(MultipleLocator(25))
    for i in range(1, 3, 1):
        bytes_ax.hlines(
            i * 1400,
            pkt_bytes_ts["time"].min(),
            pkt_bytes_ts["time"].max(),
            colors="red",
        )
        bytes_ax.text(10, i * 1400 + 20, f"packet {i}", color="red")
    fig.legend(loc="lower center", ncol=3)
    fig.tight_layout(rect=[0, 0.02, 1, 1])

    print(f"save {path}")
    fig.savefig(path)
    plt.close(fig)


def create_map_pkt_count_plots(sim: Simulation):
    sql: CrownetSql = sim.sql
    sql.append_index_if_missing()

    out_path = sim.path("fig/packet_count")

    for vec_id in range(50):
        pkt_bytes_ts = sql.vec_data_pivot(
            module_name=f"World.pNode[{vec_id}].app[2].scheduler",
            vector_name_map={
                "consumedData:vector": {"name": "consumed", "dtype": float},
                "scheduledData:vector": {"name": "scheduled", "dtype": float},
            },
        )
        pkt_bytes_ts["consumed"] = pkt_bytes_ts["consumed"] / 8
        pkt_bytes_ts["scheduled"] = pkt_bytes_ts["scheduled"] / 8
        pkt_bytes_ts = pkt_bytes_ts.reset_index()
        pkt_sent = sql.vector_ids_to_host(
            module_name=f"World.pNode[{vec_id}].app[2].app",
            vector_name="packetSent:vector(packetBytes)",
            name_columns=["hostId"],
            pull_data=True,
            value_name="pkt_sent",
            columns=["vectorId", "simtimeRaw", "eventNumber", "value"],
            index=["hostId", "eventNumber", "time"],
        )
        host_id = pkt_bytes_ts["hostId"].unique()[0]
        map = pd.read_csv(
            sim.path(f"entropyMap_{host_id}.csv"), comment="#", delimiter=";"
        )

        pkt_count_ts = (
            pkt_sent.groupby(["hostId", "time"])["pkt_sent"]
            .count()
            .droplevel(0)
            .reset_index()
        )
        map_size_ts = map.groupby(["simtime"])["count"].count().reset_index()

        plot_map_and_pkt_count(
            pkt_count_ts,
            map_size_ts,
            pkt_bytes_ts,
            f"pNode[{vec_id}] with TTL 15 seconds",
            sim.path_mk(out_path, f"node_{vec_id}.png"),
        )


if __name__ == "__main__":
    print("run qoi....")
    _, cfg_denisty, cfg_entropy = SimulationRun.postprocessing(
        "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream/simulation_runs/outputs/Sample_0_0/final_multi_enb_out/",
        # "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream/simulation_runs/outputs/oldSample_0/final_multi_enb_out/",
        # qoi="plot_application_tx_throughput",
        qoi="plot_application_tx_time_hist_ecdf",
        exec=True,
    )
    e_sim: Simulation = Simulation.from_dpmm_cfg(cfg_entropy)
    d_sim: Simulation = Simulation.from_dpmm_cfg(cfg_denisty)
    pos = NodePositionWithRsdHdf.get_or_create(d_sim, d_sim.path("position.h5"))
    tx_data = NodeTxData.get_or_create(
        d_sim.path("node_tx_data.h5"),
        [
            SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
            SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
            SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim),
        ],
        pos,
    )
    rx_data = NodeRxData.get_or_create(
        hdf_path=d_sim.path("node_rx_data.h5"),
        apps=[
            SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
            SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
            SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim),
        ],
        node_pos=pos,
    )
