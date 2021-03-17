#!/usr/bin/python3
import sys, os

sys.path.append(os.path.abspath(".."))
from import_PYTHON_PATHS import *

from datetime import datetime

from roveranalyzer.simulators.rover.runner import BaseRunner


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":

    settings = [
        "--experiment-label",
        datetime.now().isoformat().replace(":", "").replace("-", ""),
        "--delete-existing-containers",
        "--create-vadere-container",
        "--with-control",
        "control.py",
        "--control-vadere-only",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
