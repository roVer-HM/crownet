#!/usr/bin/env python3
#
# Run script for setting up a single simulation run.
#
# When doing a simulation study (see "study" folder), this script is executed for
# each simulation run.

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
