import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from oppanalyzer import Config, ScaveTool, StatsTool, OaiMeasurement
from typing import List

"""
The Simulation class specifies the information required to read and plot a simulation scenario.

:param id:          Unique ID of the scenario
:param path:        Path to all *.vec and *.sca files produced by the scenario
:param description: Human readable description used in plot legend for this scenario
"""


class Simulation(object):
    def __init__(self, id: str, path: str, description: str):
        self.id = id
        self.path = path
        self.desc = description

"""
PlotAttrs is a singleton guaranteeing unique plot parameters
"""
class PlotAttrs:
    class __PlotAttrs:
        plot_marker = [".", "*", "o", "v", "1", "2", "3", "4"]
        plot_color = ["b", "g", "r", "c", "m", "y", "k", "w"]

        def __init__(self):
            pass

    instance: object = None

    def __init__(self):
        if not PlotAttrs.instance:
            PlotAttrs.instance = PlotAttrs.__PlotAttrs()
        self.idx_m = -1
        self.idx_c = -1

    def __getattr__(self, name):
        return getattr(self.instance, name)

    def get_marker(self) -> str:
        ret = self.instance.plot_marker[self.idx_m]
        self.idx_m += 1
        if self.idx_m >= len(self.instance.plot_marker):
            self.idx_m = 0
        return ret

    def get_color(self) -> str:
        ret = self.instance.plot_color[self.idx_c]
        self.idx_c += 1
        if self.idx_c >= len(self.instance.plot_color):
            self.idx_c = 0
        return ret

    def reset(self):
        self.idx_c = 0;
        self.idx_m = 0;

# Type definitions
SimList = List[Simulation]

# Default parameter values
STARTUP_TIME = 60
DEFAULT_NR_BINS = 100
OUTPUT_FOLDER = "plots"

cnf = Config()
att = PlotAttrs()

"""
Process a specific simulation scenario

:param name:            Name of the scenario
:param id:              Unique short ID of this scenario
:param measure_subpath: Location of measurement data
:param sims:            List of simulations (tuples of (sim_subpath: str, sim_description: str))
"""


