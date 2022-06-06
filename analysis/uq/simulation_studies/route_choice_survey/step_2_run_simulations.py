#!/usr/bin/env python3
# !/usr/bin/python3

import sys
import os
import time
from datetime import timedelta

import numpy as np
import pandas as pd

from suqc.CommandBuilder.VadereControlCommand import VadereControlCommand
from suqc.utils.SeedManager.VadereSeedManager import VadereSeedManager
from suqc.request import CoupledDictVariation
# This is just to make sure that the systems path is set up correctly, to have correct imports, it can be ignored:

sys.path.append(os.path.abspath(""))

run_local = True
###############################################################################################################
# Usecase: Set yourself the parameters you want to change. Do this by defining a list of dictionaries with the
# corresponding parameter. Again, the Vadere output is deleted after all scenarios run.

scenario = "three_corridors" #TODO test "three_corridors" or "route_choice_real_world"
#scenario = "route_choice_real_world"

def run_controller(controller, qoi, par_var):

    simulation_dir = "/mnt/data/route_choice_survey"

    if os.environ["CROWNET_HOME"] is None:
        raise SystemError(
            "Please add CROWNET_HOME to your system variables to run a rover simulation."
        )

    path2ini = os.path.join(os.environ["CROWNET_HOME"], "crownet/simulations/route_choice_app/omnetpp.ini")

    print(f"\n\n\nSimulation runs for controller-type = {controller} started.")

    output_folder = os.path.join(simulation_dir, controller)

    model = VadereControlCommand() \
        .create_vadere_container() \
        .experiment_label("output") \
        .with_control("control.py") \
        .scenario_file(f"vadere/scenarios/{scenario}.scenario") \
        .control_argument("controller-type", controller) \
        .vadere_tag("latest") \
        .control_tag("latest")


    setup = CoupledDictVariation(
        ini_path=path2ini,
        config=f"env_{scenario}", #TODO scenario defined in omnetpp.ini must be the same as in VadereControlCommand
        parameter_dict_list=par_var,
        qoi=qoi,
        model=model,
        post_changes=None,
        output_path=os.path.dirname(output_folder),
        output_folder=output_folder,
        remove_output=False,
    )

    par, data = setup.run(12)

    par.to_csv(os.path.join(os.getcwd(), f"{controller}_parameters.csv"))
    for qoi_, vals_ in data.items():
        file_path = os.path.join(os.getcwd(), f"{controller}_{qoi_.replace('.txt', '.csv')}")
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    print(f"Simulation runs for controller-type = {controller} completed.\n\n\n")


if __name__ == "__main__":


    dists = pd.read_csv("route_choice_dists.dat")

    # where to store raw simulation output (*.traj, ..), note: collected quantities of interest are stored in cwd
    start_time = time.time()

    reaction_probability_key = "finishTime"
    probs = np.linspace(0.0, 1.0, 10)

    par_var_ = list()
    conditions = ["A1","A2","A3","A4","B1","B2","B3","B4"]
    for condition in conditions:



        for stat in ["mean", "median", "q1", "q3"]:

            df = dists[dists["condition"] == condition]

            values_short_route = [1.0, 0.0, 0.0]
            values_long_route = df[stat].round(2).values.tolist()
            values_medium_route = [1-values_long_route[-1], values_long_route[-1] , 0.0]


            compliances = {'routeChoices.[instruction=="use target 11"].targetProbabilities': values_short_route,
                       "routeChoices.[instruction=='use target 21'].targetProbabilities": values_medium_route,
                       'routeChoices.[instruction==use target 31].targetProbabilities': values_long_route}

            sample = {'vadere': compliances, 'dummy_var': {"condition": condition, "statistic": stat}}
            par_var_.append(sample)


    reps = 3
    par_var = VadereSeedManager(par_variations=par_var_, rep_count=reps, vadere_fixed=False).get_new_seed_variation()

    qoi1 = "densities.txt"
    qoi2 = "fundamentalDiagramm.txt"
    qoi3 = "startTime.txt"
    qoi4 = "targetReachTime.txt"
    qoi5 = "path_choice.txt" # collect these quantities of interest

    run_controller(controller="OpenLoop", par_var= par_var , qoi= [qoi1, qoi2, qoi3, qoi4, qoi5] )
    #run_controller(controller="NoController", par_var=par_var[:reps], qoi=[qoi1, qoi2, qoi3, qoi4])  # only zero needed

    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")
