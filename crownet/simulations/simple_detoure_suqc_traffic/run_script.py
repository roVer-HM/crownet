#!/usr/bin/python3
import os
import sys

sys.path.append(os.path.abspath(".."))

import numpy

import pandas as pd

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner, process_as

from crownetutils.analysis.common import Simulation
from crownetutils.analysis.omnetpp import OppAnalysis

import matplotlib
matplotlib.rcParams.update(matplotlib.rcParamsDefault)
import matplotlib.pyplot as plt


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    @staticmethod
    def get_degree_informed_dataframe(filepath):
        df_r = pd.read_csv(filepath, delimiter=" ", header=[0], comment="#")
        dt = 0.4  # one time frame = 0.4s
        timeStep = 249
        df_r = df_r.iloc[timeStep:, :]
        df_r["timeStep"] = (
            dt * df_r["timeStep"] - 100.0
        )  # information dissemination starts at 100s
        df_r.set_index("timeStep", drop=True, inplace=True)
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
            os.path.join(self.result_base_dir(), "degree_informed_extract.txt"),
            sep=" ",
        )


        self.wait_for_file(
            os.path.join(self.result_base_dir(), "degree_informed_extract.txt")
        )

        # plot
        df_r["percentageInformed-PID12"].plot()
        plt.axhline(y=0.95, c="r")
        plt.xlabel("Time [s] (Time = 0s : start of information dissemination)")
        plt.ylabel("Percentage of pedestrians informed [%]")
        plt.title("Information degree")
        plt.savefig(os.path.join(self.result_base_dir(), "vadere.d", "InformationDegree.pdf"))


    @process_as({"prio": 10, "type": "post"})
    def time_95_informed(self):
        filename = "DegreeInformed.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", filename),
        )

        # replace it with file extraction
        df_r = self.get_degree_informed_dataframe(filepath)

        df_r = df_r[df_r['percentageInformed-PID12'] >= 0.95]

        if len(df_r) > 0:
            time95 = df_r.index.min()
        else:
            time95 = numpy.inf

        with open(os.path.join(self.result_base_dir(), "time_95_informed.txt"), "w") as f:
            f.write(f"index timeToInform95PercentAgents\n0 {time95}")

        self.wait_for_file(
            os.path.join(self.result_base_dir(), "time_95_informed.txt")
        )



    @process_as({"prio": 30, "type": "post"})
    def poisson_parameter(self):
        print("Poisson write")
        filename = "numberPedsGen.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", filename),
        )

        # replace it with file extraction
        df_r = pd.read_csv(filepath, delimiter=" ", header=[0], comment="#")

        poisson_parameter = numpy.mean(df_r.iloc[:, 1])

        with open(os.path.join(self.result_base_dir(), "vadere.d" , "poisson_parameter.txt"), "w") as f:
            f.write(f" PoissonParameter\n0 {poisson_parameter}")

        self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d" , "poisson_parameter.txt"),
        )

    @process_as({"prio": 50, "type": "post"})
    def number_of_peds(self):

        file_path = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", "numberPedsInSimulation.txt"),
        )

        df_r = pd.read_csv(file_path, delimiter=" ", header=[0], comment="#")
        timeStep = 249
        df_r = df_r.iloc[timeStep:, :]

        df_r = df_r["NumPeds-PID104"].describe()
        peds = pd.DataFrame(df_r).transpose().reset_index()

        file_path_new = os.path.join(self.result_base_dir(), "vadere.d" , "number_of_peds.txt")
        peds.to_csv(file_path_new, sep=" ")

    @process_as({"prio": 40, "type": "post"})
    def packet_age(self):
        filename = "packet_age.txt"
        # wait for files
        self.wait_for_file(
            os.path.join(self.result_base_dir(), "vars_rep_0.vec"),
        )
        self.wait_for_file(
            os.path.join(self.result_base_dir(), "vars_rep_0.sca"),
        )

        sim = Simulation(self.result_base_dir(), label="sim-1")

        packet_age = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World_Wlan.pNode[%].app[0]",
            vector_name="rcvdPkLifetime:vector",
            value_name="lifetime"
        )

        packet_age = packet_age["lifetime"].describe()
        packet_age = pd.DataFrame(packet_age).transpose().reset_index()

        packet_age.to_csv(os.path.join(self.result_base_dir(),filename), sep=" ")

        self.wait_for_file(
            os.path.join(self.result_base_dir(),filename),
        )



if __name__ == "__main__":

    settings = [
                "vadere-opp",
                "--write-container-log",
                "--qoi",
                "degree_informed_extract.txt",
                "poisson_parameter.txt",
                "time_95_informed.txt",
                "packet_age.txt",
                "number_of_peds.txt",
                "--create-vadere-container",
                "--override-host-config",
                "--run-name",
                "Sample__0_0",
                "--experiment-label",
                "output",
            ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()

