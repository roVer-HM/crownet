import os
import matplotlib.pyplot as plt
from roveranalyzer.analysis.common import Simulation, SimulationGroup, SuqcStudy, RunMap
from roveranalyzer.analysis.omnetpp import HdfExtractor, OppAnalysis
from matplotlib.backends.backend_pdf import PdfPages
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.utils.misc import change_locale
from roveranalyzer.utils.plot import FigureSaverPdfPages, FigureSaverSimple, PlotUtil
from roveranalyzer.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169
import pandas as pd
import numpy as np
from pandas import IndexSlice as _i
from copy import deepcopy
from roveranalyzer.analysis.plot import (
    PlotAppMisc,
    PlotEnb,
    PlotAppTxInterval,
    PlotDpmMap,
)
import roveranalyzer as r

load_matplotlib_style(STYLE_SIMPLE_169)


def _corridor_filter_target_cells(df: pd.DataFrame) -> pd.DataFrame:
    # remove cells under target area
    xy = df.index.to_frame()
    mask = np.repeat(False, xy.shape[0])
    # for x in [400, 405, 410]: # remove target cells
    for x in [400, 405, 410, 0, 5, 10]:  # remove source/target cells
        y = 180.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    # for x in [0.0, 5.0, 10.0]:    # remove target cells
    for x in [0.0, 5.0, 10.0, 400, 405, 410]:  # remove source/target cells
        y = 205.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    ret = df[~mask].copy(deep=True)
    print(f"remove {df.shape[0]-ret.shape[0]} rows")
    return ret


def build_run_0(output_dir) -> RunMap:
    """
    Simulation with N=20 seeds and two different numbers of agents
    in the simulation. The study used the mf topography.
    """

    def sim_group_f(sim: Simulation, **kwds):
        iat = sim.run_context.ini_get(
            "*.bonnMotionServer.traceFile", regex=r".*exp_(\d+)_.*", apply=int
        )
        max_bw_beacon = sim.run_context.ini_get(
            "*.misc[*].app[0].scheduler.maxApplicationBandwidth", apply=str
        )
        max_bw_map = sim.run_context.ini_get(
            "*.misc[*].app[1].scheduler.maxApplicationBandwidth", apply=str
        )
        group_name = f"g_iat_{iat}"
        attr = deepcopy(kwds.get("attr", {}))
        attr["iat"] = iat
        attr["beaconMaxBw"] = max_bw_beacon
        attr["mapMaxBw"] = max_bw_map
        kwds["attr"] = attr
        return SimulationGroup(group_name, **kwds)

    study = SuqcStudy("/mnt/data1tb/results/s0_mf_topo_2")
    run_map = study.update_run_map(
        run_map=RunMap(output_dir),
        sim_per_group=1,
        # sim_per_group=20,
        id_offset=0,
        sim_group_factory=sim_group_f,
    )
    return run_map


def get_run_map_0() -> RunMap:
    return RunMap.load_or_create(
        build_run_0, output_path="/mnt/data1tb/results/_dynamicTxInterval/s0-003"
    )


def plot_per_sim_data(sim: Simulation):

    # redirect all plot here
    print("create saver")
    saver = FigureSaverSimple(
        override_base_path=sim.path("fig_out_22"), figure_type=".png"
    )

    # agent count data
    print("app misc")
    PlotAppMisc.plot_number_of_agents(sim, saver=saver)

    # application data
    PlotAppMisc.plot_system_level_tx_rate_based_on_application_layer_data(
        sim=sim, saver=saver
    )

    # app tx data
    PlotAppTxInterval.plot_txinterval_all(
        data_root=sim.data_root, sql=sim.sql, app="Beacon", saver=saver
    )
    PlotAppTxInterval.plot_txinterval_all(
        data_root=sim.data_root, sql=sim.sql, app="Map", saver=saver
    )

    PlotAppMisc.plot_application_delay_jitter(sim, saver=saver)

    # map specifics
    print("map misc")
    dmap = sim.get_dcdMap()
    dmap.plot_map_count_diff(savefig=saver.with_name("Map_count.png"))

    # remove source cells.
    msce = dmap.cell_count_measure(columns=["cell_mse"])
    msce = _corridor_filter_target_cells(msce).reset_index()

    # msce time series
    PlotDpmMap.plot_msce_ts(msce, savefig=saver.with_name("Map_msce_ts.png"))
    # msce ecdf
    PlotDpmMap.plot_msce_ecdf(
        msce["cell_mse"], savefig=saver.with_name("Map_msce_ecdf.png")
    )


def main():

    saver = FigureSaverSimple(
        override_base_path="/mnt/data1tb/results/s1_corridor_5/simulation_runs/outputs/fig_out",
        figure_type=".png",
    )
    os.makedirs(saver.override_base_path, exist_ok=True)
    sim_nt = Simulation(
        f"/mnt/data1tb/results/s1_corridor_4/simulation_runs/outputs/Sample_0_0/final_dynamic_m_bonn_motion_out/",
        "AdaptiveTx_NT",
    )
    sim_map = Simulation(
        f"/mnt/data1tb/results/s1_corridor_4/simulation_runs/outputs/Sample_20_0/final_dynamic_m_bonn_motion_out/",
        "AdaptiveTx_Map_2000ms",
    )
    sim_const = Simulation(
        f"/mnt/data1tb/results/s1_corridor_4/simulation_runs/outputs/Sample_40_0/final_dynamic_m_bonn_motion_out/",
        "Constant_Map_2000ms",
    )
    sim_map_100ms_min = Simulation(
        f"/mnt/data1tb/results/s1_corridor_5/simulation_runs/outputs/Sample_20_0/final_dynamic_m_bonn_motion_out/",
        "AdaptiveTx_Map_100ms",
    )
    # sims = [sim_const, sim_nt, sim_map]
    sims = [sim_const, sim_map, sim_map_100ms_min]

    # msce ecdf
    PlotDpmMap.cmp_msce_ecdf(
        sims,
        frame_c=_corridor_filter_target_cells,
        savefig=saver.with_name("Map_msce_ecdf_comparison.png"),
    )

    PlotAppMisc.cmp_system_level_tx_rate_based_on_application_layer_data(
        sims=sims, saver=saver
    )

    PlotDpmMap.cmp_plot__map_count_diff(
        sims=sims, savefig=saver.with_name("Map_count_diff_comparison.png")
    )

    PlotEnb.cmp_plot_served_blocks_ul_all(sims=sims, saver=saver)

    print("done.")


if __name__ == "__main__":
    main()
