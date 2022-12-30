import datetime

from matplotlib import pyplot as plt
from os.path import expanduser

import roveranalyzer.simulators.crownet.analysis.compare as a
from roveranalyzer.simulators.crownet.analysis.compare import How

from roveranalyzer.utils.path import PathHelper

# paths

path_results = PathHelper.from_env(
    f"CROWNET_HOME",
    # f"crownet/simulations/cmp_vadere_sumo/study/simple_study/simulation_runs/outputs/",
    f"crownet/simulations/combined_vadere_sumo/results/",
)


def main():
    parameters = ["eNodeBTxPower=20"]
    # timeframe = [datetime.datetime(2022, 6, 12, 0, 0, 0), datetime.datetime(2022, 6, 13, 0, 0, 0)]
    timeframe = [datetime.datetime(2000, 1, 1, 0, 0, 0), datetime.datetime.now()]

    sims_sumo_mf = a.simulations_from_folder(
        path_results, "mfSumo_Study", timeframe=timeframe
    )
    sims_combined_mf = a.simulations_from_folder(
        path_results, "mfCombined_Study", timeframe=timeframe
    )

    if len(sims_sumo_mf) > 0 and len(sims_combined_mf) > 0:
        # plot_pnode_positions
        plt.rc('font', size=7)
        sim = sims_combined_mf[0]
        fig_pos_vadere, ax = a.plot_pnode_positions(sim, 20, 180, 40, vLabel="Vehicle (Sumo)", pLabel="Person (Vadere)")
        
        plt.rc('font', size=7)
        sim = sims_sumo_mf[0]
        fig_pos_sumo, ax = a.plot_pnode_positions(sim, 20, 180, 40, vLabel="Vehicle (Sumo)", pLabel="Person (Sumo)")
        
    fig_pos_vadere.set_figheight(3.4)
    fig_pos_sumo.set_figheight(3.4)
    
    fig_pos_vadere.savefig("/home/rupp/pos_plot_vadere.png")
    fig_pos_vadere.savefig("/home/rupp/pos_plot_vadere.svg")
    fig_pos_vadere.savefig("/home/rupp/pos_plot_vadere.pdf")
    fig_pos_sumo.savefig("/home/rupp/pos_plot_sumo.png")
    fig_pos_sumo.savefig("/home/rupp/pos_plot_sumo.svg")
    fig_pos_sumo.savefig("/home/rupp/pos_plot_sumo.pdf")


if __name__ == "__main__":
    main()
