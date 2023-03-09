#!/usr/bin/env python3
import sys, os

import pandas as pd
import numpy as np
from roveranalyzer.analysis.common import Simulation
from roveranalyzer.analysis.plot.app_misc import PlotAppMisc
from roveranalyzer.simulators.crownet.runner import (
    BaseRunner,
    process_as,
)
from matplotlib.backends.backend_pdf import PdfPages
from roveranalyzer.utils.plot import FigureSaverPdfPages, FigureSaverSimple
from roveranalyzer.analysis import VadereAnalysis, OppAnalysis, HdfExtractor
from roveranalyzer.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from roveranalyzer.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169

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


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

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
        _, builder, _ = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        # builder.set_imputation_strategy(DeleteMissingImputation())
        builder.build(self.ns.get("hdf_override", "False"))

    @process_as({"prio": 980, "type": "post"})
    def append_err_measure_hdf(self):
        try:
            sim = Simulation.from_suqc_result(data_root=self.result_base_dir())
        except ValueError:
            print(
                "No suqc context found. Try creating Simulation object without context. Some features of the Simulation analysis might be not supported."
            )
            sim = Simulation.from_output_dir(self.result_base_dir())
        OppAnalysis.append_err_measures_to_hdf(sim)

    @process_as({"prio": 975, "type": "post"})
    def create_rcvd_stats(self):
        sim = Simulation(self.result_base_dir(), label="")
        HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    @process_as({"prio": 970, "type": "post"})
    def create_common_plots(self):
        result_dir, builder, sql = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        os.makedirs(self.result_dir("fig_out"))
        with PdfPages(self.result_dir("fig_out/app_data.pdf")) as pdf:
            PlotEnb.plot_served_blocks_ul_all(
                self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
            )
            if sql.vector_exists(
                sql.m_beacon(), sql.OR(["txInterval:vector", "txInterval:vector"])
            ):
                PlotAppTxInterval.plot_txinterval_all(
                    self.result_base_dir(),
                    sql=sql,
                    app="Beacon",
                    saver=FigureSaverPdfPages(pdf),
                )

            if sql.vector_exists(
                sql.m_map(), sql.OR(["txInterval:vector", "txInterval:vector"])
            ):
                PlotAppTxInterval.plot_txinterval_all(
                    self.result_base_dir(),
                    sql=sql,
                    app="Map",
                    saver=FigureSaverPdfPages(pdf),
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
        result_dir, builder, sql = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        sim = Simulation(self.result_base_dir(), label="")
        saver = FigureSaverSimple(
            override_base_path=self.result_dir("fig_out"), figure_type=".png"
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

    @process_as({"prio": 960, "type": "post"})
    def remove_density_map_csv(self):
        _, builder, _ = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        for f in builder.map_paths:
            os.remove(f)

    @process_as({"prio": 900, "type": "post"})
    def vadere_position(self):
        path = self.result_dir("vadere.d/numAgents.csv")
        if os.path.exists(path):
            df = VadereAnalysis.read_time_step(path)
            VadereAnalysis.plot_number_agents_over_time(
                data=df, savefig=self.result_dir("vadere.d/numAgents.pdf")
            )


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
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
