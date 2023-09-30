import io
import os
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.hdf.dpmm_provider import DpmmProvider
from crownetutils.analysis.dpmm.imputation import (
    ArbitraryValueImputation,
    DeleteMissingImputation,
    ImputationStream,
    FullRsdImputation,
    OwnerPositionImputation,
)
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.node_position import (
    EnbPositionHdf,
    NodePositionHdf,
)
from crownetutils.analysis.omnetpp import (
    OppAnalysis,
    HdfExtractor,
)
from crownetutils.analysis.plot.app_misc import PlotAppMisc, PlotAppTxInterval
from crownetutils.analysis.plot.dpmMap import PlotDpmMap
from crownetutils.analysis.plot.enb import PlotEnb
from crownetutils.utils.dataframe import merge_on_interval
from crownetutils.utils.logging import TimeIt
from crownetutils.utils.misc import Timer
from crownetutils.utils.plot import FigureSaverSimple, enb_patch
from matplotlib.ticker import FuncFormatter, MultipleLocator
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.collections import LineCollection, PatchCollection
from matplotlib.patches import Rectangle
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType

from PIL import Image

sim_base = os.path.join(os.environ["CROWNET_HOME"], "crownet/simulations/multi_enb")
mnt_base = os.path.join(
    os.environ["OPP_EXTERN_DATA_MNT"].split(":")[0], "results/multi_enb/"
)


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


def build_all(base_dir):
    for f in [get_density_cfg, get_entropy_cfg]:
        # for f in [get_entropy_cfg]:
        cfg = f(base_dir)
        build_hdf(cfg)


def build_hdf(cfg: DpmmCfg):
    sim: Simulation = Simulation.from_dpmm_cfg(cfg)
    sim.sql.append_index_if_missing()

    b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
    b.only_selected_cells(True)
    rsd_origin_position = sim.sql.get_resource_sharing_domains(
        apply_offset=False, bottom_left_origin=True
    )
    if cfg.is_count_map():
        stream = ImputationStream()
        stream.append(ArbitraryValueImputation(fill_value=0))
        # stream.append(DeleteMissingImputation())
        stream.append(FullRsdImputation(rsd_origin_position=rsd_origin_position))
        stream.append(OwnerPositionImputation())
        b.set_imputation_strategy(stream)
    else:
        stream = ImputationStream()
        stream.append(DeleteMissingImputation())
        stream.append(FullRsdImputation(rsd_origin_position=rsd_origin_position))
        stream.append(OwnerPositionImputation())
        b.set_imputation_strategy(stream)

    b.build()

    if cfg.map_type == MapType.ENTROPY:
        BaseHdfProvider(cfg.hdf_path()).repack_hdf(keep_old_file=False)

    OppAnalysis.append_err_measures_to_hdf(sim)

    HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    HdfExtractor.extract_packet_loss(
        sim.path("pkt_loss.h5"), "map", sim.sql, sim.sql.m_map()
    )
    # HdfExtractor.extract_packet_loss(
    #     sim.path("pkt_loss.h5"), "beacon", sim.sql, sim.sql.m_beacon()
    # )

    HdfExtractor.extract_trajectories(sim.path("position.h5"), sim.sql)


def plot_defaults(sim: Simulation):
    # enb data
    # auslastung, number of nodes over time

    # density map errours
    #
    saver = FigureSaverSimple(override_base_path=sim.path("fig"), figure_type=".png")
    PlotDpmMap.create_common_plots_density(
        sim.data_root, sim.builder, sim.sql, saver=saver
    )
    # PlotEnb.plot_served_blocks_ul_all(sim.data_root, sim.sql, saver=saver)
    PlotAppTxInterval.plot_txinterval_all_beacon(sim.data_root, sim.sql, saver=saver)
    PlotAppTxInterval.plot_txinterval_all_map(sim.data_root, sim.sql, saver=saver)

    return


