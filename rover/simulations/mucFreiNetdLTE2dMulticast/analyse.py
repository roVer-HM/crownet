import os
import errno
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from roveranalyzer.oppanalyzer.utils import Config, ScaveTool, StatsTool
from roveranalyzer.oppanalyzer.rover_measurements import OaiMeasurement
from typing import List, Dict

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

    def reset(self) -> object:
        self.idx_c = 0;
        self.idx_m = 0;


# Type definitions
SimList = List[Simulation]

# Default parameter values
STARTUP_TIME = 0
DEFAULT_NR_BINS = 100
OUTPUT_FOLDER = "plots"
SET_TITLE = False


cnf = Config()
att = PlotAttrs()
SIM_RESULT_BASE = f"{cnf.rover_main}/rover/simulations/mucFreiNetdLTE2dMulticast/results"

"""
Process a specific simulation scenario

:param name:            Name of the scenario
:param id:              Unique short ID of this scenario
:param measure_subpath: Location of measurement data
:param sims:            List of simulations (tuples of (sim_subpath: str, sim_description: str))
:param plot_args:       A dictionary containing the kwargs to be used for specific plot-types,
                        plot-types: "delay"      - delay vs time
                                    "delay_hist" - delay histogram
                                    "delay_cdf"  - CDF of delays
                                    "iat_hist"   - histogram of inter-arrival times
"""

