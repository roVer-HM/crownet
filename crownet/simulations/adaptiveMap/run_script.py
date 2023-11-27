#!/usr/bin/env python3
import sys, os
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType
from crownetutils.analysis.dpmm.imputation import (
    DeleteMissingArbitraryGlobalValueForImagined,
    FullRsdImputation,
    ImputationStream,
    OwnerPositionImputation,
)

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

from crownetutils.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169

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
        module_vectors=["misc"],
    )
    return density_cfg


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    def get_cfg(self):
        return get_density_cfg(self.result_base_dir())

    def get_builder(self):

        cfg = get_density_cfg(self.result_base_dir())
        sim: Simulation = Simulation.from_dpmm_cfg(cfg)
        sim.sql.append_index_if_missing()

        b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
        b.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        rsd_origin_position = sim.sql.get_resource_sharing_domains(
            apply_offset=False, bottom_left_origin=True
        )
        stream = ImputationStream()
        # stream.append(ArbitraryValueImputation(fill_value=0.0))
        stream.append(DeleteMissingArbitraryGlobalValueForImagined(glb_fill_value=0.0))
        # stream.append(DeleteMissingImputation())
        stream.append(FullRsdImputation(rsd_origin_position=rsd_origin_position))
        stream.append(OwnerPositionImputation())
        b.set_imputation_strategy(stream)

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

    @process_as({"prio": 1000, "type": "post"})
    def build_sql_index(self):
        cfg = self.get_cfg()
        sim = Simulation.from_dpmm_cfg(cfg)
        sim.sql.append_index_if_missing()

    @process_as({"prio": 999, "type": "post"})
    def build_hdf(self):
        builder = self.get_builder()
        builder.build(self.ns.get("hdf_override", "False"))

    @process_as({"prio": 980, "type": "post"})
    def append_err_measure_hdf(self):
        sim = Simulation.from_dpmm_cfg(self.get_cfg(), label="")
        OppAnalysis.append_err_measures_to_hdf(sim)

    @process_as({"prio": 975, "type": "post"})
    def create_rcvd_stats(self):
        sim = Simulation.from_dpmm_cfg(self.get_cfg(), label="")
        HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    @process_as({"prio": 970, "type": "post"})
    def create_common_plots(self):
        result_dir, builder, sql = OppAnalysis.builder_from_cfg(self.get_cfg())
        os.makedirs(self.result_dir("fig_out"), exist_ok=True)
        with PdfPages(self.result_dir("fig_out/app_data.pdf")) as pdf:
            PlotEnb.plot_served_blocks_ul_all(
                self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
            )

        if sql.is_count_map():
            print("build count based default plots")
            builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
            PlotDpmMap.create_common_plots_density(
                result_dir, builder, sql, selection=builder.get_selected_alg()
            )
        else:
            print("build entropy map based plots")

    @process_as({"prio": 965, "type": "post"})
    def add_plots(self):
        sim = Simulation.from_dpmm_cfg(self.get_cfg(), label="")
        saver = FigureSaverSimple(
            override_base_path=self.result_dir("fig_out"), figure_type=".png"
        )
        # agent count data
        print("app misc")
        PlotAppMisc.plot_nt_map_comparison(sim, saver=saver)

        # application data
        PlotAppMisc.plot_system_level_tx_rate_based_on_application_layer_data(
            sim=sim, saver=saver
        )

        # app tx data
        PlotAppTxInterval.plot_txinterval_all_beacon(
            data_root=sim.data_root, sql=sim.sql, saver=saver
        )
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
    #     _, builder, _ = OppAnalysis.builder_from_output_folder(
    #         data_root=self.result_base_dir()
    #     )
    #     for f in builder.map_paths:
    #         os.remove(f)


if __name__ == "__main__":

    settings = []

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