def zero_enb_time_hist(serving, path, override):
    if os.path.exists(path) and not override:
        return

    os.makedirs(os.path.dirname(path), exist_ok=True)

    ax = (
        serving[serving["servingEnb"] == 0]
        .groupby("hostId")["delta"]
        .sum()
        .sort_values(ascending=False)
        .hist()
    )
    ax.set_ylabel("number of hosts")
    ax.set_xlabel("Time a node is NOT connected to any bases station")
    ax.set_title(
        "Amount of time nodes are not connected to any \n base station with simulation time of 600 seconds."
    )
    ax.get_figure().tight_layout()
    ax.get_figure().savefig(path)


def plot_enb_association(sim: Simulation):
    _i = pd.IndexSlice

    # density maps per node
    c = sim.get_dcdMap().count_p[_i[:, :, :, 1:], ["count"]]

    enb = OppAnalysis.get_serving_enb_interval(sim)

    fig, ax = PlotEnb.plot_node_enb_association_ts(data=enb)
    fig.savefig(sim.path_mk("fig_err", "enb_association_per_node.png"))
    plt.close(fig)

    # density map node count with upper bound based on cell association
    fig, ax = sim.get_dcdMap().plot_map_count_diff()

    cc = c.groupby(["ID", "simtime"]).sum().sort_index()
    cc.index.names = ["hostId", "time"]
    d = merge_on_interval(cc, enb).drop(columns=["interval", "start", "end"]).dropna()

    num_associated_over_time = (
        d[d["servingEnb"] > 0].groupby("time")["hostId"].count().reset_index()
    )
    ax.plot(
        num_associated_over_time["time"],
        num_associated_over_time["hostId"],
        label="total number of associated nodes (any eNB)",
    )

    ax.legend().remove()
    ax.legend()
    fig.tight_layout()
    fig.savefig(sim.path("fig_err", "enb_association_ts.png"))


def plot_enb_association_map(sim: Simulation):

    pos_enb: EnbPositionHdf = EnbPositionHdf.from_sim(sim=sim)
    pos_ue: NodePositionHdf = NodePositionHdf.from_sim(sim=sim)

    ue = pos_ue.get_dataframe()

    ue = OppAnalysis.get_node_serving_data_color_coded(
        sim,
        enb_pos=pos_enb.frame(),
        ue_pos=ue,
    )

    fig, ax = PlotEnb.plot_node_enb_association_map(ue, pos_enb.get_dataframe())
    fig.savefig(sim.path_mk("fig_err", "enb_association_map.png"))