def process_scenario(name: str, id: str, measure_subpath: str, sims: SimList,
                     plot_args: Dict[str, Dict[str, str]] = {}):
    # initialize
    std_figsize = [6.4, 2.4]  # standard size of figure
    scave = ScaveTool()
    # Read simulation data of all simulations within specified list of simulations
    alertdelay_ue = {}  # dictionary for storing the delays for alerts (application layer)
    df_sims: Dict[str, pd.DataFrame] = {}  # dictionary for storing the dataframes of simulations
    for sim in sims:
        input_sca = f"{SIM_RESULT_BASE}/{sim.path}/*.sca"
        input_vec = f"{SIM_RESULT_BASE}/{sim.path}/*.vec"
        output_csv = f"{SIM_RESULT_BASE}/results_{sim.id}.csv"
        print(f"Converting {input_sca} and {input_vec} to {output_csv}")
        csv = scave.create_or_get_csv_file(
            csv_path=output_csv,
            input_paths=[
                # input_sca,
                input_vec,
            ],
            scave_filter='module("*.app[*]") AND '
                         'name("*Delay:vector")'
            ,
            override=True,
            recursive=True,
        )
        df_sims[sim.id] = scave.load_csv(csv)
        df_sims[sim.id].opp.info()

        alertdelay_ue[sim.id] = df_sims[sim.id].opp.filter().vector().name_regex("alertDelay.*").apply().iloc[0]
        # alertdelay_ue[sim.id] = df.iloc[0]

        # consider only values after the startup-time
        # valid_values = alertdelay_ue[sim.id].vectime > STARTUP_TIME
        # alertdelay_ue[sim.id].vectime = alertdelay_ue[sim.id].vectime[valid_values]
        # alertdelay_ue[sim.id].vecvalue = alertdelay_ue[sim.id].vecvalue[valid_values]

    # %% Read measurement data
    # input_trace = f"{cnf.rover_main}/rover/simulations/openair/measurements/{measure_subpath}"
    # print(f"Reading measurement data: {input_trace}")
    # camdelay_ue_measure = OaiMeasurement.read_packet_trace(input_trace)
    #
    # # consider only values after the startup-time
    # valid_values = camdelay_ue_measure.vectime > STARTUP_TIME
    # camdelay_ue_measure.vectime = camdelay_ue_measure.vectime[valid_values]
    # camdelay_ue_measure.vecvalue = camdelay_ue_measure.vecvalue[valid_values]
    #
    # # use subplots:
    # # fig_adelay, axes = plt.subplots(nrows=2)
    # # ax_time = axes[0]
    # # ax_hist = axes[1]
    #
    # # use individual figures: dictionaries for figures and their axes
    figures = {
    }

    axes = {
    }

    # %% statistics
    print(StatsTool.stats_table(camdelay_ue_measure.vecvalue, "s", f"{id} (measurement): CAM Delay  (time_difference)"))
    for sim in sims:
        print(StatsTool.stats_table(alertdelay_ue[sim.id].vecvalue, "s",
                                    f"{sim.id} (simulation): CAM Delay  (alertDelay)"))

    # %% figure: delay over time
    figures.update({"delay": plt.figure(figsize=std_figsize)})
    plt.figure(figures["delay"].number)
    axes.update({"delay": plt.axes()})
    att.reset()
    # Process measurement data  (ugly: we "misuse" the dataframe of the first simulation)
    df_sims[sims[0].id].opp.plot.create_time_series(axes["delay"], camdelay_ue_measure, label="measurement",
                                                    color=att.get_color(),
                                                    marker=att.get_marker(), auto_labels=False)

    # Process corresponding simulation data
    for sim in sims:
        df_sims[sim.id].opp.plot.create_time_series(axes["delay"], alertdelay_ue[sim.id], label=sim.desc,
                                                    color=att.get_color(),
                                                    marker=att.get_marker())
    if SET_TITLE:
        axes["delay"].set_title(f"{id}: Packet Delay")
    else:
        axes["delay"].set_title("")
    axes["delay"].set_ylabel("delay [s]")
    format_plot("delay", axes, plot_args)

    plt.show()

    # %% figure: delay histogram
    figures.update({"delay_hist": plt.figure(figsize=std_figsize)})
    plt.figure(figures["delay_hist"].number)
    axes.update({"delay_hist": plt.axes()})
    att.reset()
    #  measurements
    (n, bins, patches) = df_sims[sims[0].id].opp.plot.create_histogram(axes["delay_hist"], camdelay_ue_measure,
                                                                       label="measurement",
                                                                       color=att.get_color(), density=False,
                                                                       histtype='step', fill=False,
                                                                       bins=DEFAULT_NR_BINS, auto_labels=False,
                                                                       range=plot_args["delay_hist"].get("range")
                                                                       )

    # simulation
    for sim in sims:
        df_sims[sim.id].opp.plot.create_histogram(axes["delay_hist"], alertdelay_ue[sim.id], label=sim.desc,
                                                  color=att.get_color(),
                                                  density=False,
                                                  histtype='step', fill=False, bins=bins,
                                                  range=plot_args["delay_hist"].get("range")
                                                  )

    # format plot
    if SET_TITLE:
        axes["delay_hist"].set_title(f"{id}: Packet Delay Histogram")
    else:
        axes["delay_hist"].set_title("")
    axes["delay_hist"].set_xlabel("delay [s]");
    axes["delay_hist"].set_ylabel("number of values");
    format_plot("delay_hist", axes, plot_args)

    plt.show()

    # %% figure: delay histogram cumulative
    figures.update({"delay_cdf": plt.figure(figsize=std_figsize)})
    plt.figure(figures["delay_cdf"].number)
    axes.update({"delay_cdf": plt.axes()})
    att.reset()

    # measurement
    (n, bins, patches) = df_sims[sims[0].id].opp.plot.create_histogram(axes["delay_cdf"], camdelay_ue_measure,
                                                                       label="measurement",
                                                                       color=att.get_color(), density=True,
                                                                       cumulative=True,
                                                                       histtype='step',
                                                                       fill=False, bins=DEFAULT_NR_BINS,
                                                                       auto_labels=False,
                                                                       range=plot_args["delay_hist"].get("range")
                                                                       )

    # simulation
    for sim in sims:
        df_sims[sim.id].opp.plot.create_histogram(axes["delay_cdf"], alertdelay_ue[sim.id], label=sim.desc,
                                                  color=att.get_color(),
                                                  density=True,
                                                  histtype='step', cumulative=True, fill=False, bins=bins,
                                                  range=plot_args["delay_hist"].get("range")
                                                  )

    # format plot
    if SET_TITLE:
        axes["delay_cdf"].set_title(f"{id}: Packet Delay CDF")
    else:
        axes["delay_cdf"].set_title("")

    axes["delay_cdf"].set_xlabel("delay [s]")
    axes["delay_cdf"].set_ylabel("P(X<x)")
    format_plot("delay_cdf", axes, plot_args)

    plt.show()

    # %% figure: packet inter-arrival time histogram
    figures.update({"iat_hist": plt.figure(figsize=std_figsize)})
    plt.figure(figures["iat_hist"].number)
    axes.update({"iat_hist": plt.axes()})
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

    (n, bins, patches) = df_sims[sims[0].id].opp.plot.create_histogram(axes["iat_hist"], iat, label="measurement",
                                                                       color=att.get_color(), density=False,
                                                                       histtype='step', fill=False,
                                                                       bins=DEFAULT_NR_BINS, auto_labels=False,
                                                                       range=plot_args["iat_hist"].get("range")
                                                                       )

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

        df_sims[sim.id].opp.plot.create_histogram(axes["iat_hist"], iat, label=sim.desc, color=att.get_color(),
                                                  density=False,
                                                  histtype='step', fill=False, bins=bins, auto_labels=False,
                                                  range=plot_args["iat_hist"].get("range")
                                                  )

    # format plot
    if SET_TITLE:
        axes["iat_hist"].set_title(f"{id}: Packet Inter-Arrival Time Histogram")
    else:
        axes["iat_hist"].set_title("")
    axes["iat_hist"].set_xlabel("inter-arrival time [ms]");
    axes["iat_hist"].set_ylabel("number of values");
    format_plot("iat_hist", axes, plot_args)

    plt.show()
    # %% create figures in PDF and PNG formats
    try:
        os.makedirs(OUTPUT_FOLDER)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

    for name, fig in figures.items():
        filename_base = OUTPUT_FOLDER + os.path.sep + id + "_" + name
        print(f"Saving figure \"{filename_base}.pdf\"")
        fig.savefig(f"{filename_base}.pdf", bbox_inches='tight')
        print(f"Saving figure \"{filename_base}.png\"")
        fig.savefig(f"{filename_base}.png")


