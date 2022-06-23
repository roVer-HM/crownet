# Script for doing the simulation study.
#
# Launch this script via python3 or within your Python IDE to perform a simulation study.

import os
import pprint

import pandas as pd
import numpy as np
from pandas.core.indexes import base
from scipy import rand
import matplotlib.figure
import matplotlib.axes
from matplotlib import pyplot as plt
import psutil
import random
import shutil
import string
import time
from enum import Enum
from typing import List, Dict, Any, Tuple

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

# number of repetitions to be performed for each parameter set
REPS = 3  # for testing we use 2, for reliable results it should be >10 (depending on stddev)

# estimated amount of memory per simulation
MEM_PER_SIM_VADERE_GB = 15
MEM_PER_SIM_SUMO_GB = 2
MEM_PER_SIM_OMNET_GB = 1

# file where analysis results are stored
QOI_RESULTS_FILE = 'qoi_results.csv'
QOI_RESULTS_TIME_FILE = 'qoi_results_t.csv'


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
    simtime = UnitValue.s(600.0)

    # parameter variation: define parameters to be varied
    configs = ["sumoOnly2", "vadereOnly2", "sumoSimple", "vadereSimple", "sumoBottleneck", "vadereBottleneck"]

    # configs = ["sumoBottleneck", "vadereBottleneck"]
    # configs = ["sumoOnly2", "vadereOnly2"]

    def var(interval):
        return {"omnet": {
            "sim-time-limit": simtime,
            "*.*Node[*].app[0].scheduler.generationInterval": f"{interval}s"
        }}

    # note: for modifying several parameters and create par_var for all combinations, use
    # itertools.product
    par_var = [var(round(inter, 3)) for inter in np.arange(0.01, 0.11, 0.01)]

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
    qoi_results_path = os.path.join(base_path, QOI_RESULTS_FILE)
    qoi_results_t_path = os.path.join(base_path, QOI_RESULTS_TIME_FILE)
    if os.path.exists(qoi_results_path):
        print(f'Warning: Analysis result file \"{qoi_results_path}\" already exists.')
        confirmation = input(f'         Really delete \"{qoi_results_path}\" to re-run analysis? (y/N)')
        if confirmation.lower() in ["y", "yes"]:
            os.remove(qoi_results_path)
            os.remove(qoi_results_t_path)

    # Parameter that was varied in parameter study
    var_parameter = {"module": "World.pNode[0].app[0].scheduler",
                     "name": "generationInterval"}

    # vectors to be analyzed
    vectors_analyzed = {
        "World.pNode[%].app[0].app": ["rcvdPkLifetime", "rcvdPkHostId"],
        "World.pNode[%].cellularNic.channelModel[0]": ["rcvdSinrUl"]
    }

    # scalar values to be analyzed
    # World.pNode[1].cellularNic.mac
    scalars_analyzed = {
        "World.pNode[%].cellularNic.mac": ["receivedPacketFromLowerLayer:count"],
        "World.pNode[%].udp": ["packetReceived:count"]
    }

    if not os.path.exists(qoi_results_path):
        qoi_results = pd.DataFrame()
        qoi_results_t = pd.DataFrame()
        for config in configs:
            qoi, qoi_t = analyze_parameter_study(base_path, config, var_parameter, vectors_analyzed,
                                                 scalars_analyzed)
            qoi_results = pd.concat([qoi_results, qoi])
            qoi_results_t = pd.concat([qoi_results_t, qoi_t])
        qoi_results.to_csv(qoi_results_path, sep=" ")
        qoi_results_t.to_csv(qoi_results_t_path, sep=" ")
    else:
        qoi_results = pd.read_csv(qoi_results_path, sep=" ")
        qoi_results_t = pd.read_csv(qoi_results_t_path, sep=" ")

    # create plots
    print(qoi_results)
    config_descriptions = {"sumoBottleneck": "bottleneck scenario: sumo",
                           "vadereBottleneck": "bottleneck scenario: vadere"}
    plot_pvar_delay(qoi_results, "Inter-TX Time [s]", "Delay [s]", config_descriptions)
    plt.show()


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


