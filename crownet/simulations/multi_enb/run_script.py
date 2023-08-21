#!/usr/bin/env python3
from functools import partial
import sys, os

import pandas as pd
import numpy as np
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
from matplotlib.backends.backend_pdf import PdfPages
from crownetutils.utils.plot import FigureSaverPdfPages, FigureSaverSimple
from crownetutils.analysis.vadere import VadereAnalysis
from crownetutils.analysis.omnetpp import HdfExtractor, OppAnalysis
from crownetutils.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from crownetutils.utils.misc import Project
from crownetutils.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.imputation import (
    ArbitraryValueImputationWithRsd,
    DeleteMissingImputation,
)

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


def get_cfg_list(base_dir):
    return [get_density_cfg(base_dir), get_entropy_cfg(base_dir)]


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    def get_builder(self, cfg: DpmmCfg):
        b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
        b.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        if cfg.is_count_map:
            rsd_origin_position = Simulation.from_dpmm_cfg(
                cfg
            ).sql.get_resource_sharing_domains(
                apply_offset=False, bottom_left_origin=True
            )

            i = ArbitraryValueImputationWithRsd(
                rsd_origin_position=rsd_origin_position, fill_value=0.0
            )
            b.set_imputation_strategy(i)
        else:
            b.set_imputation_strategy(DeleteMissingImputation())

        return b

    # todo: add pre and post processing to apply for each simulation run.
    # Post processing might inlcude result striping or aggreagtion.
    # add as many methods as needed and add the process_as annotation.
    # Use `prio` to ensure exeuction order (Bigger number higher prio)

    # @process_as({"prio": 20, "type": "post"})
    # def foo(self):
    # pass

    # @process_as({"prio": 10, "type": "pre"})
    # def bar(self):
    # pass

    @process_as({"prio": 999, "type": "post"})
    def build_hdf(self):

        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            builder = self.get_builder(cfg)
            builder.build(self.ns.get("hdf_override", "False"))

    @process_as({"prio": 980, "type": "post"})
    def append_err_measure_hdf(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg)
            OppAnalysis.append_err_measures_to_hdf(sim)

    @process_as({"prio": 975, "type": "post"})
    def create_rcvd_stats(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg, label="")
            HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    @process_as({"prio": 970, "type": "post"})
    def create_common_plots(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            result_dir, builder, sql = OppAnalysis.builder_from_cfg(cfg)
            _out = cfg.makedirs_output("fig_out", exist_ok=True)
            with PdfPages(self.result_dir(_out, "app_data.pdf")) as pdf:
                PlotEnb.plot_served_blocks_ul_all(
                    self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
                )
                # if sql.vector_exists(
                #     sql.m_beacon(), sql.OR(["txInterval:vector", "txInterval:vector"])
                # ):
                #     PlotAppTxInterval.plot_txinterval_all_beacon(
                #         self.result_base_dir(),
                #         sql=sql,
                #         saver=FigureSaverPdfPages(pdf),
                #     )

                # if sql.vector_exists(
                #     sql.m_map(), sql.OR(["txInterval:vector", "txInterval:vector"])
                # ):
                #     PlotAppTxInterval.plot_txinterval_all_map(
                #         self.result_base_dir(),
                #         sql=sql,
                #         saver=FigureSaverPdfPages(pdf),
                #     )
            if sql.is_count_map():
                print("build count based default plots")
                s = FigureSaverSimple(
                    self.result_dir(
                        _out,
                    )
                )
                builder.only_selected_cells(
                    self.ns.get("hdf_cell_selection_mode", True)
                )
                PlotDpmMap.create_common_plots_density(
                    result_dir,
                    builder,
                    sql,
                    selection=builder.get_selected_alg(),
                    saver=s,
                )
            else:
                print("build entropy map based plots")

    @process_as({"prio": 965, "type": "post"})
    def add_plots(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg, label="")
            p = cfg.makedirs_output("fig_out", exist_ok=True)
            saver = FigureSaverSimple(override_base_path=p, figure_type=".png")
            # agent count data
            print("app misc")
            # if cfg.is_count_map():
            #     PlotAppMisc.plot_number_of_agents(sim, saver=saver)

            # # application data
            # PlotAppMisc.plot_system_level_tx_rate_based_on_application_layer_data(
            #     sim=sim, saver=saver
            # )

            # # app tx data
            if cfg.is_count_map():
                PlotAppTxInterval.plot_txinterval_all_beacon(
                    data_root=sim.data_root, sql=sim.sql, saver=saver
                )
            # )
            PlotAppTxInterval.plot_txinterval_all_map(
                data_root=sim.data_root, sql=sim.sql, saver=saver
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

    # @process_as({"prio": 960, "type": "post"})
    # def remove_density_map_csv(self):
    #     cfg: DpmmCfg
    #     for cfg in get_cfg_list(self.result_base_dir()):
    #         builder = DpmmHdfBuilder.get(cfg)
    #         for f in builder.map_paths:
    #             os.remove(f)


if __name__ == "__main__":

    settings = []
    # settings = [
    #     "post-processing",
    #     "--qoi",
    #     "all",
    #     "--override-hdf",
    #     "--resultdir",
    #     # "results/S1_bonn_motion_dev_20221007-13:43:08",
    #     "results/S1_bonn_motion_dev_20221010-08:51:11",
    # ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        print("hi")
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
