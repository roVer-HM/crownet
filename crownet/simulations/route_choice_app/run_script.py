#!/usr/bin/env python3
import sys, os
import time
sys.path.append(os.path.abspath(".."))

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner, process_as


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":


    settings = [
        'vadere-control',
        '--write-container-log',
        '--create-vadere-container',
        '--experiment-label',
        'output_avoidshort',
        '--with-control',
        'control.py',
        '--scenario-file',
        'vadere/scenarios/three_corridors.scenario',
        '--ctrl.controller-type',
        'AvoidShort',
        '--vadere-tag',
        'latest',
        '--control-tag',
        'latest',
        '--override-host-config',
        '--run-name',
        'test',
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()

    print("Write results to ./results/_vadere_controlled_output/")