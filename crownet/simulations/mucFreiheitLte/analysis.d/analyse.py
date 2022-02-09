import errno
import os
from typing import List, Dict

import matplotlib.pyplot as plt
import pandas as pd
from crownetanalyzer.oppanalyzer.utils import Config, ScaveTool, StatsTool, Simulation, PlotAttrs

# Type definitions
SimList = List[Simulation]

# Default parameter values
STARTUP_TIME = 0
DEFAULT_NR_BINS = 100
OUTPUT_FOLDER = "plots"
SET_TITLE = False

cnf = Config()
att = PlotAttrs()
SIM_RESULT_BASE = f"{cnf.crownet_main}/crownet/simulations/mucFreiNetdLTE2dMulticast/results"

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

def process_scenario(name: str, id: str, sims: SimList,
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
        output_csv = f"{SIM_RESULT_BASE}/{sim.path}/results_{sim.id}.csv"
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

    figures = {
    }

    axes = {
    }

    # %% statistics
    for sim in sims:
        print(StatsTool.stats_table(alertdelay_ue[sim.id].vecvalue, "s",
                                    f"{sim.id} (simulation): CAM Delay  (alertDelay)"))

    # %% figure: delay over time
    figures.update({"delay": plt.figure(figsize=std_figsize)})
    plt.figure(figures["delay"].number)
    axes.update({"delay": plt.axes()})
    att.reset()

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

    bins = DEFAULT_NR_BINS
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
    axes["delay_hist"].set_xlabel("delay [s]")
    axes["delay_hist"].set_ylabel("number of values")
    format_plot("delay_hist", axes, plot_args)

    plt.show()

    # %% figure: delay histogram cumulative
    figures.update({"delay_cdf": plt.figure(figsize=std_figsize)})
    plt.figure(figures["delay_cdf"].number)
    axes.update({"delay_cdf": plt.axes()})
    att.reset()

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

    # simulation
    for sim in sims:
        # calculate inter-arrival times: simulation
        iter_arr = pd.Series(alertdelay_ue[sim.id]['vectime'].copy())
        iter_arr = iter_arr.diff(1)[1:] * 1000

        df_sims[sim.id].opp.plot.create_histogram(axes["iat_hist"], iter_arr, label=sim.desc, color=att.get_color(),
                                                  density=False,
                                                  histtype='step', fill=False, bins=bins, auto_labels=False,
                                                  range=plot_args["iat_hist"].get("range")
                                                  )

    # format plot
    if SET_TITLE:
        axes["iat_hist"].set_title(f"{id}: Packet Inter-Arrival Time Histogram")
    else:
        axes["iat_hist"].set_title("")
    axes["iat_hist"].set_xlabel("inter-arrival time [ms]")
    axes["iat_hist"].set_ylabel("number of values")
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


if __name__ == "__main__":
    # %% Process all scenarios  (set if statement to True/False in order to turn individual plots on/off)

    # Scenario 1: XXX
    simList = [Simulation("D2D.1", "vadere00_20200424-09:18:06", "simulation (D2D with LOS corrected)"),
               Simulation("D2D.2", "vadere00_20200424-11:15:06", "simulation (D2D with static LOS true)")]

    plot_args = {
        'delay': {'loc': 'upper right'},
        'delay_hist': {'loc': 'upper right'},
        'delay_cdf': {'loc': 'lower right'},
        'iat_hist': {'loc': 'upper right'}
    }

    if True:
        process_scenario(name="Direct", id="D2D1", sims=simList, plot_args=plot_args)
