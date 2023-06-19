#!/usr/bin/env python3
# !/usr/bin/python3

import sys
import os
import time
from datetime import timedelta

import numpy as np
import shutil

from suqc.CommandBuilder.VadereControlCommand import VadereControlCommand
from suqc.utils.SeedManager.VadereSeedManager import VadereSeedManager
from suqc.request import CoupledDictVariation

sys.path.append(os.path.abspath(""))
run_local = True

mnt = os.environ["OPP_EXTERN_DATA_MNT"].split(":")[0]
assert os.path.isdir(mnt), "Please specify OPP_EXTERN_DATA_MNT. See CROWNET_HOME/config"
simulation_dir = os.path.join(mnt, "guiding_crowds_forward_propagation")

def run_controller(controller, qoi, par_var):


    if os.environ["CROWNET_HOME"] is None:
        raise SystemError(
            "Please add ROVER_MAIN to your system variables to run a rover simulation."
        )

    path2ini = os.path.join(os.environ["CROWNET_HOME"], "crownet/simulations/route_choice_app/omnetpp.ini")

    print(f"\n\n\nSimulation runs for controller-type = {controller} started.")

    output_folder = os.path.join(simulation_dir, controller)


    model = VadereControlCommand() \
        .create_vadere_container() \
        .experiment_label("output") \
        .with_control("control.py") \
        .control_argument("controller-type", controller) \
        .vadere_tag("496ff02c") \
        .control_tag("496ff02c")

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

    par, data = setup.run(5)
    par.to_csv(os.path.join(simulation_dir, f"{controller}_parameters.csv"))

    for qoi_, vals_ in data.items():
        file_path = os.path.join(simulation_dir, f"{controller}_{qoi_.replace('.txt', '.csv')}")
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    print(f"Simulation runs for controller-type = {controller} completed.\n\n\n")


if __name__ == "__main__":

    # where to store raw simulation output (*.traj, ..), note: collected quantities of interest are stored in cwd
    if os.path.isdir(simulation_dir):
        print(f"Remove {simulation_dir}")
        shutil.rmtree(simulation_dir,ignore_errors=True)
    os.makedirs(simulation_dir)

    start_time = time.time()

    probs = np.linspace(0, 1.0, 41)
    DIGIT=4
    par_var_ = [{'vadere': {'routeChoices.[instruction=="use target 11"].targetProbabilities': [1.0, 0.0, 0.0],
                   'routeChoices.[instruction=="use target 21"].targetProbabilities': [round(1-p,DIGIT), round(p,DIGIT), 0.0],
                   'routeChoices.[instruction=="use target 31"].targetProbabilities': [round(1-p,DIGIT), 0.0, round(p,DIGIT)]}} for p in probs]

    reps = 1
    par_var = VadereSeedManager(par_variations=par_var_, rep_count=reps, vadere_fixed=False).get_new_seed_variation()

    qoi1 = "densities.txt"
    qoi2 = "fundamentalDiagramm.txt"
    qoi3 = "startTime.txt"
    qoi4 = "targetReachTime.txt"
    qoi5 = "path_choice.txt" # collect these quantities of interest

    run_controller(controller="NoController", par_var= par_var[:reps] , qoi= [qoi1, qoi2, qoi3, qoi4] )
    run_controller(controller="ClosedLoop", par_var= par_var , qoi= [qoi1, qoi2, qoi3, qoi4, qoi5] )
    run_controller(controller="OpenLoop", par_var=par_var, qoi=[qoi1, qoi2, qoi3, qoi4, qoi5])
    run_controller(controller="AvoidShort", par_var=par_var, qoi=[qoi1, qoi2, qoi3, qoi4, qoi5])

    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")
    sys.exit(0)
