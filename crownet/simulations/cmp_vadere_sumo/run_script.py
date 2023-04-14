#!/usr/bin/env python3
#
# Run script for setting up a single simulation run.
#
# When doing a simulation study (see "study" folder), this script is executed for
# each simulation run.

import sys, os

from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
from crownetutils.omnetpp.scave import CrownetSql


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    # todo: add pre and post processing to apply for each simulation run.
    # Post-processing might include result striping or aggregation.
    # add as many methods as needed and add the process_as annotation.
    # Use `prio` to ensure execution order (Bigger number higher prio)

    # @process_as({"prio": 20, "type": "post"})
    # def foo(self):
    # pass

    # @process_as({"prio": 10, "type": "pre"})
    # def bar(self):
    # pass

    # Example for Post-Processing
    # ===========================
    # Note: currently disabled by commenting out the annotation since we do all analysis parts later in study.py
    # @process_as({"prio": 40, "type": "post"})
    def packet_age(self):
        filename = "rcvdPkLifetime.txt"
        data_root = self.result_base_dir()

        sql = CrownetSql(
            vec_path=f"{data_root}/vars_rep_0.vec",
            sca_path=f"{data_root}/vars_rep_0.sca",
            network="World",
        )

        print(f"module vectors: {sql.module_vectors}")

        # filepath = os.path.join(self.result_base_dir(), "vadere.d", filename)

        # data_raw, data_desc = OppAnalysis.get_received_packet_delay(
        #     sql=sql,
        #     module_name="World.pNode[%].app[0].app"
        # )

        # data_raw.to_csv(os.path.join(self.result_base_dir(), filename), sep=" ")
        # data_desc.to_csv(os.path.join(self.result_base_dir(), 'stats_'+filename), sep=" ")


if __name__ == "__main__":

    # default settings if script is started directly and not as part of a simulation study
    settings = [
        "vadere-opp",
        "--qoi",
        "all",
        "--create-vadere-container",
        "--override-host-config",
        "--run-name",
        "Sample__0_0",
        "--resultdir",
        "./results",
        "--opp.-c",
        "vadereOnly2",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        print(f"starting with default settings: {settings}")
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