def analyze_parameter_study(base_path: str, config: str, var_parameter: Dict[str, str], vectors: Dict[str, List[str]],
                            scalars: Dict[str, List[str]]) \
        -> Tuple[pd.DataFrame, pd.DataFrame]:
    """
    Analyze a parameter study whose results are stored under the specified base path.

    :param scalars: dictionary mapping module names to all scalars to be analyzed for this module
    :param var_parameter: varied parameter in form of dictionary
            with "module" mapped to module name, "name" mapped to parameter name
    :param vectors: dictionary mapping module names to all vectors to be analyzed for this module
    :param base_path: Path to the base location where simulation results to be analyzed are stored.
    :param config: Name of the OMNeT++ configuration to be analyzed.
    :return: A tuple of two DataFrames. The first contains the aggregated analysis results,
    the second contains the averaged, time-based data, e.g. for plotting the vectors over time.
    """
    # define what is to be analyzed
    # module = "World.pNode[%].app[0].app"
    #
    # vec_names = {
    #     "rcvdPkLifetime:vector": {
    #         "name": "rcvdPktLifetime",
    #         "dtype": float,
    #     },
    #     "rcvdPkHostId:vector": {"name": "srcHostId", "dtype": np.int32},
    # }

    # do analysis
    suq_run = SuqcRun(os.path.join(get_results_dir(base_path), config))
    par_to_sims = get_parameter_to_sims(suq_run, var_parameter["module"], var_parameter["name"])

    dfs = pd.DataFrame()
    dfs_scalar = pd.DataFrame()
    for param_value in par_to_sims.keys():
        print(f'Analyzing all simulations for parameter value: {param_value}')

        for sim in par_to_sims[param_value]:
            print(f'   Analyzing simulation: {sim.data_root}')
            new_data = _retrieve_vector_data(sim, vectors)
            new_scalar_data = _retrieve_scalar_data(sim, scalars)
            new_data = pd.concat({sim.label: new_data}, names=['run_label'])  # add run_label as multi-level idx
            new_scalar_data = pd.concat({sim.label: new_scalar_data}, names=['run_label'])
            new_data = pd.concat({param_value: new_data}, names=['param'])  # add parameter value as multi-level idx
            new_scalar_data = pd.concat({param_value: new_scalar_data}, names=['param'])
            dfs = pd.concat([dfs, new_data])
            dfs_scalar = pd.concat([dfs_scalar, new_scalar_data])

    # calculate aggregated statistics for each parameter value
    # aggregated_results = dfs.groupby(level=["param"])["rcvdPktLifetime"].agg(["mean", "std", "max", "min"])
    aggregated_results = dfs.groupby(level=["param"]).agg(["mean", "std", "max", "min"])
    additional_sca_results = dfs_scalar.groupby(level=["param"]).agg(["mean", "std", "max", "min"])
    aggregated_results = aggregated_results.merge(additional_sca_results, left_index=True, right_index=True,
                                                  how='outer')

    # calculate time-based averages (e.g. for plotting over time)
    time_avg_df = _time_average(dfs)

    # TODO: call plot methods for aggregated time-intervals
    data_to_plot = time_avg_df.xs('0.01s')["rcvdPkLifetime", "mean"]
    plt.scatter(data_to_plot.index, data_to_plot)
    data_to_plot = time_avg_df.xs('0.1s')["rcvdPkLifetime", "mean"]
    plt.scatter(data_to_plot.index, data_to_plot)
    plt.show()

    # add config as index level
    aggregated_results = pd.concat([aggregated_results], keys=[config], names=['config'])
    time_avg_df = pd.concat([time_avg_df], keys=[config], names=['config'])

    return aggregated_results, time_avg_df


def _retrieve_vector_data(sim, vectors):
    new_data = pd.DataFrame()
    for vector_module in vectors.keys():
        vec_data = sim.sql.vec_data_pivot(
            vector_module,
            _get_vector_names(vectors, vector_module),
            # append_index=["srcHostId"],
        )
        # merge vector data of different modules
        if new_data.empty:
            new_data = vec_data
        else:
            new_data = pd.merge(new_data, vec_data, on=["hostId", "time"], how="outer").sort_index()
    return new_data


def _retrieve_scalar_data(sim, scalars):
    new_data = pd.DataFrame()
    for scalar_module in scalars.keys():
        for sca_name in scalars[scalar_module]:
            sca_data = sim.sql.sca_data(
                module_name=scalar_module,
                scalar_name=sca_name,
            )
            sca_data.drop(columns='scalarName', inplace=True)
            sca_data.rename(columns={'scalarValue': sca_name}, inplace=True)

            # merge vector data of different modules
            if new_data.empty:
                new_data = sca_data
            else:
                # new_data = pd.merge(new_data, vec_data, on=["hostId"], how="outer").sort_index()
                new_data = pd.concat([new_data, sca_data], ignore_index=True, sort=False)
    return new_data

def _get_vector_names(vectors, vector_module):
    vec_names = {}
    for vec_name in vectors[vector_module]:
        vec_names[f"{vec_name}:vector"] = {"name": vec_name, "dtype": float}
    return vec_names


def _time_average(dfs, avg_interval=1.0):
    aggregated_time_series = dfs.reset_index("time").groupby(level=["param", "run_label"])
    # loop over all groups (runs) and calculate average over avg_interval [s]
    collected_time_series_list = list()
    max_time = np.ceil(dfs.index.get_level_values("time").max())

    for key, grp in aggregated_time_series:
        print(f"Time-based averaging {key} with {avg_interval}s intervals")
        avg = grp.groupby(pd.cut(grp["time"], np.arange(0, max_time, avg_interval))).aggregate(np.nanmean)
        avg.index = np.arange(0, max_time, avg_interval)[:len(np.arange(0, max_time, avg_interval)) - 1]
        avg = pd.concat([avg], keys=[key[1]], names=['run_label'])
        avg = pd.concat([avg], keys=[key[0]], names=['param'])
        collected_time_series_list.append(avg)

    time_avg_df = pd.concat(collected_time_series_list)
    time_avg_df.index.set_names('sampling_time', level=2, inplace=True)
    time_avg_df.index = time_avg_df.index.swaplevel('run_label', 'sampling_time')
    time_avg_df = time_avg_df.groupby(level=["param", "sampling_time"]).agg(["mean"])
    return time_avg_df


def plot_pvar_delay(data: pd.DataFrame, x_label: str, y_label: str,
                    configs: Dict[str, str]) -> (matplotlib.figure.Figure,
                                                 matplotlib.axes.Axes):
    plt.rc('font', size=20)
    fig, ax = plt.subplots()
    fig.set_size_inches(16, 9)

    for config in configs.keys():
        # selected_data = data.loc[[config]]
        selected_data = data[data["config"] == config]
        plt.scatter(selected_data["param"], selected_data["mean"],
                    label=configs[config])
        # plt.errorbar(selected_data["param"], selected_data["mean"], yerr=selected_data["std"], fmt="o",
        #              label=configs[config])
    plt.legend(loc="upper left")
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    return fig, ax


if __name__ == "__main__":
    main("./")
