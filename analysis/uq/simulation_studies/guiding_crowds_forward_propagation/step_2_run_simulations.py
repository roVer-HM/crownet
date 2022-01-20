#!/usr/bin/env python3
# !/usr/bin/python3

import sys
import os

from suqc.CommandBuilder.VadereControlCommand import VadereControlCommand
from suqc.utils.SeedManager.VadereSeedManager import VadereSeedManager
from suqc.request import CoupledDictVariation
# This is just to make sure that the systems path is set up correctly, to have correct imports, it can be ignored:

sys.path.append(os.path.abspath(""))

run_local = True
###############################################################################################################
# Usecase: Set yourself the parameters you want to change. Do this by defining a list of dictionaries with the
# corresponding parameter. Again, the Vadere output is deleted after all scenarios run.


if __name__ == "__main__":

    if os.environ["CROWNET_HOME"] is None:
        raise SystemError(
            "Please add ROVER_MAIN to your system variables to run a rover simulation."
        )

    path2ini = os.path.join(os.environ["CROWNET_HOME"], "crownet/simulations/route_choice_app/omnetpp.ini")

    reaction_probability_key = 'reactionProbabilities.[stimulusId==-400].reactionProbability'
    par_var = [{'vadere': {reaction_probability_key: 1.0}},
               {'vadere': {reaction_probability_key: 0.5}}]

    # sampling
    par_var = VadereSeedManager(par_variations=par_var, rep_count=2, vadere_fixed=False).get_new_seed_variation()
    qoi = ["densities.txt", "fundamentalDiagramm.txt"]

    for controller in ["ClosedLoop", "OpenLoop", "NoController"]:

        print(f"Simulation runs for controller-type = {controller} started.")

        output_folder = os.path.join(os.getcwd(), controller)

        model = VadereControlCommand() \
            .create_vadere_container() \
            .experiment_label("output") \
            .with_control("control.py") \
            .scenario_file("vadere/scenarios/simplified_default_sequential.scenario") \
            .control_argument("controller-type", controller) \
            .vadere_tag("211214-1621") \
            .control_tag("211210-1432")

        setup = CoupledDictVariation(
            ini_path=path2ini,
            config="final",
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

        print(f"Simulation runs for controller-type = {controller} completed.")
