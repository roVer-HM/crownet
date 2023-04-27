#!/usr/bin/env python3
import sys, os

sys.path.append(os.path.abspath(".."))

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)


if __name__ == "__main__":

    # TODO: discss Setting up network "crownet.simulations.networks.World"...
    # Initializing...
    # <!> Error: check_and_cast(): Cannot cast 'AmcPilot*' to type 'AmcPilotD2D *' -- in module (LteMacEnbD2D) World.eNB[0].cellularNic.mac (id=68), during network initialization
    # /home/christina/repos/crownet/crownet/src/run_crownet: line 27:    23 Segmentation fault      (core dumped) $DIR/CROWNET $COMMAND_LINE_OPTIONS $*
    # Container terminated (ERROR: 139).

    settings = [
        "vadere-opp-control",
        "--create-vadere-container",
        "--override-host-config",
        "--experiment-label",
        "output",
        "--with-control",
        "control.py",
        "--ctrl.controller-type",
        "OpenLoop",
        "--opp.-c",
        "final",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
