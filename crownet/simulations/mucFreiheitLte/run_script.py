#!/usr/bin/env python3
import sys, os

from datetime import datetime
from roveranalyzer.simulators.crownet.runner import BaseRunner, process_as, result_dir_with_opp
import roveranalyzer.simulators.crownet.dcd as DensityMap
import roveranalyzer.simulators.opp as OMNeT
from roveranalyzer.analysis import OppAnalysis
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
        result_dir = self.result_base_dir()
        builder = DensityMap.DcdHdfBuilder.get(
            hdf_path="data.h5",
            source_path=result_dir
        ).epsg(Project.UTM_32N)
        builder.build(override_hdf=True)
        #todo delete csv files
        
    
    @process_as({"prio": 100, "type": "post"})
    def create_figures(self):
        result_dir = self.result_base_dir() 
        fig_dir = os.path.join(result_dir, "fig")
        os.makedirs(fig_dir, exist_ok=True)
        vec_name = "vars_rep_0.vec"
        sca_name = "vars_rep_0.sca"
        sql = OMNeT.CrownetSql(
            vec_path=os.path.join(result_dir, vec_name),
            sca_path=os.path.join(result_dir, sca_name),
            network="World"
        )
        builder = DensityMap.DcdHdfBuilder.get(
            hdf_path="data.h5",
            source_path=result_dir
        ).epsg(Project.UTM_32N)
        dcd = builder.build_dcdMap(selection="ymfPlusDist")

        # Node count over time
        fig, ax = dcd.plot_count_diff()
        ax.set_ylim([20, 110])
        ax.set_xlim([0, 35])
        fig.savefig(f"{fig_dir}/count_over_time.png")
        fig.savefig(f"{fig_dir}/count_over_time.pdf")

        # Error hist
        fig, ax = dcd.plot_error_histogram(
            value="sqerr",
            stat="percent"
        )
        ax.set_ylim([0, 40])
        ax.set_xlim([-1.5, 1.0])
        fig.savefig(f"{fig_dir}/error_histogram.png")
        fig.savefig(f"{fig_dir}/error_histogram.pdf")

        # Neighborhood size over time
        data, idx = OppAnalysis.get_neighborhood_table_size(sql)
        fig, ax = OppAnalysis.plot_neighborhood_table_size_over_time(
            data, idx 
        )
        ax.set_ylim([0, 110])
        ax.set_xlim([0, 35])
        fig.savefig(f"{fig_dir}/neighborhood_table_size.png")
        fig.savefig(f"{fig_dir}/neighborhood_table_size.pdf")


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
