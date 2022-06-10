# Script for doing the simulation study.
#
# Launch this script via python3 or within your Python IDE to perform a simulation study.

import os
import pprint

import pandas as pd
import psutil
import random
import shutil
import string
import time
from enum import Enum
from typing import List, Dict, Any

import numpy as np
from pandas.core.indexes import base
from scipy import rand

from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator, coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager

from roveranalyzer.utils.path import PathHelper
from roveranalyzer.simulators.opp.scave import ScaveTool
from roveranalyzer.simulators.opp.utils import Simulation
from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun
from roveranalyzer.simulators.crownet.analysis.compare import aggregate_vectors_from_simulation, average_sim_data
from roveranalyzer.simulators.crownet.analysis.compare import How

# number of repetitions to be performed for each parameter set
REPS = 2  # for testing we use 2, for reliable results it should be >10 (depending on stddev)

# estimated amount of memory per simulation
MEM_PER_SIM_VADERE_GB = 11
MEM_PER_SIM_SUMO_GB = 1
MEM_PER_SIM_OMNET_GB = 1


# --- generic utility functions - should be moved to roveranalyzer lateron
# TODO: move to roveranalyzer and remove duplicate code here and in analysis.py
# def find(file_extension: str, path: str, must_contain: str = None) -> List[str]:
#     """ Finds all file of a certain type within a folder and its sub-folders.
#
#     :param file_extension: The file extension
#     :param path: The root folder to be searched
#     :param must_contain: Optional folder name which must be part of the path.
#     :return: A list of paths to all files with the given extension in the folder and all sub-folders
#     """
#     result = []
#     for root, dirs, files in os.walk(path):
#         for name in files:
#             if name.endswith(file_extension) and (not must_contain or (must_contain in root)):
#                 result.append(os.path.join(root, name))
#     return result


def get_parameter_to_sims(run: SuqcRun, module_name: str, parameter_name: str):
    simulations = run.get_simulations()
    parameter_to_sims = {}
    for sim in simulations:
        scave_filter = ScaveTool().filter_builder().t_parameter().AND().module(module_name).AND().gOpen().name(
            parameter_name).gClose().build()
        df_param = ScaveTool().read_parameters(sim.sql.sca_path, scave_filter=scave_filter)
        param_value = df_param.at[0, "value"]
        if param_value not in parameter_to_sims:
            parameter_to_sims[param_value] = []
        parameter_to_sims[param_value].append(sim)
    return parameter_to_sims


# --- end of utility functions


class MobilitySimulatorType(Enum):
    OMNET = 1
    VADERE = 2
    SUMO = 3


def main(base_path):
    time = UnitValue.s(600.0)

    # parameter variation: define parameters to be varied
    configs = ["sumoOnly2", "vadereOnly2", "sumoSimple", "vadereSimple", "sumoBottleneck", "vadereBottleneck"]

    par_var = [
        {
            "omnet": {
                "sim-time-limit": time,
                "*.*Node[*].app[0].scheduler.generationInterval": "0.1s"
            },
        },
        {
            "omnet": {
                "sim-time-limit": time,
                "*.*Node[*].app[0].scheduler.generationInterval": "0.01s"
            },
        },
    ]

    # run simulation studies
    for config in configs:
        if "sumo" in config:
            mobility_type = MobilitySimulatorType.SUMO
        elif "vadere" in config:
            mobility_type = MobilitySimulatorType.VADERE
        else:
            raise ValueError("Cannot determine mobility type based on config name.")
        run_parameter_study(base_path, mobility_type, config, par_var)

    # perform analysis
    qoi_results = pd.DataFrame()
    for config in configs:
        qoi = analyze_parameter_study(base_path, config)
        qoi_results = pd.concat([qoi_results, qoi])
    print(qoi_results)
    qoi_results.to_csv(os.path.join(base_path, 'qoi_results.csv'), sep=" ")


def get_results_dir(base_path: string):
    study_name = os.path.basename(__file__).replace(".py", "")
    return os.path.abspath(base_path + "../results/" + study_name + "/")


