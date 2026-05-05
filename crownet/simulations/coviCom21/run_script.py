#!/usr/bin/env python3
import sys, os

sys.path.append(os.path.abspath(".."))


from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    def run(self):
        import glob
        import os
        import shutil
        res_dir = self.ns["opp_args"].get_value("--result-dir")
        if not res_dir:
            res_dir = self.ns.get("result_dir")
            if not res_dir:
                res_dir = self.result_dir()
        if os.path.exists(res_dir):
            for f in glob.glob(os.path.join(res_dir, "*.db")) + glob.glob(os.path.join(res_dir, "*.csv")):
                try:
                    os.remove(f)
                except Exception:
                    pass
        super().run()
        
        sim_res_dir = self.result_dir()
        if res_dir and res_dir != sim_res_dir and os.path.exists(res_dir) and os.path.exists(sim_res_dir):
            for f in glob.glob(os.path.join(res_dir, "*.db")) + glob.glob(os.path.join(res_dir, "*.csv")):
                try:
                    shutil.move(f, os.path.join(sim_res_dir, os.path.basename(f)))
                except Exception:
                    pass


if __name__ == "__main__":

    settings = [
        "vadere-opp",
        "--create-vadere-container",
        "--override-host-config",
        "--opp.-c",
        "vadere_120",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