# %% Format plot
def format_plot(name: str, axes, plot_args):
    if "loc" in plot_args[name]:
        location = plot_args[name].get("loc")
        axes[name].legend(loc=location)
    else:
        axes[name].legend(loc='lower right')
    if "ylim" in plot_args[name]:
        ylim = plot_args[name].get("ylim")
        axes[name].set_ylim(ylim)
    if "xlim" in plot_args[name]:
        xlim = plot_args[name].get("xlim")
        axes[name].set_xlim(xlim)


# %% Process all scenarios  (set if statement to True/False in order to turn individual plots on/off)

# Scenario 1: UDP 300 B CAM Packets, no other traffic
simList = [Simulation("S1.UA", "S1-UA", "simulation (default model)"),
           Simulation("S1", "S1", "simulation (adapted model)")]

plot_args = {
    'delay': {'ylim': [0.0, 0.015], 'loc': 'upper right'},
    'delay_hist': {'range': [0.0, 0.015], 'xlim': [0.0, 0.015], 'loc': 'upper right'},
    'delay_cdf' : {'range': [0.0, 0.015], 'xlim': [0.0, 0.015], 'loc': 'lower right'},
    'iat_hist'  : {'loc': 'upper right'}
}

if False:
    process_scenario("Scenario 1", "S1", "S1/2020-01-31_16-53-24_cam_receive_log.csv", simList, plot_args);

# Scenario 1.25: UDP 300 B CAM Packets, no other traffic, 25 RBs only
simList = [Simulation("S1.25.UA", "S1-25-UA", "simulation (default model)"),
           Simulation("S1.25", "S1-25", "simulation (adapted model)")]

if False:
    process_scenario("Scenario 1.25 (25 RBs)", "S1.25",
                     "S1/25RBs/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", simList,
                     plot_args);

# Scenario 2: UDP 300 B CAM Packets, overload due to 512B background traffic
# process_scenario("Scenario 2", "S2", "OpenAirInterface-CAM-DL-2-1UE", "S2/2020-01-31_16-59-44_cam_receive_log.csv", range=[2.0,3.0])
simList = [Simulation("S2.UA", "S2-UA", "simulation (default model)"),
           Simulation("S2", "S2", "simulation (adapted model)")]

