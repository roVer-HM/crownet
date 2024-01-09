#!/usr/bin/env python3
import sys, os

from os.path import join
from crownetutils.analysis.common import Simulation
from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
from crownetutils.analysis.omnetpp import HdfExtractor, OppAnalysis
from crownetutils.analysis.plot import PlotDpmMap, PlotEnb

from analysis.crownetutils.crownetutils.analysis.vadere import VadereAnalysis


class SimulationRun(BaseSimulationRunner):
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

    @process_as({"prio": 990, "type": "post"})
    def extract_hdf(self):
        result_dir, _, sql = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        # search config in sca file for apps extract packet loss
        # only for apps present.
        app_selector = sql.get_app_selector()

        if "beacon" in app_selector:
            HdfExtractor.extract_packet_loss(
                join(result_dir, "packet_loss.h5"),
                "beacon",
                sql,
                app=app_selector["beacon"],
            )
        if "map" in app_selector:
            HdfExtractor.extract_packet_loss(
                join(result_dir, "packet_loss.h5"), "map", sql, app=app_selector["map"]
            )
        HdfExtractor.extract_trajectories(join(result_dir, "trajectories.h5"), sql)

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

    @process_as({"prio": 970, "type": "post"})
    def create_common_plots(self):
        result_dir, builder, sql = OppAnalysis.builder_from_output_folder(
            data_root=self.result_base_dir()
        )
        PlotEnb.plot_served_blocks_ul_all(result_dir, builder, sql)
        if sql.is_count_map():
            print("build count based default plots")
            builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
            sel = list(OppAnalysis.find_selection_method(builder))
            if len(sel) > 1:
                print(f"multiple selections found: {sel}")
            PlotDpmMap.create_common_plots_density(
                result_dir, builder, sql, selection=sel[0]
            )
        else:
            print("build entropy map based plots")

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
