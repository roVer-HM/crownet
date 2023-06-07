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
        'vadere-control',
        '--create-vadere-container',
        '--experiment-label',
        'output',
        '--with-control',
        'control.py',
        '--scenario-file',
        'vadere/scenarios/route_choice_real_world.scenario', #TODO: test 'vadere/scenarios/route_choice_real_world.scenario' or 'vadere/scenarios/three_corridors.scenario'
        '--ctrl.controller-type',
        'ClosedLoop', #TODO: 'NoController' or 'ClosedLoop' or 'OpenLoop'
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