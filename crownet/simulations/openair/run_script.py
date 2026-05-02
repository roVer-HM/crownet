#!/usr/bin/env python3
import os
import sys

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":
    settings = []

    if len(sys.argv) == 1:
        runner = SimulationRun(os.getcwd(), settings)
    else:
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