def main(override):
    base_dir = os.path.join(
        mnt_base,
        "final_multi_enb_dev_20230929-15:03:00/",
        # "final_multi_enb_30_min_20230829-with_3_apps_25m_radius_entropy_rnd_stream",
    )
    cfg: DpmmCfg = get_entropy_cfg(
        # os.path.join(sim_base, "results/count_and_entropy")
        # os.path.join(mnt_base, "test_run_600s_killed/",
        # os.path.join(sim_base,"results/count_and_entropy")
        # os.path.join(sim_base, "results/final_muli_enb_dev_20230821-07:18:40")
        # os.path.join(
        #     mnt_base,
        #     "final_multi_enb_30_min_20230828-test_with_3_apps_1cell_entropy_rnd_stream",
        # )
        base_dir=base_dir
    )

    build_all(cfg.base_dir)

    sim = Simulation(
        data_root=cfg.base_dir,
        label="mulit_enb",
        dpmm_cfg=cfg,
    )
    sim.sql.append_index_if_missing()

    m = Simulation.from_dpmm_cfg(get_density_cfg(base_dir)).get_dcdMap()
    OppAnalysis.plot_simtime_to_realtime_ratios(
        sim.path("cmd.out"),
        sim.path("cmd.pdf"),
        nodes=m.glb_map.groupby(["simtime"]).sum(),
    )
    base_path = sim.path("age_over_distance")
    os.makedirs(base_path, exist_ok=True)

    bound = sim.sql.sim_bound()
    enb = EnbPositionHdf.from_sim(sim).frame()
    enb["x"] += bound.offset[0]
    enb["y"] += bound.offset[1]

    patches = []
    for _, row in enb.iterrows():
        patches.append(enb_patch(20, (row["x"], row["y"])))

    patches = PatchCollection(patches)

    for host_id in sim.sql.module_to_host_ids().id_map.values():

        m = DpmmProvider(hdf_path=cfg.hdf_path())
        with Timer(name="read hdf") as timer:
            df1 = m[pd.IndexSlice[:, :, :, :, host_id], :]
            timer.round("read by ID")
            df2 = m[pd.IndexSlice[5, :, :, :, :], :]
            timer.round("read by time")
            print("hi")
        x = m[pd.IndexSlice[:, :, :, :, host_id], :]
        xx = (
            x[["x_owner", "y_owner"]]
            .droplevel(["x", "y", "source", "ID"])
            .drop_duplicates()
        )
        fig, ax = plt.subplots(1, 1, figsize=(16, 9))
        ax.set_title(f"HostId {host_id}")
        ax.scatter(
            xx["x_owner"], xx["y_owner"], color="blue", marker="x", label="trajectory"
        )
        ax.set_xlim(0, sim.sql.sim_bound().abs_bound[0] + 1200)
        ax.set_ylim(0, sim.sql.sim_bound().abs_bound[1] + 1200)
        ax.set_ylabel("y in meter")
        ax.set_xlabel("x in meter")
        cells = PatchCollection(
            patches=[
                Rectangle(
                    xy, 5.0, 5.0, fill=False, edgecolor="black", facecolor="white"
                )
                for xy in x.index.droplevel(["simtime", "source", "ID"]).unique().values
            ],
            edgecolor="black",
            facecolor="white",
        )
        ax.add_collection(cells)
        patches.set_transform(ax.transData)
        ax.add_collection(patches)
        ax.set_aspect("equal")

        bins = pd.interval_range(
            start=0, end=x["owner_dist"].max(), freq=25.0, closed="left"
        )
        x["dist_bin"] = pd.cut(x["owner_dist"], bins=bins)
        fig.tight_layout()
        io_buf = io.BytesIO()
        fig.savefig(io_buf)
        plt.close(fig)

        fig, ax = plt.subplots(1, 1, figsize=(16, 9))
        agg_df = x.groupby("dist_bin")["measurement_age"].agg(["count", "mean", "std"])
        agg_df["x"] = [i.right for i in agg_df.index.values]
        agg_df = agg_df.reset_index(drop=True).set_index(["x"])
        agg_df = agg_df.set_axis(
            ["count", "mean age in second", "std age in second"], axis=1
        )
        agg_df.plot(
            subplots=True,
            sharex=True,
            ax=ax,
            marker=".",
            title="Distance node to measured cell in meter",
            xlabel="distance owner to measurement",
        )
        ax.set_title(f"AoI for node {host_id}")
        ax.xaxis.set_major_locator(MultipleLocator(25.0))
        ax.set_xlim(0, agg_df.index.max() + 20)
        fig.tight_layout()
        io_buf2 = io.BytesIO()
        fig.savefig(io_buf2)
        plt.close(fig)

        i1 = Image.open(io_buf)
        i2 = Image.open(io_buf2)
        out = Image.new("RGB", (i1.width + i2.width, i1.height))
        out.paste(i1, (0, 0))
        out.paste(i2, (i1.width, 0))
        out.save(os.path.join(base_path, f"host_id{host_id}.png"))
        del i1
        del i2
        del out
        print("hi")

    # # map = sim.get_dcdMap()
    # # h = sim.get_base_provider(group_name="cell_measures_by_rsd", path=cfg.hdf_path())
    # # h.clear_group(group="cell_measures_by_rsd", repack=True)
    # OppAnalysis.append_err_measures_to_hdf(sim)

    # plot_enb_association(sim)
    # plot_enb_association_map(sim)
    # # inter = OppAnalysis.get_serving_enb_interval(sim)


if __name__ == "__main__":
    main(override=False)
    # queue_test()