def run_parameter_study(base_path: str, mobility_sim: MobilitySimulatorType, config: str,
                        par_var: List[Dict[str, Any]]):
    par_var = OmnetSeedManager(
        par_variations=par_var,
        rep_count=REPS,
        omnet_fixed=False,
        vadere_fixed=None).get_new_seed_variation()

    # Enviroment setup.
    # 
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = get_results_dir(base_path)
    if os.path.exists(os.path.join(base_dir, config)):
        print(f'Warning: {base_dir} already exists - skipping configuration {config}')
        return

    os.makedirs(base_dir, exist_ok=True)

    if mobility_sim == MobilitySimulatorType.SUMO:
        mobility_container = ("sumo", "latest")
        model = SumoCommand() \
            .sumo_tag("latest")
    elif mobility_sim == MobilitySimulatorType.VADERE:
        mobility_container = ("vadere", "latest")
        model = VadereOppCommand() \
            .vadere_tag("latest")
    else:
        mobility_container = None
        model = OmnetCommand()

    model.write_container_log()
    model.omnet_tag("latest")
    model.experiment_label("out")
    model.timeout = None
    model.qoi(["all"])
    model.verbose()

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=config,
        opp_config=config,
        opp_basename="omnetpp.ini",
        mobility_sim=mobility_container,
        communication_sim=("omnet", "latest"),
        handle_existing="force_replace"
        # handle_existing="ask_user_replace"
    )

    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[]
    )

    _rnd = ''.join(random.choices(string.ascii_lowercase + string.digits, k=6))
    parameter_variation = ParameterVariationBase().add_data_points(par_var)

    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out=config + "_runscript.out"
    )

    print("setup done")
    nr_parallel = max_parallel_sims(mobility_sim)
    nr_retries = 20
    while nr_parallel < 1 and nr_retries > 0:
        print(f'Cannot start simulations - not enough free memory! Retrying in 3s.')
        time.sleep(3)
        nr_parallel = max_parallel_sims(mobility_sim)
        nr_retries = nr_retries - 1
    print(f'estimated maximum number of parallel simulations: {nr_parallel}')
    setup.run(nr_parallel)


def max_parallel_sims(mobility_sim: MobilitySimulatorType) -> int:
    estimated_mem_per_sim = MEM_PER_SIM_OMNET_GB
    if mobility_sim == MobilitySimulatorType.SUMO:
        estimated_mem_per_sim = estimated_mem_per_sim + MEM_PER_SIM_SUMO_GB
    elif mobility_sim == MobilitySimulatorType.VADERE:
        estimated_mem_per_sim = estimated_mem_per_sim + MEM_PER_SIM_VADERE_GB
    mem = psutil.virtual_memory()
    # consider free and cached memory for simulations (since cached memory will be freed upon request)
    mem_available_gb = ((mem.free + mem.cached) / (1024 * 1024 * 1024))
    max_sims = int(mem_available_gb / estimated_mem_per_sim)
    return max_sims


def analyze_parameter_study(base_path: str, config: str) -> pd.DataFrame:
    # define what is to be analyzed
    module = "World.pNode[%].app[0].app"
    var_parameter = {"module": "World.pNode[0].app[0].scheduler",
                 "name": "generationInterval"}

    vec_names = {
        "rcvdPkLifetime:vector": {
            "name": "rcvdPktLifetime",
            "dtype": float,
        },
        "rcvdPkHostId:vector": {"name": "srcHostId", "dtype": np.int32},
    }

    # do analysis
    suq_run = SuqcRun(os.path.join(get_results_dir(base_path), config))
    par_to_sims = get_parameter_to_sims(suq_run, var_parameter["module"], var_parameter["name"])
    mean_values = []
    stddev_values = []
    index_values = []
    column_headers = ['config', 'mean', 'stddev']
    for param_value in par_to_sims.keys():
        print(f'Analyzing all simulations for parameter value: {param_value}')
        dfs_aggregated = []
        for sim in par_to_sims[param_value]:
            print(f'   Analyzing simulation: {sim.data_root}')
            vec_data = sim.sql.vec_data_pivot(
                module,
                vec_names,
                append_index=["srcHostId"],
            )
            vec_data = vec_data.sort_index()
            dfs_aggregated.append(vec_data)
        dfs_avg = average_sim_data(dfs_aggregated)
        dfs_avg.to_csv(os.path.join(os.path.join(get_results_dir(base_path), config),
                                    f'{var_parameter["name"]}_{param_value}_rcvdPktLifetime_avg.csv'), sep=" ")
        mean_values.append(dfs_avg["value"].mean())
        stddev_values.append(dfs_avg["value"].std())
        index_values.append(param_value)

    results_array = np.array([[config] * len(mean_values), mean_values, stddev_values])
    results_array = np.transpose(results_array)
    results_df = pd.DataFrame(data=results_array, index=index_values, columns=column_headers)

    return results_df


if __name__ == "__main__":
    main("./")