# def process_scenario(name: str, id: str, sim_subpath: str, measure_subpath: str, **kwargs):
def process_scenario(name: str, id: str, measure_subpath: str, sims: SimList, **kwargs):
    # %% initialize
    std_figsize = [6.4, 2.4]  # standard size of figure
    scave = ScaveTool()
    # %% Read simulation data of all simulations within specified list of simulations
    alertdelay_ue = {}  # dictionary for storing the delays for alerts (application layer)
    for sim in sims:
        input_sca = f"{cnf.rover_main}/rover/simulations/openair/results/{sim.path}/*.sca"
        input_vec = f"{cnf.rover_main}/rover/simulations/openair/results/{sim.path}/*.vec"
        output_csv = f"{cnf.rover_main}/rover/simulations/openair/results/results_{sim.id}.csv"
        print(f"Converting {input_sca} and {input_vec} to {output_csv}")
        csv = scave.create_or_get_csv_file(
            csv_path=output_csv,
            input_paths=[
                input_sca,
                input_vec,
            ],
            scave_filter=None,
            override=True,
            recursive=True,
        )
        df = scave.load_csv(csv)

        alertdelay_ue[sim.id] = df.opp.filter().vector().name_regex("alertDelay.*").apply().iloc[0]

        # consider only values after the startup-time
        valid_values = alertdelay_ue[sim.id].vectime > STARTUP_TIME
        alertdelay_ue[sim.id].vectime = alertdelay_ue[sim.id].vectime[valid_values]
        alertdelay_ue[sim.id].vecvalue = alertdelay_ue[sim.id].vecvalue[valid_values]

    # %% Read measurement data
    input_trace = f"{cnf.rover_main}/rover/simulations/openair/measurements/{measure_subpath}"
    print(f"Reading measurement data: {input_trace}")
    camdelay_ue_measure = OaiMeasurement.read_packet_trace(input_trace)

    # consider only values after the startup-time
    valid_values = camdelay_ue_measure.vectime > STARTUP_TIME
    camdelay_ue_measure.vectime = camdelay_ue_measure.vectime[valid_values]
    camdelay_ue_measure.vecvalue = camdelay_ue_measure.vecvalue[valid_values]

    # use subplots:
    # fig_adelay, axes = plt.subplots(nrows=2)
    # ax_time = axes[0]
    # ax_hist = axes[1]

    # use individual figures:
    figures = {
        "delay": plt.figure(figsize=std_figsize),
        "delay_hist": plt.figure(figsize=std_figsize),
        "delay_cdf": plt.figure(figsize=std_figsize),
        "iat_hist": plt.figure(figsize=std_figsize)
    }

    axes = {
    }

    # create axes for all figures
    for name, fig in figures.items():
        plt.figure(fig.number)
        axes[name] = plt.axes()

    # %% statistics
    print(StatsTool.stats_table(camdelay_ue_measure.vecvalue, "s", f"{id} (measurement): CAM Delay  (time_difference)"))
    for sim in sims:
        print(StatsTool.stats_table(alertdelay_ue[sim.id].vecvalue, "s",
                                    f"{sim.id} (simulation): CAM Delay  (alertDelay)"))

    # %% figure: delay over time
    att.reset()
    # Process measurement data
    df.opp.plot.create_time_series(axes["delay"], camdelay_ue_measure, label="measurement", color=att.get_color(),
                                   marker=att.get_marker())

    # Process corresponding simulation data
    for sim in sims:
        df.opp.plot.create_time_series(axes["delay"], alertdelay_ue[sim.id], label=sim.desc, color=att.get_color(),
                                   marker=att.get_marker())
    axes["delay"].legend(loc='lower right')
    axes["delay"].set_title(f"{id}: Packet Delay")
    axes["delay"].set_ylabel("delay [s]");
    axes["delay"].legend(loc='lower right')
    axes["delay"].set_ylim(bottom=0)

    # %% figure: delay histogram
    att.reset()
    #  measurements
    hist_add_args = kwargs

    if "hist_range" in hist_add_args:
        hist_add_args["range"] = hist_add_args.pop("hist_range")

    (n, bins, patches) = df.opp.plot.create_histogram(axes["delay_hist"], camdelay_ue_measure, label="measurement",
                                                      color=att.get_color(), density=False, histtype='step', fill=False,
                                                      bins=DEFAULT_NR_BINS, **hist_add_args)

    # simulation
    for sim in sims:
        df.opp.plot.create_histogram(axes["delay_hist"], alertdelay_ue[sim.id], label=sim.desc, color=att.get_color(),
                                     density=False,
                                     histtype='step', fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["delay_hist"].set_title(f"{id}: Packet Delay Histogram")
    axes["delay_hist"].set_xlabel("delay [s]");
    axes["delay_hist"].set_ylabel("number of values");
    axes["delay_hist"].legend(loc='upper center')

    # %% figure: delay histogram cumulative
    att.reset()
    hist_add_args = kwargs

    if "hist_range" in hist_add_args:
        hist_add_args["range"] = hist_add_args.pop("hist_range")

    # measurement
    (n, bins, patches) = df.opp.plot.create_histogram(axes["delay_cdf"], camdelay_ue_measure, label="measurement",
                                                      color=att.get_color(), density=True, cumulative=True, histtype='step',
                                                      fill=False, bins=DEFAULT_NR_BINS, **hist_add_args)

    # simulation
    for sim in sims:
        df.opp.plot.create_histogram(axes["delay_cdf"], alertdelay_ue[sim.id], label=sim.desc, color=att.get_color(), density=True,
                                     histtype='step', cumulative=True, fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["delay_cdf"].set_title(f"{id}: Packet Delay CDF")
    axes["delay_cdf"].set_xlabel("delay [s]");
    axes["delay_cdf"].set_ylabel("P(X<x)");
    axes["delay_cdf"].legend(loc='lower center')

    # %% figure: packet inter-arrival time histogram
    att.reset()
    # calculate inter-arrival times: measurements
    nr_values = len(camdelay_ue_measure['vecvalue']) - 1
    iat_time = np.zeros(nr_values)
    iat_value = np.zeros(nr_values)
    for i, time in enumerate(iat_value):
        iat_time[i] = (camdelay_ue_measure['vectime']).iloc[i]
        iat_value[i] = ((camdelay_ue_measure['vectime']).iloc[i + 1] - (camdelay_ue_measure['vectime']).iloc[i]) * 1000

    iat = pd.Series({
        "vectime": iat_time,
        "vecvalue": iat_value
    })

    hist_add_args = kwargs

    if "range" in hist_add_args:
        hist_add_args.pop("range")

    # TODO: should be dynamic
    # hist_add_args["range"] = [0.0, 200]

    (n, bins, patches) = df.opp.plot.create_histogram(axes["iat_hist"], iat, label="measurement",
                                                      color=att.get_color(), density=False, histtype='step', fill=False,
                                                      bins=DEFAULT_NR_BINS,
                                                      **hist_add_args)

    # simulation
    for sim in sims:
        # calculate inter-arrival times: simulation
        nr_values = len(alertdelay_ue[sim.id]['vecvalue']) - 1
        iat_time = np.zeros(nr_values)
        iat_value = np.zeros(nr_values)
        for i, time in enumerate(iat_value):
            iat_time[i] = (alertdelay_ue[sim.id]['vectime'])[i]
            iat_value[i] = ((alertdelay_ue[sim.id]['vectime'])[i + 1] - (alertdelay_ue[sim.id]['vectime'])[i]) * 1000

        iat = pd.Series({
            "vectime": iat_time,
            "vecvalue": iat_value
        })

        df.opp.plot.create_histogram(axes["iat_hist"], iat, label=sim.desc, color=att.get_color(), density=False,
                                     histtype='step', fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["iat_hist"].set_title(f"{id}: Packet Inter-Arrival Time Histogram")
    axes["iat_hist"].set_xlabel("inter-arrival time [ms]");
    axes["iat_hist"].set_ylabel("number of values");
    axes["iat_hist"].legend()

    # %% create figures in PDF and PNG formats
    plt.show()
    try:
        os.makedirs(OUTPUT_FOLDER)
    except OSError as e:
        if e.errno != os.errno.EEXIST:
            raise

    for name, fig in figures.items():
        filename_base = OUTPUT_FOLDER + os.path.sep + id + "_" + name
        print(f"Saving figure \"{filename_base}.pdf\"")
        fig.savefig(f"{filename_base}.pdf", bbox_inches='tight')
        print(f"Saving figure \"{filename_base}.png\"")
        fig.savefig(f"{filename_base}.png")


# %% Process all scenarios

# Scenario 1: UDP 300 B CAM Packets, no other traffic
simList = [Simulation("S1.UA", "S1-UA", "simulation (default model)"),
           Simulation("S1", "S1", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 1", "S1", "S1/2020-01-31_16-53-24_cam_receive_log.csv", simList, range=[0.0, 0.015]);

# Scenario 1.25: UDP 300 B CAM Packets, no other traffic, 25 RBs only
simList = [Simulation("S1.25.UA", "S1-25-UA", "simulation (default model)"),
           Simulation("S1.25", "S1-25", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 1.25 (25 RBs)", "S1.25",
                    "S1/25RBs/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", simList, range=[0.0,0.015]);

# Scenario 2: UDP 300 B CAM Packets, overload due to 512B background traffic
# process_scenario("Scenario 2", "S2", "OpenAirInterface-CAM-DL-2-1UE", "S2/2020-01-31_16-59-44_cam_receive_log.csv", range=[2.0,3.0])
simList = [Simulation("S2.UA", "S2-UA", "simulation (default model)"),
           Simulation("S2", "S2", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 2", "S2",
                     "S2/2020-01-31_16-59-44_cam_receive_log.csv", simList, range=[0.0,1.2]);

# Scenario 2.25: UDP 300 B CAM Packets, overload due to 512B background traffic, 25 RBs only
# process_scenario("Scenario 2 (25 RBs)", "S2.25", "OpenAirInterface-CAM-DL-2-1UE", "S2/25RBs/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv", range=[2.0,3.0])
simList = [Simulation("S2.25.UA", "S2-25-UA", "simulation (default model)"),
           Simulation("S2.25", "S2-25", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 2.25 (25 RBs)", "S2.25",
                     "S2/25RBs/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv", simList,
                     range=[0.0,2.4])

# Scenario 3: UE-2-UE Traffic (UE1 sends via uplink to eNodeB, eNodeB sends to UE0), low-rate UDP traffic
# measurement 5: 2020-01-31_15-39-18_cam_receive_log.csv
# measurement 6: 2020-01-31_15-44-49_cam_receive_log.csv
simList = [Simulation("S3.UA", "S3-UA", "simulation (default model)"),
           Simulation("S3", "S3", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 3", "S3",
                     "S3/Measurement5/2020-01-31_15-39-18_cam_receive_log.csv", simList, range=[0.0,0.2])

# Scenario 4: UE-2-UE Traffic (UE1 sends via uplink to eNodeB, eNodeB sends to UE0), overload due to background traffic
# 2020-01-31_16-13-32_cam_receive_log.csv
simList = [Simulation("S4.UA", "S4-UA", "simulation (default model)"),
           Simulation("S4", "S4", "simulation (adapted model)")]

if True:
    process_scenario("Scenario 4", "S4",
                     "S4/2020-01-31_16-13-32_cam_receive_log.csv", simList)

# possible further datagram types
# - CAM data rate (bit/s)  (info already available in inter-arrival plots)
# - background traffic data rate (bit/s)

