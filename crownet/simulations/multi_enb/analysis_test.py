import os
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.imputation import (
    ArbitraryValueImputationWithRsd,
    DeleteMissingImputation,
)
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.node_position import (
    EnbPositionHdf,
    NodePositionHdf,
    RsdPositionHdf,
)
from crownetutils.analysis.omnetpp import (
    OppAnalysis,
    HdfExtractor,
    NodePositionWithRsdHdf,
)
from crownetutils.analysis.plot.app_misc import PlotAppMisc, PlotAppTxInterval
from crownetutils.analysis.plot.dpmMap import PlotDpmMap
from crownetutils.analysis.plot.enb import PlotEnb
from crownetutils.utils.dataframe import merge_on_interval
from crownetutils.utils.logging import TimeIt
from crownetutils.utils.plot import FigureSaverSimple
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.collections import LineCollection
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType


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
        global_map_csv_name="entropyMap_global.csv",
        node_map_csv_glob="entropyMap_*.csv",
        beacon_app_path=None,
        map_app_path="app[2]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def build_all(base_dir):
    for f in [get_density_cfg, get_entropy_cfg]:
        cfg = f(base_dir)
        build_hdf(cfg)


def build_hdf(cfg: DpmmCfg):
    sim: Simulation = Simulation.from_dpmm_cfg(cfg)
    b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
    b.only_selected_cells(True)
    if cfg.is_count_map:
        rsd_origin_position = sim.sql.get_resource_sharing_domains(
            apply_offset=False, bottom_left_origin=True
        )

        i = ArbitraryValueImputationWithRsd(
            rsd_origin_position=rsd_origin_position, fill_value=0.0
        )
        b.set_imputation_strategy(i)
    else:
        b.set_imputation_strategy(DeleteMissingImputation())

    sim.sql.append_index_if_missing()

    b.build()

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
    cfg = get_density_cfg(
        # os.path.join(sim_base, "results/count_and_entropy")
        # os.path.join(mnt_base, "test_run_600s_killed/",
        # os.path.join(sim_base,"results/count_and_entropy")
        # os.path.join(sim_base, "results/final_muli_enb_dev_20230821-07:18:40")
        os.path.join(
            mnt_base,
            "final_multi_enb_30_min_20230828-test_with_3_apps_1cell_entropy_rnd_stream",
        )
    )

    # build_all(cfg.base_dir)
    sim = Simulation(
        data_root=cfg.base_dir,
        label="mulit_enb",
        dpmm_cfg=cfg,
    )
    m = sim.get_dcdMap()
    OppAnalysis.plot_simtime_to_realtime_ratios(
        os.path.join(
            mnt_base,
            "final_multi_enb_30_min_20230828-test_with_3_apps_1cell_entropy_rnd_stream/cmd.out",
        ),
        os.path.join(
            mnt_base,
            "final_multi_enb_30_min_20230828-test_with_3_apps_1cell_entropy_rnd_stream/cmd.pdf",
        ),
        nodes=m.glb_map.groupby(["simtime"]).sum(),
    )

    # # map = sim.get_dcdMap()
    # # h = sim.get_base_provider(group_name="cell_measures_by_rsd", path=cfg.hdf_path())
    # # h.clear_group(group="cell_measures_by_rsd", repack=True)
    # OppAnalysis.append_err_measures_to_hdf(sim)

    # plot_enb_association(sim)
    # plot_enb_association_map(sim)
    # # inter = OppAnalysis.get_serving_enb_interval(sim)


if __name__ == "__main__":
    main(override=False)
