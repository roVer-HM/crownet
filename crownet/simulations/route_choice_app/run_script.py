#!/usr/bin/env python3
import sys, os
import time
sys.path.append(os.path.abspath(".."))

from roveranalyzer.simulators.crownet.runner import BaseRunner


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":

    settings = [
        "vadere-control",
        "--create-vadere-container",
        "--override-host-config",
        "--experiment-label",
        "output",
        "--with-control",
        "control.py",
        "--scenario-file",
        "vadere/scenarios/simplified_default_sequential.scenario",
        "--ctrl.controller-type",
        "ClosedLoop",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()