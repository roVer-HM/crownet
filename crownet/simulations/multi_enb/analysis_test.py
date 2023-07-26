import os
from attr import dataclass
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.hdf_providers.node_position import (
    EnbPositionHdf,
    NodePositionHdf,
)
from crownetutils.analysis.omnetpp import OppAnalysis, HdfExtractor, ServingEnbHdf
from crownetutils.analysis.plot.app_misc import PlotAppMisc, PlotAppTxInterval
from crownetutils.analysis.plot.dpmMap import PlotDpmMap
from crownetutils.analysis.plot.enb import PlotEnb
from crownetutils.utils.dataframe import merge_on_interval
from crownetutils.utils.plot import FigureSaverSimple
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.collections import LineCollection


def build_hdf(sim: Simulation):
    override_hdf = False
    _, builder, _ = OppAnalysis.builder_from_output_folder(
        data_root=sim.data_root, override_hdf=override_hdf
    )
    builder.only_selected_cells(True)
    builder.build(override_hdf=override_hdf)

    OppAnalysis.append_err_measures_to_hdf(sim)

    HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    HdfExtractor.extract_packet_loss(
        sim.path("pkt_loss.h5"), "map", sim.sql, sim.sql.m_map()
    )
    HdfExtractor.extract_packet_loss(
        sim.path("pkt_loss.h5"), "beacon", sim.sql, sim.sql.m_beacon()
    )

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
    PlotAppTxInterval.plot_txinterval_all(
        sim.data_root, sim.sql, app="Beacon", saver=saver
    )
    PlotAppTxInterval.plot_txinterval_all(
        sim.data_root, sim.sql, app="Map", saver=saver
    )

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

    ue = OppAnalysis.get_node_serving_data(
        sim,
        enb_pos=pos_enb,
        ue_pos=ue,
    )

    fig, ax = PlotEnb.plot_node_enb_association_map(ue, pos_enb.get_dataframe())
    fig.savefig(sim.path_mk("fig_err", "enb_association_map.png"))


def main(override):
    # sim = Simulation(
    #     data_root="/mnt/data1tb/results/multi_enb/test_run_600s/",
    #     label="mulit_enb",
    # )
    sim = Simulation(
        # data_root="/mnt/data1tb/results/multi_enb/test_run_600s_killed/",
        data_root="/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/results/with_rsd/",
        label="mulit_enb",
    )
    build_hdf(sim)

    plot_enb_association(sim)
    # plot_enb_association_map(sim)
    # inter = OppAnalysis.get_serving_enb_interval(sim)


if __name__ == "__main__":
    main(override=False)
