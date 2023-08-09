#!/usr/bin/env python3
import sys, os

sys.path.append(os.path.abspath(".."))

from crownetutils.dockerrunner.simulationrunner import BaseSimulationRunner, process_as
import pandas as pd
import numpy as np

from crownetutils.analysis.common import Simulation
from crownetutils.analysis.omnetpp import OppAnalysis

import matplotlib
matplotlib.rcParams.update(matplotlib.rcParamsDefault)
import matplotlib.pyplot as plt


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    @process_as({"prio": 89, "type": "post"})
    def docker_stats(self):
        __ = self.wait_for_file(os.path.join(self.result_base_dir(), "container_opp_stats.out"))
        __ = self.wait_for_file(os.path.join(self.result_base_dir(), "container_vadere_stats.out"))
        __ = self.wait_for_file(os.path.join(self.result_base_dir(), "container_control_stats.out"))

        sim = Simulation.from_output_dir(self.result_base_dir())
        required_stats = ["container_vadere_stats.out", "container_opp_stats.out","container_control_stats.out"]
        df = sim.get_docker_stats_all(required=required_stats)

        df["Timestamp"] = df["Timestamp"] - df["Timestamp"].min()
        df["Execution time in s"] = df["Timestamp"].dt.seconds

        df = df.pivot_table(index="Execution time in s",columns="container", values=["DockerStatsCPUPerc", "DockerStatsRamGByte" ])
        df.to_csv(os.path.join(self.result_base_dir(), "docker_stats.csv" ), sep = " ")

        df["DockerStatsCPUPerc"].plot(marker= "o")
        plt.ylabel("CPU usage in CPUPer")
        plt.savefig(os.path.join(self.result_base_dir(), "DockerStatsCPUPerc.pdf"))
        plt.show(block=False)
        plt.close()

        df["DockerStatsRamGByte"].plot(marker= "o")
        plt.ylabel("RAM usage in GByte")
        plt.savefig(os.path.join(self.result_base_dir(), "RAMUsageGByte.pdf"))
        plt.show(block=False)
        plt.close()


    @process_as({"prio": 90, "type": "post"})
    def simsec_sec_ratio(self):

        opp_out = self.wait_for_file(os.path.join(self.result_base_dir(), "container_opp.out"))
        df = OppAnalysis.get_sim_real_time_ratio(omnetpp_log_file_path=opp_out)
        df.to_csv(os.path.join(self.result_base_dir(), "simsec_sec_ratio.csv"), sep = " ")
        df.set_index("sim_time",inplace=True)
        (1./df["ratio_sim_real"]).plot()
        plt.xlabel("Simulation time in s")
        plt.ylabel("Ratio: simulation time / real time ")
        plt.savefig(os.path.join(self.result_base_dir(), "RatioSimulationTimeRealTime.pdf"))
        plt.show(block=False)


    @process_as({"prio": 10, "type": "post"})
    def number_of_peds(self):
        filename = "numberOfPeds.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", filename),
        )

        df = pd.read_csv(filepath, sep = " ")
        df["timeStep"] = df["timeStep"] * 0.4  # timeStep to simulation time
        df.set_index(["timeStep"], inplace=True)
        df.index.name = "Simulation time in s"

        df.plot(legend=False)
        plt.ylabel("Number of agents")
        plt.savefig(os.path.join(self.result_base_dir(), "numberOfPeds.pdf"))
        plt.show(block=False)
        plt.close()

    @process_as({"prio": 20, "type": "post"})
    def path_choice(self):
        filename = "path_choice.txt"
        # wait for file
        filepath = self.wait_for_file(
            os.path.join(self.result_base_dir(), "flowcontrol.d", filename),
        )

        path_choice = pd.read_csv(filepath, sep=" ")
        path_choice["timeStep"] = path_choice["timeStep"] * 0.4
        bins = np.arange(0, path_choice["timeStep"].max(), 10)
        path_choice[path_choice["corridorRecommended"] != "skipped"]["timeStep"].hist(bins=bins)
        plt.xlabel("Simulation time in s")
        plt.ylabel("Number of times the long route is recommended")
        plt.savefig(os.path.join(self.result_base_dir(), "numberRecommendations.pdf"))
        plt.show(block=False)
        plt.close()

    @process_as({"prio": 30, "type": "post"})
    def densities_mapped(self):
        fp_densitites_true = self.wait_for_file(
            os.path.join(self.result_base_dir(), "vadere.d", "densities.txt"),
        )
        densities_true = pd.read_csv(fp_densitites_true, sep=" ")

        fp_densitites_meas = self.wait_for_file(
            os.path.join(self.result_base_dir(), "flowcontrol.d", "densities_mapped.txt"),
        )
        densities_meas = pd.read_csv(fp_densitites_meas, sep=" ")
        df = pd.merge(densities_true, densities_meas, on=["timeStep"])

        df["timeStep"] = df["timeStep"] * 0.4  # timeStep to simulation time
        df.set_index(["timeStep"], inplace=True)
        df.index.name = "Simulation time in s"


        areas = ["meas_area_1",
                 "meas_area_2",
                 "meas_area_3"]
        dataprocessor = ["areaDensityCountingNormed-PID101",
                         "areaDensityCountingNormed-PID102",
                         "areaDensityCountingNormed-PID103"]
        routes = ["Short route",
                  "Medium route",
                  "Long route"]

        for densitiy_estimate, density_ground_truth, route in zip(areas, dataprocessor, routes):
            a = densitiy_estimate
            b = density_ground_truth
            df[a].plot(label="Estimate", )
            df[b].plot(label="Ground truth", linestyle="dotted")
            plt.legend()
            plt.title(route)
            plt.ylabel("Pedestrian density in $ped/m^2$")
            # plt.ylim(0,0.5)
            plt.savefig(os.path.join(self.result_base_dir(), f"{route}.pdf"))
            plt.show(block=False)
            plt.close()

    @process_as({"prio": 40, "type": "post"})
    def packet_loss_heat_map(self):

        sim = Simulation(self.result_base_dir(),label="sim-1")

        lost_packet = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[1].app",
            vector_name="rcvdPktPerSrcCount:vector",
            value_name="lost_pack",
            with_host_id=True
        )

        lost_packet.to_csv(os.path.join(self.result_base_dir(), "packet_loss_heat_map.csv"), sep= " ")

        lost_packet["time"].hist(weights=lost_packet["lost_pack"])
        plt.xlabel("Simulation time in s")
        plt.ylabel("Number of lost packets")
        plt.title("Density map application")
        plt.savefig(os.path.join(self.result_base_dir(), "LostPacketsDensityMap.pdf"))
        plt.show(block=False)
        plt.close()

    @process_as({"prio": 50, "type": "post"})
    def packet_loss_info_diss(self):

        sim = Simulation(self.result_base_dir(),label="sim-1")

        no_lost_packets_info_diss = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[0].app",
            vector_name="rcvdPktPerSrcCount:vector",
            value_name="lost_pack",
            with_host_id=True
        )
        print(f"Packets lost with route recommendations:")
        print(no_lost_packets_info_diss)
        no_lost_packets_info_diss.to_csv(os.path.join(self.result_base_dir(), "packet_loss_info_diss.csv"), sep=" ")



    @process_as({"prio": 60, "type": "post"})
    def info_diss_time(self):

        sim = Simulation(self.result_base_dir(),label="sim-1")

        info_diss_time = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[0].app",
            vector_name="rcvdPkLifetime:vector",
            value_name="diss_time",
            with_host_id=True,
        )

        info_diss_time = info_diss_time[info_diss_time["diss_time"] > 0]  # remove self messages
        info_diss_time.to_csv(os.path.join(self.result_base_dir(), "info_diss_time.csv"), sep=" ")

        plt.scatter(x=info_diss_time["time"], y=info_diss_time["diss_time"])
        plt.xlabel("Simulation time in s")
        plt.ylabel("Dissemination time in s")
        plt.title("Route recommendation")
        plt.savefig(os.path.join(self.result_base_dir(), "DisseminationTimeRouteRecommendation.pdf"))
        plt.show(block=False)
        plt.close()

    @process_as({"prio": 70, "type": "post"})
    def heat_map_diss_time(self):

        sim = Simulation(self.result_base_dir(),label="sim-1")

        heatmap_diss_time = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[1].app",
            vector_name="rcvdPkLifetime:vector",
            value_name="diss_time",
            with_host_id=True,
        )
        heatmap_diss_time = heatmap_diss_time[heatmap_diss_time["diss_time"] > 0]
        plt.scatter(x=heatmap_diss_time["time"], y=heatmap_diss_time["diss_time"])

        heatmap_diss_time.to_csv(os.path.join(self.result_base_dir(), "heat_map_diss_time.csv"), sep=" ")
        plt.xlabel("Simulation time in s")
        plt.ylabel("Dissemination time in s")
        plt.title("Density map dissemination")
        plt.savefig(os.path.join(self.result_base_dir(), "DisseminationTimeDensityMap.pdf"))
        plt.show(block=False)
        plt.close()


    @process_as({"prio": 120, "type": "post"})
    def test_omnetpp_output(self):

        # used in the fingerprint test to check omnetpp results

        sim = Simulation(self.result_base_dir(),label="sim-1")

        heatmap = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[1].app",
            vector_name="rcvdPkLifetime:vector",
            value_name="lifetime",
            with_host_id=True,
        )
        beacon = OppAnalysis.get_rcvd_generic_vec_data(
            sql=sim.sql,
            module_name="World.pNode[%].app[2].app",
            vector_name="rcvdPkLifetime:vector",
            value_name="lifetime",
            with_host_id=True,
        )
        n = 200 # test the first 2000 rows
        heatmap.head(n=n).to_csv(os.path.join(self.result_base_dir(), "heatmap_diss_time.csv"), sep=" ", index=False)
        beacon.head(n=n).to_csv(os.path.join(self.result_base_dir(), "beacon_diss_time.csv"), sep=" ", index=False)


if __name__ == "__main__":


    settings = [
        "vadere-opp-control",
        "--write-container-log",
        "--create-vadere-container",
        "--override-host-config",
        "--experiment-label",
        "output",
        '--vadere-tag',
        'latest',
        '--control-tag',
        'latest',
        '--omnet-tag',
        'latest',
        "--with-control",
        "control.py",
        "--ctrl.controller-type",
        "AvoidShort",
        "--opp.-c",
        "final",
        "--qoi",
        "number_of_peds.txt",
        "path_choice.txt",
        "densities_mapped.txt",
        "packet_loss_info_diss.csv",
        "packet_loss_heat_map.csv",
        "heat_map_diss_time.csv",
        "info_diss_time.csv",
        "simsec_sec_ratio.csv",
        "docker_stats.csv",
        "--log-docker-stats",
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
