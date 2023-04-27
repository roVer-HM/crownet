#!/usr/bin/env python3
import sys, os
import time

sys.path.append(os.path.abspath(".."))

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":

    settings = [
        "--port",
        "9999",
        "--host-name",
        "localhost",
        "--client-mode",
        "--start-server",
        "--gui-mode",
        "--output-dir",
        "sim-output-task1",
        "-j",
        "/home/christina/repos/crownet/vadere/VadereManager/target/vadere-server.jar",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
