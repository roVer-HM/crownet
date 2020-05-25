#!/usr/bin/python3
# import roveranalzer
# import logging # <-- runSim.py erzeuge logfile runSim.log
import os
import shutil
from datetime import datetime

import matplotlib.pyplot as plt
import pandas as pd

from runner.opprunner import BaseRunner, process_as


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args):
        super().__init__(working_dir, args)

    @staticmethod
    def get_degree_informed_dataframe(filepath):
        df_r = pd.read_csv(filepath, delimiter=" ", header=[0], comment="#")
        dt = 0.4  # one time frame = 0.4s
        df_r = df_r.iloc[249:, :]
        df_r["timeStep"] = (
            dt * df_r["timeStep"] - 100.0
        )  # information dissemination starts at 100s
        df_r.index = df_r.index - 249
        return df_r

    @process_as({"prio": 20, "type": "post"})
    def degree_informed_extract(self):
        filename = "DegreeInformed.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", filename)
        )
        # replace it with file extraction
        df_r = self.get_degree_informed_dataframe(filepath)
        df_r.to_csv(
            os.path.join(os.path.dirname(filepath), "DegreeInformed_extract.txt"),
            sep=" ",
        )

        # plot
        plt.plot(df_r.iloc[:, 0], df_r.iloc[:, 3])
        plt.axhline(y=0.95, c="r")
        plt.xlabel("Time [s] (Time = 0s : start of information dissemination)")
        plt.ylabel("Percentage of pedestrians informed [%]")
        plt.title("Information degree")
        plt.savefig(os.path.join(os.path.dirname(filepath), "InformationDegree.png"))

    @process_as({"prio": 10, "type": "post"})
    def time_95_informed(self):
        filename = "DegreeInformed.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", filename)
        )
        # replace it with file extraction
        df_r = self.get_degree_informed_dataframe(filepath)

        dt = 0.4
        time95 = 0.0
        for perc in df_r.iloc[:, 3]:
            if perc >= 0.95:
                break
            time95 += dt

        f = open(os.path.join(os.path.dirname(filepath), "Time95Informed.txt"), "x")
        f.write(f" timeToInform95PercentAgents\n0 {time95}")
        f.close()

    @process_as({"prio": -99, "type": "post"})
    def clean_dir(self):
        if not self.ns["keep"]:
            shutil.rmtree("vadere")
            shutil.rmtree("sumo")

            file_list = []
            for root, d_names, f_names in os.walk(os.getcwd()):
                for f in f_names:
                    file_list.append(os.path.join(root, f))

            for file_path in file_list:
                file_name = os.path.basename(file_path)
                if os.path.splitext(file_name)[0] not in self.ns["qoi"]:
                    os.remove(file_path)


if __name__ == "__main__":

    # runner = SimulationRun(os.getcwd(), sys.argv)
    runner = SimulationRun(
        "/home/sts/repos/rover-main/rover/simulations/mucFreiNetdLTE2dMulticast/",
        [
            "--qoi",
            "DegreeInformed_extract.txt",
            "Time95Informed.txt",
            "--config",
            "vadere01",
            "--keep",
            "--debug",
            "--experiment-label",
            datetime.now().isoformat().replace(":", "").replace("-", ""),
        ],
    )
    runner.run()