plot_args = {
    'delay': {'ylim': [0.0, 1.4], 'loc': 'center right'},
    'delay_hist': {'range': [0.0, 1.4], 'xlim': [0.0, 1.4], 'loc': 'upper center'},
    'delay_cdf' : {'range': [0.0, 1.4], 'xlim': [0.0, 1.4], 'loc': 'lower center'},
    'iat_hist'  : {'range': [60, 140], 'xlim': [60, 140], 'loc': 'upper right'}
}

if False:
    process_scenario("Scenario 2", "S2",
                     "S2/2020-01-31_16-59-44_cam_receive_log.csv", simList, plot_args);

# Scenario 2.25: UDP 300 B CAM Packets, overload due to 512B background traffic, 25 RBs only
# process_scenario("Scenario 2 (25 RBs)", "S2.25", "OpenAirInterface-CAM-DL-2-1UE", "S2/25RBs/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv", range=[2.0,3.0])
simList = [Simulation("S2.25.UA", "S2-25-UA", "simulation (default model)"),
           Simulation("S2.25", "S2-25", "simulation (adapted model)")]

plot_args = {
    'delay': {'ylim': [0.0, 2.8], 'loc': 'center right'},
    'delay_hist': {'range': [0.0, 2.8], 'xlim': [0.0, 2.8], 'loc': 'upper center'},
    'delay_cdf' : {'range': [0.0, 2.8], 'xlim': [0.0, 2.8], 'loc': 'lower center'},
    'iat_hist'  : {'range': [60, 140], 'xlim': [60, 140], 'loc': 'upper right'}
}

if False:
    process_scenario("Scenario 2.25 (25 RBs)", "S2.25",
                     "S2/25RBs/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv", simList,
                     plot_args)

# Scenario 3: UE-2-UE Traffic (UE1 sends via uplink to eNodeB, eNodeB sends to UE0), low-rate UDP traffic
# measurement 5: 2020-01-31_15-39-18_cam_receive_log.csv
# measurement 6: 2020-01-31_15-44-49_cam_receive_log.csv
simList = [Simulation("S3.UA", "S3-UA", "simulation (default model)"),
           Simulation("S3", "S3", "simulation (adapted model)")]

plot_args = {
    'delay': {'ylim': [0.0, 0.07], 'loc': 'upper right'},
    'delay_hist': {'range': [0.0, 0.07], 'xlim': [0.0, 0.07], 'loc': 'upper right'},
    'delay_cdf' : {'range': [0.0, 0.07], 'xlim': [0.0, 0.07], 'loc': 'lower right'},
    'iat_hist'  : {'range': [60, 140], 'xlim': [60, 140], 'loc': 'upper right'}
}

if Fals:
    process_scenario("Scenario 3", "S3",
                     "S3/Measurement5/2020-01-31_15-39-18_cam_receive_log.csv", simList, plot_args)

# Scenario 4: UE-2-UE Traffic (UE1 sends via uplink to eNodeB, eNodeB sends to UE0), overload due to background traffic
# 2020-01-31_16-13-32_cam_receive_log.csv
simList = [Simulation("S4.UA", "S4-UA", "simulation (default model)"),
           Simulation("S4", "S4", "simulation (adapted model)")]

plot_args = {
    'delay': {'ylim': [0.0, 1.4], 'loc': 'center right'},
    'delay_hist': {'range': [0.0, 1.4], 'xlim': [0.0, 1.4], 'loc': 'upper center'},
    'delay_cdf' : {'range': [0.0, 1.4], 'xlim': [0.0, 1.4], 'loc': 'lower center'},
    'iat_hist'  : {'range': [60, 140], 'xlim': [60, 140], 'loc': 'upper right'}
}

if False:
    process_scenario("Scenario 4", "S4",
                     "S4/2020-01-31_16-13-32_cam_receive_log.csv", simList, plot_args)

# possible further datagram types
# - CAM data rate (bit/s)  (info already available in inter-arrival plots)
# - background traffic data rate (bit/s)
