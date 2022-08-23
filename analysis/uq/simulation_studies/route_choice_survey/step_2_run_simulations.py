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

    simulation_dir = "/mnt/data1TB/route_choice_survey"

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

    par, data = setup.run(10)

    par.to_csv(os.path.join(os.getcwd(), f"{controller}_parameters.csv"))
    for qoi_, vals_ in data.items():
        file_path = os.path.join(os.getcwd(), f"{controller}_{qoi_.replace('.txt', '.csv')}")
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    print(f"Simulation runs for controller-type = {controller} completed.\n\n\n")


if __name__ == "__main__":


    dists = pd.read_csv("RouteUtilizations.csv")
    sources = dists[dists["condition"] == "Uninformed"]

    sources = sources.drop(columns="condition").set_index("population")
    sources = 1/sources
    sources = sources.div(sources.sum(axis=1), axis=0)*10
    sources = sources.reset_index()


    work_around = dists[dists["condition"] == "Uninformed"].drop(columns="condition").set_index("population").reset_index()



    # where to store raw simulation output (*.traj, ..), note: collected quantities of interest are stored in cwd
    start_time = time.time()

    reaction_probability_key = "finishTime"
    probs = np.linspace(0.0, 1.0, 10)

    par_var_ = list()
    conditions = ["Uninformed", "A1","A2","A3","A4","B1","B2","B3","B4"]

    for condition in conditions:

        for stat in ["Students", "Fans"]:

            sources_ = sources[sources["population"] == stat]
            work_around_ = work_around[work_around["population"] == stat]

            df = dists[dists["condition"] == condition]
            df = df[df["population"] == stat]

            target_11_p = df["routeC"].values[0]
            target_21_p = df["routeB"].values[0]
            target_31_p = df["routeA"].values[0]

            #default_vals = [target_11_p, target_21_p, target_31_p]
            #values_short_route = default_vals
            #values_long_route = default_vals
            #values_medium_route = default_vals

            if condition == "Uninformed":
                compliances = {"usePsychologyLayer": False,
                               'routeChoices.[instruction=="use target 11"].targetProbabilities': [-1,-1,-1],
                               "routeChoices.[instruction=='use target 21'].targetProbabilities": [-1,-1,-1],
                               'routeChoices.[instruction==use target 31].targetProbabilities': [-1,-1,-1]
                               }
            else:
                values_short_route = [-1,-1,-1]
                values_medium_route = [-1,-1,-1]
                values_long_route = [df["routeC"].values[0], df["routeB"].values[0], df["routeA"].values[0]]

                compliances = {"usePsychologyLayer": True,
                        'routeChoices.[instruction=="use target 11"].targetProbabilities': values_short_route,
                       "routeChoices.[instruction=='use target 21'].targetProbabilities": values_medium_route,
                       'routeChoices.[instruction==use target 31].targetProbabilities': values_long_route}

            sources__ = {'sources.[id==11].distributionParameters.updateFrequency': sources_["routeC"].values[0],
                        'sources.[id==21].distributionParameters.updateFrequency': sources_["routeB"].values[0],
                        'sources.[id==31].distributionParameters.updateFrequency': sources_["routeA"].values[0],
            }

            compliances = dict(compliances, **sources__)


            sample = {'vadere': compliances, 'dummy_var': {"condition": condition, "statistic": stat}}
            par_var_.append(sample)


    reps = 5
    par_var = VadereSeedManager(par_variations=par_var_, rep_count=reps, vadere_fixed=False).get_new_seed_variation()

    qoi1 = "densities.txt"
    qoi2 = "fundamentalDiagramm.txt"
    qoi3 = "startTime.txt"
    qoi4 = "targetReachTime.txt"
    qoi5 = "path_choice.txt" # collect these quantities of interest

    run_controller(controller="ClosedLoop", par_var= par_var , qoi= [qoi1, qoi2, qoi3, qoi4, qoi5] )
    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")
