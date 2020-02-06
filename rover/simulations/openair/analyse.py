import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from oppanalyzer import Config, ScaveTool, StatsTool, OaiMeasurement

STARTUP_TIME = 60

cnf = Config()
scave = ScaveTool()


def process_scenario(name: str, id: str, sim_subpath: str, measure_subpath: str, **kwargs):
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

    fig_adelay, axes = plt.subplots(nrows=2)
    ax_time = axes[0]
    ax_hist = axes[1]



    # %% Process corresponding measurements
    print(StatsTool.stats_table(camdelay_ue_measure.vecvalue, "s", f"{id} (measurement): CAM Delay  (time_difference)"))

    # fig_adelay_measure, axes = plt.subplots(nrows=2)
    # ax_time = axes[0]
    # ax_time.set_label("Measurement: CAM Delay")
    # ax_time.set_xlabel("time [s]");

    df.opp.plot.create_time_series(ax_time, camdelay_ue_measure, label="measurement", color="r", marker=".")

    hist_add_args = kwargs

    if "hist_range"  in hist_add_args:
        hist_add_args["range"] = hist_add_args.pop("hist_range")

    df.opp.plot.create_histogram(ax_hist, camdelay_ue_measure, label="measurement", color="r", density=False, histtype='step', fill=False, **hist_add_args)

    # %% Simulation: Create time series and histogram of receive SINR at the eNB

    print(StatsTool.stats_table(alertdelay_ue.vecvalue, "s", f"{id} (simulation): CAM Delay  (alertDelay)"))

    df.opp.plot.create_time_series(ax_time, alertdelay_ue, label="simulation", color="b", marker="x")
    ax_time.legend()

    df.opp.plot.create_histogram(ax_hist, alertdelay_ue, label="simulation", color="b", density=False, histtype='step', fill=False, **hist_add_args)

    # %% format plot and save it to PDF and PNG files
    ax_time.set_title(f"{id}: Packet Delay")
    ax_time.set_ylabel("delay [s]");
    ax_time.legend(loc='upper right')
    ax_time.set_ylim(bottom=0)

    ax_hist.set_title(f"{id}: Packet Delay Histogram")
    ax_hist.set_xlabel("delay [s]");
    ax_hist.set_ylabel("number of values");
    ax_hist.legend()

    # fig_adelay_measure.show()
    fig_adelay.show()
    plt.show()

    # create figure in PDF and PNG formats
    fig_adelay.savefig(f"delay_{id}.pdf", bbox_inches='tight')
    fig_adelay.savefig(f"delay_{id}.png")


# %% Process all scenarios

# Scenario 1: UDP 300 B CAM Packets, no other traffic
# process_scenario("Scenario 1", "S1", "OpenAirInterface-CAM-DL-1", "2018-06-28/measurement1/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", range=[0.0,0.015]);

# Scenario 1: UDP 300 B CAM Packets, no other traffic
process_scenario("Scenario 1B", "S1B", "OpenAirInterface-CAM-DL-1B", "2018-06-28/measurement1/2018-06-28_13-08-53_cam_receive_log__no_general_traffic.csv", range=[0.0,0.015]);

# Scenario 2: UDP 300 B CAM Packets, overload due to 512B background traffic
# process_scenario("Scenario 2", "S2", "OpenAirInterface-CAM-DL-2", "2018-06-28/measurement2/2018-06-28_14-42-39_cam_receive_log__send_general_traffic_freq=0.00017.csv")





# possible datagram types
# - standard statistics
# - CAM Delay (delay vs. time, delay histogram)
# - CAM data rate (bit/s)
# - CAM inter-packet times (inter-packet time vs. time, inter-packet time histogram)
# - background traffic data rate (bit/s)
#
# single plots instead of two in one?
