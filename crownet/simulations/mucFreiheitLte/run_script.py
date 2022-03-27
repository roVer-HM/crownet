#!/usr/bin/env python3
import shutil
import sys, os

from os.path import join
from datetime import datetime
from roveranalyzer.analysis.common import Simulation
from roveranalyzer.simulators.crownet.runner import BaseRunner, process_as, result_dir_with_opp
import roveranalyzer.simulators.crownet.dcd as DensityMap
import roveranalyzer.simulators.opp as OMNeT
from roveranalyzer.analysis import OppAnalysis, HdfExtractor
from roveranalyzer.utils.general import Project



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
        _, builder, _ = OppAnalysis.builder_from_output_folder(data_root=self.result_base_dir())
        builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        builder.build(self.ns.get("hdf_override", "False"))

    @process_as({"prio": 990, "type": "post"})
    def create_common_plots(self):
        result_dir, builder, sql = OppAnalysis.builder_from_output_folder(data_root=self.result_base_dir())
        builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        sel = list(OppAnalysis.find_selection_method(builder))
        if len(sel) > 1:
            print(f"multiple selections found: {sel}")
        OppAnalysis.create_common_plots(result_dir, builder, sql, selection=sel[0])

    @process_as({"prio": 980, "type": "post"})
    def extract_hdf(self):
        result_dir, _, sql = OppAnalysis.builder_from_output_folder(data_root=self.result_base_dir())
        HdfExtractor.extract_packet_loss(join(result_dir, "packet_loss.h5"), "beacon", sql, app=sql.m_app0())
        HdfExtractor.extract_packet_loss(join(result_dir, "packet_loss.h5"), "map", sql, app=sql.m_app1())
        HdfExtractor.extract_trajectories(join(result_dir, "trajectories.h5"), sql)

    @process_as({"prio": 970, "type": "post"})
    def append_hdf(self):
        sim = Simulation.from_suqc_result(data_root=self.result_base_dir())
        OppAnalysis.append_count_diff_to_hdf(sim)
    
    @process_as({"prio": 960, "type": "post"})
    def remove_denisty_map_csv(self):
        _, builder, _ = OppAnalysis.builder_from_output_folder(data_root=self.result_base_dir())
        for f in builder.map_paths:
            os.remove(f)

    # @process_as({"prio": 970, "type": "post"})
    # def abs_err(self):
    #     result_dir, builder, sql = OppAnalysis.builder_from_output_folder(data_root=self.result_base_dir())
    #     builder.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
    #     sel = list(OppAnalysis.find_selection_method(builder))
    #     if len(sel) > 1:
    #         print(f"multiple selections found: {sel}")
    #     dcd = builder.build_dcdMap(selection=sel[0])
    #     dcd.plot_count_diff()
    #     print("foo")





        
    


if __name__ == "__main__":

    settings = [

    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
