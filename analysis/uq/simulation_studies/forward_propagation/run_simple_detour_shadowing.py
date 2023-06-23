#!/usr/bin/env python3
#!/usr/bin/python3

import os, sys
sys.path.append(os.path.abspath(""))


import time
from datetime import timedelta
sys.path.append(os.path.abspath(""))
import shutil
import string, random

from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.request import CoupledDictVariation
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import docker

run_local = True


def run_simulations(par_var, summary_dir, quantity_of_interest, simulation_dir, parralel_runs=1):

    start_time = time.time()
    print(f"Simulation started at {time.ctime()}")

    path2ini = os.path.join(
        os.environ["CROWNET_HOME"],
        "crownet/simulations/simple_detoure_suqc_traffic/omnetpp.ini",
    )

    network_id = f"simple_detour_shadow_{''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))}"

    client = docker.from_env(timeout=120, max_pool_size=10)
    try:
        client.networks.get(network_id=network_id).remove()
    except:
        pass
    client.networks.create(network_id)

    model = VadereOppCommand() \
        .create_vadere_container() \
        .experiment_label("output") \
        .vadere_tag("030b71de") \
        .omnet_tag("6.0.1") \
        .qoi(quantity_of_interest) \
        .write_container_log() \
        .network_name(network_id)

    if os.path.isdir(simulation_dir):
        print(f"Remove {simulation_dir}")
        shutil.rmtree(simulation_dir, ignore_errors=True)

    setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=quantity_of_interest,
        model=model,
        post_changes=None,
        output_path=os.path.dirname(simulation_dir),
        output_folder=os.path.basename(simulation_dir),
        remove_output=False,
    )

    par, data = setup.run(parralel_runs)
    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

    os.makedirs(summary_dir)

    par.to_csv(os.path.join(summary_dir, f"parameters.txt"))

    for qoi_, vals_ in data.items():
        file_path = os.path.join(summary_dir, qoi_)
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    client.networks.create(network_id)

    return par, data


if __name__ == "__main__":

    DIGIT = 3

    if len(sys.argv) == 1:
        # default behavior of script
        file_path = os.path.dirname(os.path.abspath(__file__))
        simulation_dir = os.path.join(file_path, "shadowing")
        high = 120
        number = 30
        para_process = 6
    elif len(sys.argv) == 5:
        # use arguments from command line
        simulation_dir = sys.argv[1]
        high = float(sys.argv[2])
        number = int(sys.argv[3])
        para_process = int(sys.argv[4])
    else:
        raise ValueError("Call the script >python3 script_name.py abspathoutputdir 120 20 1< "
                         "where abspathoutputdir is the path to the output directory"
                         "where 120 is the max of the number of peds parameter."
                         "where 20 is the number of samples"
                         "where 1 is the number of parallel runs.")


    samples = np.linspace(1, high, number)

    summary_dir = os.path.join(simulation_dir, "results_summary")
    output_dir = os.path.join(simulation_dir, "Shadowing_results")

    par_var = list()
    for val1 in samples:

        # Parameter 1: number of agents
        # The number of agents is a parameter that is specified in the vadere simulator.
        # There are four sources in the simulation that spawn agents according to Poisson processes.
        # The unit of the Poisson parameter p is [agents/1 seconds].
        # Since the 'number of agents' parameter refers to 100s and there are four sources, p must be:
        p = round(val1 * 0.01 / 4, 4) # source parameters must be of type >list< in vadere, other parameters require other types

        sample = {
            'vadere': {'sources.[id==1].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==2].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==5].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==6].spawner.distribution.numberPedsPerSecond': p}
            }

        par_var.append(sample)

    par_var = OmnetSeedManager(par_variations=par_var, rep_count=1, vadere_fixed=False, omnet_fixed=True).get_new_seed_variation()

    quantity_of_interest = [
        "degree_informed_extract.txt",
        "poisson_parameter.txt",
        "time_95_informed_redirection_area.txt",
        "time_95_informed_all.txt",
        "packet_age.txt",
        "number_of_peds.txt"
    ]

    info, qoi = run_simulations(par_var,summary_dir, quantity_of_interest, simulation_dir, para_process)
    is_post_processing_only = True # skip simulations? comment line 115 and set this true

    if is_post_processing_only:
        print("try to read from old simulation")
        time_95 = pd.read_csv(os.path.join(summary_dir, "time_95_informed_all.txt"))
        number_peds = pd.read_csv(os.path.join(summary_dir, "number_of_peds.txt"))
        info = pd.read_csv(os.path.join(summary_dir, "parameters.txt"))
    else:
        time_95 = qoi["time_95_informed_all.txt"]
        number_peds = qoi["number_of_peds.txt"]

    aa = number_peds[time_95["timeToInform95PercentAgents"] == np.inf]
    bb = number_peds[time_95["timeToInform95PercentAgents"] != np.inf]

    bins = np.linspace(0, high, int(np.ceil(np.sqrt(number))))
    width = bins[1] - bins[0]
    number_notrec = np.histogram(aa["mean"], bins=bins)
    number_rec = np.histogram(bb["mean"], bins=bins)
    prob_fail = number_notrec[0] / (number_notrec[0] + number_rec[0])
    prob_fail = np.nan_to_num(prob_fail)

    # post processing of results
    if os.path.isdir(output_dir):
        print(f"Remove {output_dir}")
        shutil.rmtree(output_dir, ignore_errors=True)
    os.makedirs(output_dir)


    plt.bar(bins[:-1], number_rec[0], color='b', width=width, align="edge")
    plt.bar(bins[:-1], number_notrec[0], bottom=number_rec[0], color='r', width=width, align="edge")
    plt.ylabel('Information dissemination below 10s')
    plt.xlabel("Number of pedestrians (mean)")
    plt.legend(labels=['<= 10s', '>10s'])
    plt.savefig(os.path.join(output_dir,"Probabilites.pdf"))
    plt.show(block=False)
    plt.close()

    plt.bar(bins[:-1], prob_fail, color='b', width=width, align="edge")
    plt.ylabel('Probability (>10s)')
    plt.xlabel("Number of pedestrians (mean)")
    plt.savefig(os.path.join(output_dir,"Probabilites_normed.pdf"))
    plt.show(block=False)
    plt.close()

    plt.scatter(x=number_peds["mean"], y=time_95["timeToInform95PercentAgents"])
    plt.xlabel("Number of pedestrians (mean)")
    plt.ylabel("Information dissemination time in s")
    plt.savefig(os.path.join(output_dir,"InfoDisserminationTime.pdf"))
    plt.show(block=False)
    plt.close()

    print(info)

    if (info["('MetaInfo', 'return_code')"] == 0).all():
        sys.exit(0)
    else:
        print("Some of the simulations failed.")
        sys.exit(-1)














