import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from oppanalyzer import Config, ScaveTool, StatsTool, OaiMeasurement

STARTUP_TIME = 60

cnf = Config()


def process_scenario(name: str, id: str, sim_subpath: str, measure_subpath: str, **kwargs):
    # %% initialize
    std_figsize = [6.4, 2.4]
    scave = ScaveTool()
    # %% Read simulation data
    csv = scave.create_or_get_csv_file(
        csv_path=f"{cnf.rover_main}/rover/simulations/openair/results/results_{id}.csv",
        input_paths=[
            f"{cnf.rover_main}/rover/simulations/openair/results/{sim_subpath}/*.sca",
            f"{cnf.rover_main}/rover/simulations/openair/results/{sim_subpath}/*.vec",
        ],
        scave_filter=None,
        override=True,
        recursive=True,
    )
    df = scave.load_csv(csv)

    alertdelay_ue = df.opp.filter().vector().name_regex("alertDelay.*").apply().iloc[0]

    # consider only values after the startup-time
    valid_values = alertdelay_ue.vectime > STARTUP_TIME
    alertdelay_ue.vectime = alertdelay_ue.vectime[valid_values]
    alertdelay_ue.vecvalue = alertdelay_ue.vecvalue[valid_values]

    # %% Read measurement data
    input_trace = f"{cnf.rover_main}/rover/simulations/openair/measurements/{measure_subpath}"

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
        "delay" : plt.figure(figsize=std_figsize),
        "delay_hist": plt.figure(figsize=std_figsize),
        "delay_cdf": plt.figure(figsize=std_figsize),
        "iat_hist" : plt.figure(figsize=std_figsize)
    }

    axes = {
    }

    # create axes for all figures
    for name, fig in figures.items():
        plt.figure(fig.number)
        axes[name] = plt.axes()


    # %% statistics
    print(StatsTool.stats_table(camdelay_ue_measure.vecvalue, "s", f"{id} (measurement): CAM Delay  (time_difference)"))
    print(StatsTool.stats_table(alertdelay_ue.vecvalue, "s", f"{id} (simulation): CAM Delay  (alertDelay)"))

    # %% figure: delay over time

    df.opp.plot.create_time_series(axes["delay"], camdelay_ue_measure, label="measurement", color="r", marker=".")

    # Process corresponding simulation data
    df.opp.plot.create_time_series(axes["delay"], alertdelay_ue, label="simulation", color="b", marker="x")
    axes["delay"].legend()
    axes["delay"].set_title(f"{id}: Packet Delay")
    axes["delay"].set_ylabel("delay [s]");
    axes["delay"].legend(loc='upper right')
    axes["delay"].set_ylim(bottom=0)

    # %% figure: delay histogram
    #  measurements
    hist_add_args = kwargs

    if "hist_range"  in hist_add_args:
        hist_add_args["range"] = hist_add_args.pop("hist_range")

    (n, bins, patches) = df.opp.plot.create_histogram(axes["delay_hist"], camdelay_ue_measure, label="measurement", color="r", density=False, histtype='step', fill=False, **hist_add_args)

    # simulation
    df.opp.plot.create_histogram(axes["delay_hist"], alertdelay_ue, label="simulation", color="b", density=False, histtype='step', fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["delay_hist"].set_title(f"{id}: Packet Delay Histogram")
    axes["delay_hist"].set_xlabel("delay [s]");
    axes["delay_hist"].set_ylabel("number of values");
    axes["delay_hist"].legend()

    # %% figure: delay histogram cumulative
    hist_add_args = kwargs

    if "hist_range"  in hist_add_args:
        hist_add_args["range"] = hist_add_args.pop("hist_range")

    # measurement
    (n, bins, patches) = df.opp.plot.create_histogram(axes["delay_cdf"], camdelay_ue_measure, label="measurement", color="r", density=True, cumulative=True, histtype='step', fill=False, **hist_add_args)

    # simulation
    df.opp.plot.create_histogram(axes["delay_cdf"], alertdelay_ue, label="simulation", color="b", density=True, histtype='step', cumulative=True, fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["delay_cdf"].set_title(f"{id}: Packet Delay CDF")
    axes["delay_cdf"].set_xlabel("delay [s]");
    axes["delay_cdf"].set_ylabel("P(X<x)");
    axes["delay_cdf"].legend()


    # %% figure: packet inter-arrival time histogram

    # calculate inter-arrival times: measurements
    nr_values = len(camdelay_ue_measure['vecvalue'])-1
    iat_time = np.zeros(nr_values)
    iat_value = np.zeros(nr_values)
    for i, time in enumerate(iat_value):
        iat_time[i] = (camdelay_ue_measure['vectime']).iloc[i]
        iat_value[i] = ((camdelay_ue_measure['vectime']).iloc[i+1] - (camdelay_ue_measure['vectime']).iloc[i])*1000

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
                                                      color="r", density=False, histtype='step', fill=False,
                                                      **hist_add_args)

    # simulation

    # calculate inter-arrival times: simulation
    nr_values = len(alertdelay_ue['vecvalue']) - 1
    iat_time = np.zeros(nr_values)
    iat_value = np.zeros(nr_values)
    for i, time in enumerate(iat_value):
        iat_time[i] = (alertdelay_ue['vectime'])[i]
        iat_value[i] = ((alertdelay_ue['vectime'])[i + 1] - (alertdelay_ue['vectime'])[i])*1000

    iat = pd.Series({
        "vectime": iat_time,
        "vecvalue": iat_value
    })

    df.opp.plot.create_histogram(axes["iat_hist"], iat, label="simulation", color="b", density=False,
                                 histtype='step', fill=False, bins=bins, **hist_add_args)

    # format plot
    axes["iat_hist"].set_title(f"{id}: Packet Inter-Arrival Time Histogram")
    axes["iat_hist"].set_xlabel("inter-arrival time [ms]");
    axes["iat_hist"].set_ylabel("number of values");
    axes["iat_hist"].legend()



    # %% create figures in PDF and PNG formats
    plt.show()
    for name, fig in figures.items():
        filename_base = id + "_" + name
        print(f"Saving figure \"{filename_base}.pdf\"")
        fig.savefig(f"{filename_base}.pdf", bbox_inches='tight')
        print(f"Saving figure \"{filename_base}.png\"")
        fig.savefig(f"{filename_base}.png")


# %% Process all scenarios

# Scenario 1: UDP 300 B CAM Packets, no other traffic
process_scenario("Scenario 1", "S1", "OpenAirInterface-CAM-DL-1", "2018-06-28/measurement1/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", range=[0.0,0.015]);

# Scenario 1: UDP 300 B CAM Packets, no other traffic
# process_scenario("Scenario 1B", "S1B", "OpenAirInterface-CAM-DL-1B", "2018-06-28/measurement1/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", range=[0.0,0.015]);

# Scenario 2: UDP 300 B CAM Packets, overload due to 512B background traffic
process_scenario("Scenario 2", "S2", "OpenAirInterface-CAM-DL-2", "2018-06-28/measurement2/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv", range=[2.0,3.0])


# + number of values in general stats

# possible datagram types
# - standard statistics
# - CAM Delay (delay vs. time, delay histogram)
# - CAM data rate (bit/s)
# - CAM inter-packet times (inter-packet time vs. time, inter-packet time histogram)
# - background traffic data rate (bit/s)
#
# single plots instead of two in one?
