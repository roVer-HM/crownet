#!/usr/bin/env python3
import sys, os

sys.path.append(os.path.abspath(".."))


from datetime import datetime

from roveranalyzer.simulators.crownet.runner import BaseRunner


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":


    settings = [
        "vadere-opp-control",
        "--create-vadere-container",
        "--override-host-config",
        "--experiment-label",
        "output",
        "--opp.-c",
        "test_control001_aid",
        "--with-control",
        "control_1.py"
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
