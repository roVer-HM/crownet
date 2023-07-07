#!/usr/bin/env python3
# !/usr/bin/python3

import os
import shutil
import sys
import time
from datetime import timedelta
import pandas as pd
import docker
import string, random

import numpy as np

from suqc.CommandBuilder.VadereControlCommand import VadereControlCommand
from suqc.request import CoupledDictVariation
from suqc.utils.SeedManager.VadereSeedManager import VadereSeedManager

sys.path.append(os.path.abspath(""))
run_local = True

from analyze_sim_results import post_processing


def run_controller(controller, qoi, par_var, simulation_dir, number_processes=1):
    if os.environ["CROWNET_HOME"] is None:
        raise SystemError(
            "Please add ROVER_MAIN to your system variables to run a rover simulation."
        )

    path2ini = os.path.join(os.environ["CROWNET_HOME"], "crownet/simulations/route_choice_app/omnetpp.ini")

    print(f"\n\n\nSimulation runs for controller-type = {controller} started.")

    output_folder = os.path.join(simulation_dir, controller)

    network_id = f"{controller}_{''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))}"

    client = docker.from_env(timeout=120, max_pool_size=10)
    try:
        client.networks.get(network_id=network_id).remove()
    except:
        pass
    client.networks.create(network_id)

    model = VadereControlCommand() \
        .create_vadere_container() \
        .experiment_label("output") \
        .with_control("control.py") \
        .control_argument("controller-type", controller) \
        .vadere_tag("030b71de") \
        .control_tag("496ff02c") \
        .write_container_log() \
        .network_name(network_id)

    setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=qoi,
        model=model,
        post_changes=None,
        output_path=os.path.dirname(output_folder),
        output_folder=controller,
        remove_output=False,
    )

    par, data = setup.run(number_processes)
    par.to_csv(os.path.join(simulation_dir, f"{controller}_parameters.csv"))

    for qoi_, vals_ in data.items():
        file_path = os.path.join(simulation_dir, f"{controller}_{qoi_.replace('.txt', '.csv')}")
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    client.networks.get(network_id=network_id).remove()
    print(f"Simulation runs for controller-type = {controller} completed.\n\n\n")


if __name__ == "__main__":


    folder_name = "guiding_crowds_forward_propagation_sims"  # make sure this is the same in analyze_sim_results.py

    if len(sys.argv) == 1:
        # default behavior of script
        file_path = os.path.dirname(os.path.abspath(__file__))
        simulation_dir = os.path.join(file_path, folder_name)
        number_compliance_rates = 41
        spawnNumber = 8
        number_processes = 10
    elif len(sys.argv) == 5:
        # use arguments from command line
        simulation_dir = sys.argv[1]
        number_compliance_rates = int(sys.argv[2])
        spawnNumber = int(sys.argv[3])
        number_processes = int(sys.argv[4])
    else:
        raise ValueError("Call the script >python3 script_name.py abspathoutputdir 41 8 2< "
                         "where abspathoutputdir is the path to the dir that contains all simulations."
                         "where 41 is the number of compliance rates"
                         "number of agents spawned every 2s"
                         "where 2 is the number of parallel processes")

    print(f"Write to {simulation_dir}. number of compliance rates: {number_compliance_rates}. "
          f"spwanNumber: {spawnNumber}."
          f"number of processes: {number_processes}")


    if os.path.isdir(simulation_dir):
        print(f"Remove {simulation_dir}")
        shutil.rmtree(simulation_dir, ignore_errors=True)
    print(f"Store all simulations in {simulation_dir}")
    os.makedirs(simulation_dir)

    # where to store raw simulation output (*.traj, ..), note: collected quantities of interest are stored in cwd
    if os.path.isdir(simulation_dir):
        print(f"Remove {simulation_dir}")
        shutil.rmtree(simulation_dir, ignore_errors=True)
    os.makedirs(simulation_dir)

    start_time = time.time()

    probs = np.linspace(0, 1.0, number_compliance_rates)
    DIGIT = 4

    par_var_ = list()
    for p in probs:
        short = [1.0, 0.0, 0.0] # assume all people follow the short route when recommended
        medium = [round(1 - p, DIGIT), round(p, DIGIT), 0.0] # assume that compying people take the medium route, rest: short
        long = [round(1 - p, DIGIT), 0.0, round(p, DIGIT)] # assume that compying people take the long route, rest: short

        sample = {'vadere':
                      {'routeChoices.[instruction=="use target 11"].targetProbabilities': short,
                       'routeChoices.[instruction=="use target 21"].targetProbabilities': medium,
                       'routeChoices.[instruction=="use target 31"].targetProbabilities': long,
                       'sources.[id==39].spawnNumber': spawnNumber},
                  'dummy': {"Compliance": round(p, DIGIT)}}
        par_var_.append(sample)

    reps = 1
    par_var = VadereSeedManager(par_variations=par_var_, rep_count=reps, vadere_fixed=False).get_new_seed_variation()

    qoi = ["densities.txt", "fundamentalDiagramm.txt", "startTime.txt", "targetReachTime.txt", "path_choice.txt"]

    run_controller(controller="NoController", par_var=par_var[:reps], qoi=qoi, number_processes=number_processes, simulation_dir=simulation_dir)
    run_controller(controller="AvoidShort", par_var=par_var, qoi=qoi, number_processes=number_processes, simulation_dir=simulation_dir)
    run_controller(controller="ClosedLoop", par_var=par_var, qoi=qoi, number_processes=number_processes,simulation_dir=simulation_dir)
    run_controller(controller="OpenLoop", par_var=par_var, qoi=qoi, number_processes=number_processes,simulation_dir=simulation_dir)

    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

    failed = list()
    for controller in ["NoController", "AvoidShort", "ClosedLoop", "OpenLoop"]:
        info = pd.read_csv(os.path.join(simulation_dir, f"{controller}_parameters.csv"))
        if (info["('MetaInfo', 'return_code')"] == 0).all() == False:
            failed.append(os.path.join(simulation_dir, controller))
            print(info[info["('MetaInfo', 'return_code')"]])

    post_processing(simulation_dir)

    if len(failed) == 0:
        sys.exit(0)
    else:
        print(f"Some of the simulations failed. Please check: {failed}.")
        sys.exit(-1)





