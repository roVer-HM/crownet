import datetime

from matplotlib import pyplot as plt
from os.path import expanduser

import crownetutils.analysis.mobility_model_compare as a
from crownetutils.analysis.mobility_model_compare import How

from crownetutils.utils.path import PathHelper

# paths

path_results = PathHelper.from_env(
    f"CROWNET_HOME",
    # f"crownet/simulations/cmp_vadere_sumo/study/simple_study/simulation_runs/outputs/",
    f"crownet/simulations/cmp_vadere_sumo/results/",
)


def main():
    # compare simulations vectors
    choice = False
    if choice:
        module = "*World.pNode[*].app[0].app"
        vector_name = "rcvdPkLifetime:vector"
        unit = "[s]"
        how = a.How.mean
    else:
        module = "*World.pNode[*].cellularNic.mac"
        vector_name = "sentPacketToUpperLayer:vector(packetBytes)"
        unit = "[byte/s]"
        how = a.How.sum

    parameters = ["eNodeBTxPower=20"]
    # timeframe = [datetime.datetime(2022, 6, 12, 0, 0, 0), datetime.datetime(2022, 6, 13, 0, 0, 0)]
    timeframe = [datetime.datetime(2000, 1, 1, 0, 0, 0), datetime.datetime.now()]

    sims_sumo_simple = a.simulations_from_folder(
        path_results, "sumoSimple", timeframe=timeframe
    )
    sims_vadere_simple = a.simulations_from_folder(
        path_results, "vadereSimple", timeframe=timeframe
    )
    # sims_sumo_simple = a._simulations_from_folder(path_results, "sumoOnly2", timeframe=timeframe)
    # sims_vadere_simple = a._simulations_from_folder(path_results, "vadereOnly2", timeframe=timeframe)
    sims_sumo_bottleneck = a.simulations_from_folder(
        path_results, "sumoBottleneck", timeframe=timeframe
    )
    sims_vadere_bottleneck = a.simulations_from_folder(
        path_results, "vadereBottleneck", timeframe=timeframe
    )

    if len(sims_sumo_simple) > 1:
        fig1, ax = a.compare_parameter_study(
            sims_sumo_simple,
            parameter_name="eNodeBTxPower",
            parameter_unit="[dBm]",
            module=module,
            vector=vector_name,
            vector_description=vector_name,
            unit=unit,
            how=how,
        )
        fig1.savefig(expanduser("~/sumoSimpleParamterStudy.png"))

    if len(sims_sumo_simple) > 1:
        fig1, ax = a.distance_plot_mean(
            sims_sumo_simple, title="Sumo Simple", cutoff=0.7
        )
        fig1.savefig(expanduser("~/sumoSimpleDistance.png"))
    if len(sims_vadere_simple) > 1:
        fig2, ax2 = a.distance_plot_mean(
            sims_vadere_simple, title="Vadere Simple", cutoff=0.7
        )
        fig2.savefig(expanduser("~/vadereSimpleDistance.png"))
    if len(sims_sumo_bottleneck) > 1:
        fig3, ax3 = a.distance_plot_mean(
            sims_sumo_bottleneck, title="Sumo Bottleneck", cutoff=0.8
        )
        fig3.savefig(expanduser("~/sumoBottleneckDistance.png"))
    if len(sims_vadere_bottleneck) > 1:
        fig4, ax4 = a.distance_plot_mean(
            sims_vadere_bottleneck, title="Vadere Bottleneck", cutoff=0.8
        )
        fig4.savefig(expanduser("~/vadereBottleneckDistance.png"))

    if len(sims_sumo_simple) > 0 and len(sims_vadere_simple) > 0:
        # plot_pnode_positions
        sim = sims_vadere_simple[0]
        fig_pos_vadere, ax = a.plot_pnode_positions(sim, 0, 60, 20)
        sim = sims_sumo_simple[0]
        fig_pos_sumo, ax = a.plot_pnode_positions(sim, 0, 60, 20)

    if len(sims_sumo_simple) > 0:
        fig6, ax6 = a.plot_comparison(
            path_results,
            ["sumoSimple"],
            module=module,
            vector_name=vector_name,
            vector_description="received Pkt lifetime",
            unit=unit,
            how=how,
            rolling_only=True,
        )

    # plot comparison
    if len(sims_sumo_simple) > 0 and len(sims_vadere_simple) > 0:
        fig7, ax7 = a.plot_comparison(
            path_results,
            ["sumoSimple", "vadereSimple"],
            module=module,
            vector_name=vector_name,
            vector_description="received Pkt lifetime",
            unit=unit,
            how=how,
            rolling_only=True,
        )

    plt.show()


if __name__ == "__main__":
    main()
