import fnmatch
import os
from typing import List
from matplotlib import pyplot as plt

import roveranalyzer.simulators.crownet.analysis.compare as a

from roveranalyzer.utils.path import PathHelper
from roveranalyzer.simulators.opp.utils import Simulation

# paths
path_sumo_bottleneck = PathHelper.from_env(
    f"CROWNET_HOME",
    f"crownet/simulations/cmp_vadere_sumo/results/sumoBottleneck/",
)
path_sumo_simple = PathHelper.from_env(
    f"CROWNET_HOME",
    f"crownet/simulations/cmp_vadere_sumo/results/sumoSimple/",
)
path_vadere_bottleneck = PathHelper.from_env(
    f"CROWNET_HOME",
    f"crownet/simulations/cmp_vadere_sumo/results/vadereBottleneck/",
)
path_vadere_simple = PathHelper.from_env(
    f"CROWNET_HOME",
    f"crownet/simulations/cmp_vadere_sumo/results/vadereSimple/",
)


# utility
def find(pattern, path) -> List[str]:
    result = []
    for root, dirs, files in os.walk(path.abs_path()):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(os.path.join(root, name))
    return result


def simulations_from_folder(path: PathHelper, description: str) -> List[Simulation]:
    res = []
    vec_files = find("*.vec", path)
    for i, vec_file in enumerate(vec_files):
        res.append(Simulation(i, os.path.dirname(vec_file), description))
    return res


def main():
    sims_sumo_bottleneck = simulations_from_folder(path_sumo_bottleneck, "sumoBottleneck")
    sims_sumo_simple = simulations_from_folder(path_sumo_simple, "sumoSimple")
    sims_vadere_bottleneck = simulations_from_folder(path_vadere_bottleneck, "vadereBottleneck")
    sims_vadere_simple = simulations_from_folder(path_vadere_simple, "vadereSimple")

    # distance plots
    fig1 = a.distance_plot_mean(sims_sumo_bottleneck, title="Sumo Bottleneck", cutoff=0.8)
    fig2 = a.distance_plot_mean(sims_vadere_bottleneck, title="Vadere Bottleneck", cutoff=0.8)
    # fig3 = a.distance_plot_mean(sims_sumo_simple, title="Sumo Simple", cutoff=0.7)
    # fig4 = a.distance_plot_mean(sims_vadere_simple, title="Vadere Simple", cutoff=0.7)

    # plot_pnode_positions
    sim = sims_vadere_simple[0]
    figs_vadere = a.plot_pnode_positions(sim, 0, 60, 20)
    sim = sims_sumo_simple[0]
    figs_sumo = a.plot_pnode_positions(sim, 0, 60, 20)

    # compare simulations vectors
    choice = True
    if choice:
        module = "*World.pNode[*].app[0].app"
        vector_name = "rcvdPkLifetime:vector"
        unit = "[s]"
        how = "mean"
    else:
        module = "*World.pNode[*].cellularNic.mac"
        vector_name = "sentPacketToLowerLayer:vector(packetBytes)"
        unit = "[byte/s]"
        how = "sum"

    # aggregate sumo sims
    dfs_sumo = []
    for sim in sims_sumo_bottleneck:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_sumo.append(df)
        print("debug")
    # average sumo sims
    df_sumo = a.average_sim_data(dfs_sumo)
    # aggregate vadere sims
    dfs_vadere = []
    for sim in sims_vadere_bottleneck:
        df = a.aggregate_vectors_from_simulation(sim, module, vector_name, how)
        dfs_vadere.append(df)
    # average vadere sims
    df_vadere = a.average_sim_data(dfs_vadere)
    # plot comparison
    fig5 = a.plot_comparison([df_vadere, df_sumo], ["vadere-bottleneck", "sumo-bottleneck"],
                             vector_name=vector_name, unit=unit, rolling_only=False)
    fig6 = a.plot_comparison([df_sumo], ["sumoBottleneck"], vector_name, unit, rolling_only=False)

    plt.show()


if __name__ == "__main__":
    main()
