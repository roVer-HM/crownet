#!/usr/bin/python3
import sys, os

sys.path.append(os.path.abspath(".."))


from datetime import datetime

from roveranalyzer.simulators.crownet.runner import BaseRunner


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)




if __name__ == "__main__":

    settings = [
        "--experiment-label",
        datetime.now().isoformat().replace(":", "").replace("-", ""),
        #"--delete-existing-containers",
        #"--create-vadere-container",
        #"--with-control",
        #"control.py",
        #"--config",
        #"final_control"
        #"--control-vadere-only",
        #"--use-timestep-label",
        "--vadere-only",
        "--scenario-file",
        os.path.join(os.getcwd(), "vadere/scenarios/test001.scenario")
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
