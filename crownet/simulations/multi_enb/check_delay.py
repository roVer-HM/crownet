from __future__ import annotations
import sys
from typing import List
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.utils.plot import FigureSaver, FigureSaverSimple, PlotUtil
from crownetutils.utils.dataframe import combine_stats, index_or_col
from crownetutils.utils.styles import STYLE_SIMPLE_169, load_matplotlib_style
from crownetutils.analysis.common import RunMap, Simulation, SuqcStudy
from matplotlib import pyplot as plt
from matplotlib.ticker import EngFormatter, MultipleLocator
import numpy as np
import pandas as pd

from run_script import SimulationRun, get_density_cfg, get_entropy_cfg

load_matplotlib_style(STYLE_SIMPLE_169)


def get_sim(path):
    return Simulation.from_dpmm_cfg(path)

def get_RunMap(path):
    study = SuqcStudy(base_path=path)
    study.get_sim()


def plot_delay_for_rsd_7and8(node_rx, node_tx, saver):
    apps = node_tx.apps
    fig, ax = plt.subplots(nrows=3, ncols=1, figsize=(16,16), sharex=True)
    for i, app in enumerate(apps):
        a: plt.Axes = ax[i]
        for rsd in [7, 8]:
            data = node_rx.rcvd_data(app).select(where=f"servingEnb={rsd}")
            a.scatter(data.index.get_level_values("time"), data["delay"], label=f"rsd{rsd}", alpha=.3)
        a.legend()
        PlotUtil.auto_major_minor_locator(a)
        a.set_title(app.name)
        a.set_xlabel("time in seconds")
        a.set_ylabel("dealy in seconds")
    fig.tight_layout
    saver(fig, "dealy_for_rsd_7and8.png")


if __name__ == "__main__":
    print("hi")
    # sims = [0,20,40,60,80,100,120,140,160]
    sims = [0]
    saver: FigureSaverSimple = FigureSaverSimple("./dbg_fig")
    for sim_id in sims:
        run, cfg_d, cfg_e = SimulationRun.postprocessing(
            # result_dir=f"/home/stsc/simulation_output/arc-dsa_multi_cell/s2_ttl_and_stream_1/simulation_runs/outputs/Sample_{sim_id}_0/final_multi_enb_out",
            # result_dir="/home/stsc/simulation_output/arc-dsa_multi_cell/s2_ttl_and_stream_2/simulation_runs/outputs/Sample_0_0_380min/final_multi_enb_out",
            result_dir="/home/stsc/simulation_output/arc-dsa_multi_cell/s2_ttl_and_stream_2/simulation_runs/outputs/Sample_160_0/final_multi_enb_out",
            qoi="all",
            exec=True
            )
        # node_rx: NodeRxData = run.create_node_rx_hdf()
        # node_tx: NodeTxData = run.create_node_tx_hdf()
        # pos: NodePositionWithRsdHdf = run.create_position_hdf()
        # d_sim: Simulation = Simulation.from_dpmm_cfg(cfg_d)
        # e_sim: Simulation = Simulation.from_dpmm_cfg(cfg_e)
        # apps = [
        #     SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
        #     SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
        #     SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim),
        # ]

        # PlotAppMisc.plot_by_app_overview(saver, sim_id, node_rx, node_tx, pos, d_sim, apps, [7,8])
        # PlotAppMisc.plot_planed_throughput_in_rsd(saver, sim_id, node_tx, pos, apps)
        # PlotAppMisc.plot_received_bursts(node_rx, figuer_title=f"Simulation_{sim_id} fixed until time 400.0 seconds", saver=saver.with_suffix(f"_{sim_id}"), where="time < 400.0")
        print(f"done with sim {sim_id}")
    print("hi")
    