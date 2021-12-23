#!/usr/bin/env python3
#!/usr/bin/python3

import os

from uq.PreProcessing.sampling.utils import read_json
from suqc import (
    CoupledConsoleWrapper,
    CoupledDictVariation,
)

if os.getenv("CROWNET_HOME") is None:
    repo = "crownet"
    crownet_root = os.path.join(os.getcwd().split(repo)[0], repo)
    os.environ["CROWNET_HOME"] = crownet_root
    print(f"Add CROWNET_HOME = {crownet_root} to env variables.")

#TODO use docstrings

# This step is computationally extensive.
# While step 1 can normally be carried out on a standard computer,
# it may be necessary to switch to a mainframe computer for this step.

if __name__ == "__main__":

    # Step 2
    # The parameter combinations (samples) have been created in step 1.
    # In step 2, we pass the parameter values to the simulation and start one simulation run for each sample.


    # Read sample values
    samplingMC = read_json("pMCHalton.json") # If not done yet, run step1_generate_parameter_combinations.py
    sample_values_raw = samplingMC.get_samples()

    # Assign the sample values to the corresponding simulator variables
    # If necessary, the values can be converted and formatted here.

    par_var = list()
    for val1, val2 in sample_values_raw:

        # Parameter 1: number of agents
        # The number of agents is a parameter that is specified in the vadere simulator.
        # There are four sources in the simulation that spawn agents according to Poisson processes.
        # The unit of the Poisson parameter p is [agents/1 seconds].
        # Since the 'number of agents' parameter refers to 100s and there are four sources, p must be:
        p = val1 * 0.01 / 4 # source parameters must be of type >list< in vadere, other parameters require other types

        # Parameter 2: message length
        # The message length is a parameter in the omnet simulator.
        # It must be an integer value with unit B.
        message_length_parameter_val = f"{int(val2)}B" # note: parameter values must be always strings in omnet
        sample = {
            'omnet': {"*.misc[0].app[0].messageLength": message_length_parameter_val}, # *.pNode[*].app[0].messageLength = 3kB, *.pNode[*].app[1].messageLength = 1000B
            'vadere': {'sources.[id==1].distributionParameters.numberPedsPerSecond': p,
                    'sources.[id==2].distributionParameters.numberPedsPerSecond': p,
                    'sources.[id==5].distributionParameters.numberPedsPerSecond': p,
                    'sources.[id==6].distributionParameters.numberPedsPerSecond': p}
            }

        # Additional configuration settings
        # While the parameter values differ for each sample, the following settings are the same for all samples
        sample['omnet']['sim-time-limit'] = '180s'
        sample['omnet']['*.misc[0].app[0].incidentTime'] = '100s'
        sample['omnet']['*.radioMedium.obstacleLoss.typename'] = 'DielectricObstacleLoss'
        par_var.append(sample)

    # COMMENT NEXT LINE to run all simulations
    par_var = par_var[:2]


    ## Define the simulation to be used
    # A rover simulation is defined by an "omnetpp.ini" file and its corresponding directory.
    # Use following *.ini file:

    path2ini = os.path.join(
        os.environ["CROWNET_HOME"],
        "crownet/simulations/simple_detoure_suqc_traffic/omnetpp.ini",
    )

    ## Define the quantities of interest (simulation output variables)
    # Make sure that corresponding post processing methods exist in the run_script2.py file

    #TODO: MW -> update
    model = CoupledConsoleWrapper(
        model="Coupled", vadere_tag="latest", omnetpp_tag="latest"
    )

    quantity_of_interest = [
        "degree_informed_extract.txt",
        "time_95_informed.txt",
        "packet_age.txt"
    ]

    setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=quantity_of_interest,
        model=model,
        scenario_runs=1,
        post_changes=None,
        output_path=os.getcwd(),
        output_folder="output",
        remove_output=False,
        seed_config={"vadere": "random", "omnet": "random"},
        env_remote=None,
    )

    # run simulations
    par_var, data = setup.run(10) # run 2 simulations in parallel

    par_var.to_csv("parameters.csv")
    output_nr = 0
    for q in quantity_of_interest:
        output_nr +=1
        # UNCOMMENT NEXT LINE to use for step 3
        #data[q].to_csv(f"result_df_{output_nr}.csv")