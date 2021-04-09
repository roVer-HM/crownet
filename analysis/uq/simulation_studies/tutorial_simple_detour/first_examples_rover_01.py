#!/usr/bin/env python3
#!/usr/bin/python3

import os, shutil
from suqc import (
    Parameter,
    DependentParameter,
    RoverSamplingFullFactorial,
    CoupledConsoleWrapper,
    CoupledDictVariation,
)


run_local = True

if os.getenv("CROWNET_HOME") is None:
    crownet_root = os.path.abspath(f"{__file__}/../../../../../..")
    if os.path.basename(crownet_root) == "crownet":
        os.environ["CROWNET_HOME"] = crownet_root
        print(f"Add CROWNET_HOME = {crownet_root} to env variables.")
    else:
        raise ValueError(
            f"Please provide environmental variable CROWNET_HOME = path/to/crownet/repo."
        )


def main(parameter="default", quantity_of_interest="default"):

    output_folder = os.path.join(os.path.dirname(os.path.abspath(__file__)), "results")

    ## Define the simulation to be used
    # A rover simulation is defined by an "omnetpp.ini" file and its corresponding directory.
    # Use following *.ini file:

    path2ini = os.path.join(
        os.environ["CROWNET_HOME"],
        "crownet/simulations/simple_detoure_suqc_traffic/omnetpp.ini",
    )

    ## Define parameters and sampling method
    # parameters
    if parameter == "default":
        parameter = [
            Parameter(
                name="number_of_agents_mean", simulator="dummy", stages=[15, 20],
            )  # number of agents that are generated in 100s
        ]

    dependent_parameters = [
        DependentParameter(
            name="sources.[id==1].distributionParameters",
            simulator="vadere",
            equation=lambda args: [(args["number_of_agents_mean"] * 0.01 / 4)],
        ),
        DependentParameter(
            name="sources.[id==2].distributionParameters",
            simulator="vadere",
            equation=lambda args: [(args["number_of_agents_mean"] * 0.01 / 4)],
        ),
        DependentParameter(
            name="sources.[id==5].distributionParameters",
            simulator="vadere",
            equation=lambda args: [(args["number_of_agents_mean"] * 0.01 / 4)],
        ),
        DependentParameter(
            name="sources.[id==6].distributionParameters",
            simulator="vadere",
            equation=lambda args: [(args["number_of_agents_mean"] * 0.01 / 4)],
        ),
        DependentParameter(name="sim-time-limit", simulator="omnet", equation="180s"),
        DependentParameter(
            name="*.station[0].app[0].incidentTime", simulator="omnet", equation="100s",
        ),
        DependentParameter(
            name="*.radioMedium.obstacleLoss.typename",
            simulator="omnet",
            equation="DielectricObstacleLoss",
        ),
    ]

    # number of repitions for each sample
    reps = 1

    # sampling
    par_var = RoverSamplingFullFactorial(
        parameters=parameter, parameters_dependent=dependent_parameters
    ).get_sampling()

    ## Define the quantities of interest (simulation output variables)
    # Make sure that corresponding post processing methods exist in the run_script2.py file

    # define tag of omnet and vadere docker images, see https://sam-dev.cs.hm.edu/rover/rover-main/container_registry/
    model = CoupledConsoleWrapper(
        model="Coupled", vadere_tag="latest", omnetpp_tag="latest"
    )

    if quantity_of_interest == "default":
        quantity_of_interest = [
            "degree_informed_extract.txt",
            "poisson_parameter.txt",
            "time_95_informed.txt",
        ]

    setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=quantity_of_interest,
        model=model,
        scenario_runs=reps,
        post_changes=None,
        output_path=os.path.dirname(output_folder),
        output_folder=output_folder,
        remove_output=False,
        seed_config={"vadere": "random", "omnet": "random"},
        env_remote=None,
    )

    if os.environ["CROWNET_HOME"] is None:
        raise SystemError(
            "Please add ROVER_MAIN to your system variables to run a rover simulation."
        )

    if run_local:
        par_var, data = setup.run(1)
    else:
        par_var, data = setup.remote(-1)

    # Save results
    summary = output_folder + "_summary"
    if os.path.exists(summary):
        shutil.rmtree(summary)

    os.makedirs(summary)

    par_var.to_csv(os.path.join(summary, "parameters.csv"))
    for q in quantity_of_interest:
        data[q].to_csv(os.path.join(summary, f"{q}"))

    print("All simulation runs completed.")


if __name__ == "__main__":

    main()
