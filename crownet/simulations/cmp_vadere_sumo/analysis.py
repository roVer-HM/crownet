import os
from typing import List
from matplotlib import pyplot as plt
from os.path import expanduser

import roveranalyzer.simulators.crownet.analysis.compare as a
from roveranalyzer.simulators.crownet.analysis.compare import How

from roveranalyzer.utils.path import PathHelper
from roveranalyzer.simulators.opp.utils import Simulation

# paths

path_results = PathHelper.from_env(
    f"CROWNET_HOME",
    # f"crownet/simulations/cmp_vadere_sumo/study/simple_study/simulation_runs/outputs/",
    f"crownet/simulations/cmp_vadere_sumo/results/",
)


def find(file_extension: str, path: str, must_contain: str = None) -> List[str]:
    """ Finds all file of a certain type within a folder and its sub-folders.

    :param file_extension: The file extension
    :param path: The root folder to be searched
    :param must_contain: Optional folder name which must be part of the path.
    :return: A list of paths to all files with the given extension in the folder and all sub-folders
    """
    result = []
    for root, dirs, files in os.walk(path.abs_path()):
        for name in files:
            if name.endswith(file_extension) and (not must_contain or (must_contain in root)):
                result.append(os.path.join(root, name))
    return result


def simulations_from_folder(path: PathHelper, configuration: str) -> List[Simulation]:
    """ Finds all simulation data under the given path and returns Simulation objects for each

    :param path: The path which will be searched
    :param configuration: Name of OMNeT++ configuration, will be given to all Simulations objects
    :return: A list of Simulations objects which were found under the path and its sub-folders
    """
    res = []
    vec_files = find(".vec", path, configuration)
    for i, vec_file in enumerate(vec_files):
        res.append(Simulation(i, os.path.dirname(vec_file), configuration))
    return res


def main():
    # sims_sumo_simple = simulations_from_folder(path_results, "sumoSimple")
    # sims_vadere_simple = simulations_from_folder(path_results, "vadereSimple")
    sims_sumo_simple = simulations_from_folder(path_results, "sumoOnly2")
    sims_vadere_simple = simulations_from_folder(path_results, "vadereOnly2")
    sims_sumo_bottleneck = simulations_from_folder(path_results, "sumoBottleneck")
    sims_vadere_bottleneck = simulations_from_folder(path_results, "vadereBottleneck")

    # distance plots
    if len(sims_sumo_simple) > 0:
        fig1: plt.Figure
        fig1, ax = a.distance_plot_mean(sims_sumo_simple, title="Sumo Simple", cutoff=0.7)
        fig1.savefig(expanduser('~/fig1.png'))
    if len(sims_vadere_simple) > 0:
        fig2, ax2 = a.distance_plot_mean(sims_vadere_simple, title="Vadere Simple", cutoff=0.7)
        fig2.savefig(expanduser('~/fig2.png'))
    if len(sims_sumo_bottleneck) > 0:
        fig3, ax3 = a.distance_plot_mean(sims_sumo_bottleneck, title="Sumo Bottleneck", cutoff=0.8)
        fig3.savefig(expanduser('~/fig3.png'))
    if len(sims_vadere_bottleneck) > 0:
        fig4, ax4 = a.distance_plot_mean(sims_vadere_bottleneck, title="Vadere Bottleneck", cutoff=0.8)
        fig4.savefig(expanduser('~/fig4.png'))

    if len(sims_sumo_simple) > 0 and len(sims_vadere_simple) > 0:
        # plot_pnode_positions
        sim = sims_vadere_simple[0]
        fig_pos_vadere, ax = a.plot_pnode_positions(sim, 0, 60, 20)
        sim = sims_sumo_simple[0]
        fig_pos_sumo, ax = a.plot_pnode_positions(sim, 0, 60, 20)

    # compare simulations vectors
    choice = True
    if choice:
        module = "*World.pNode[*].app[0].app"
        vector_name = "rcvdPkLifetime:vector"
        unit = "[s]"
        how = How.mean
    else:
        module = "*World.pNode[*].cellularNic.mac"
        vector_name = "sentPacketToLowerLayer:vector(packetBytes)"
        unit = "[byte/s]"
        how = How.sum



    # TODO: redundant code - refactoring required!
    # aggregate sumo sims
    dfs_sumo = []
    for sim in sims_sumo_simple:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_sumo.append(df)
        print("debug")
    # average sumo sims
    if len(dfs_sumo) > 0:
        df_sumo = a.average_sim_data(dfs_sumo)
        fig6, ax6 = a.plot_comparison([df_sumo], ["sumoSimple"], vector_name=vector_name,
                                      vector_description="received Pkt lifetime", unit=unit, rolling_only=False)
    # aggregate vadere sims
    dfs_vadere = []
    for sim in sims_vadere_simple:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_vadere.append(df)
    # average vadere sims
    if len(dfs_vadere) > 0:
        df_vadere = a.average_sim_data(dfs_vadere)
    # plot comparison
    if len(dfs_sumo) > 0 and len(dfs_vadere) > 0:
        fig5, ax5 = a.plot_comparison([df_vadere, df_sumo], ["vadere-simple", "sumo-simple"],
                                      vector_name=vector_name, vector_description="received Pkt lifetime", unit=unit,
                                      rolling_only=False)

    # TODO: redundant code - refactoring required!
    # aggregate sumo sims
    dfs_sumo = []
    for sim in sims_sumo_bottleneck:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_sumo.append(df)
        print("debug")
    # average sumo sims
    if len(dfs_sumo) > 0:
        df_sumo = a.average_sim_data(dfs_sumo)
        fig7, ax7 = a.plot_comparison([df_sumo], ["sumoBottleneck"], vector_name=vector_name,
                                      vector_description="received Pkt lifetime", unit=unit, rolling_only=False)
    # aggregate vadere sims
    dfs_vadere = []
    for sim in sims_vadere_bottleneck:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_vadere.append(df)
    # average vadere sims
    if len(dfs_vadere) > 0:
        df_vadere = a.average_sim_data(dfs_vadere)
    # plot comparison
    if len(dfs_sumo) > 0 and len(dfs_vadere) > 0:
        fig8, ax8 = a.plot_comparison([df_vadere, df_sumo], ["vadere-bottleneck", "sumo-bottleneck"],
                                      vector_name=vector_name, vector_description="received Pkt lifetime", unit=unit,
                                      rolling_only=False)


    plt.show()


if __name__ == "__main__":
    main()
