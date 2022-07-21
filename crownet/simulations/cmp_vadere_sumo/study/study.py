# Script for doing the simulation study.
#
# Launch this script via python3 or within your Python IDE to perform a simulation study.
import gc
import os
import pprint
from functools import partial

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
import feather
from enum import Enum
from typing import List, Dict, Any, Tuple
from multiprocessing import Pool
from itertools import repeat

from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator, coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider

from roveranalyzer.utils.path import PathHelper
from roveranalyzer.simulators.opp.scave import ScaveTool, SqlEmptyResult
from roveranalyzer.simulators.opp.utils import Simulation
from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun

# number of repetitions to be performed for each parameter set
REPS = 10  # for testing we use 2, for reliable results it should be significantly more (depending on stddev)

# estimated amount of memory per simulation
MEM_PER_SIM_VADERE_GB = 15
MEM_PER_SIM_SUMO_GB = 2
MEM_PER_SIM_OMNET_GB = 1

# file where analysis results are stored
QOI_RESULTS_FILE = 'qoi_results.csv'
QOI_RESULTS_TIME_FILE = 'qoi_results_t.csv'

_MP_NR_PROCESSES = 10  # maximum number of parallel processes when analyzing the data


def main(base_path):
    # ------------------------------------------------------------------------------------------------------------
    # Configure simulation
    # ------------------------------------------------------------------------------------------------------------
    simtime = UnitValue.s(600.0)

    # parameter variation: define parameters to be varied
    configs = ["sumoOnly2", "vadereOnly2", "sumoSimple", "sumoSimpleTcp", "vadereSimple", "vadereSimpleTcp",
               "sumoBottleneck", "sumoBottleneckTcp", "vadereBottleneck", "vadereBottleneckTcp"]

    # subsets of the configurations (used for testing and repeating single parts)
    # configs = ["sumoBottleneck", "vadereBottleneck"]
    # configs = ["sumoOnly2", "vadereOnly2"]
    # configs = ["sumoSimpleTcp", "vadereSimpleTcp"]
    # configs = ["sumoOnly2", "sumoSimpleTcp"]

    # define parameter variation template for configurations
    # The parameter p is lateron varied, usually in the range 1..10
    def var(configuration, p):
        if "Tcp" in configuration:
            # TCP (varies load based on replyLength)
            return {"omnet": {
                "sim-time-limit": simtime,
                "*.*Node[*].app[0].replyLength": f"intWithUnit(exponential({p * 10}kB))"
            }}

        # default: assume beacon app  (varies load based on generation interval)
        return {"omnet": {
            "sim-time-limit": simtime,
            "*.*Node[*].app[0].scheduler.generationInterval": f"{p * 0.01}s"
        }}

    # note: for modifying several parameters and create par_var for all combinations, use
    # itertools.product
    def par_var(configuration):
        return [var(configuration, round(inter, 3)) for inter in np.arange(1, 11, 1)]

    # ------------------------------------------------------------------------------------------------------------
    # Configure analysis that will be performed on the simulation results
    # ------------------------------------------------------------------------------------------------------------
    # Parameter that was varied in parameter study and is
    def var_parameter(c):
        if "Tcp" in c:
            # for HTTP/TCP, the size of the reply sent back from the HTTP server is varied
            return {"module": "World.pNode[0].app[0]",
                    "name": "replyLength"}
        # default:
        return {"module": "World.pNode[0].app[0].scheduler",
                "name": "generationInterval"}

    # vectors to be analyzed
    def vectors_analyzed(c):
        if "Tcp" in c:
            # for HTTP/TCP, we evaluate the RTT reported by TCP
            # and the 5G New Radio Signal to Noise + Interference Ration (SINR) in the uplink
            return {
                "World.pNode[%].tcp.conn%": ["rtt"],
                "World.pNode[%].cellularNic.nrChannelModel[0]": ["rcvdSinrUl"]
            }
        # default:
        return {
            "World.pNode[%].app[0].app": ["rcvdPkLifetime", "rcvdPkHostId"],
            "World.pNode[%].cellularNic.channelModel[0]": ["rcvdSinrUl"]
        }

    # scalar values to be analyzed
    # World.pNode[1].cellularNic.mac
    def scalars_analyzed(c):
        if "Tcp" in c:
            return {
                "World.pNode[%].cellularNic.mac": ["receivedPacketFromLowerLayer:count"],
                "World.pNode[%].app[%]": ["packetReceived:count"]
            }
        # default
        return {
            "World.pNode[%].cellularNic.mac": ["receivedPacketFromLowerLayer:count"],
            "World.pNode[%].udp": ["packetReceived:count"]
        }

    # ------------------------------------------------------------------------------------------------------------
    # run simulation studies
    # ------------------------------------------------------------------------------------------------------------
    for config in configs:
        if "sumo" in config:
            mobility_type = MobilitySimulatorType.SUMO
        elif "vadere" in config:
            mobility_type = MobilitySimulatorType.VADERE
        else:
            raise ValueError("Cannot determine mobility type based on config name.")
        run_parameter_study(base_path, mobility_type, config, par_var(config))

    # ------------------------------------------------------------------------------------------------------------
    # perform analysis (as configured above)
    # ------------------------------------------------------------------------------------------------------------
    qoi_results_path = os.path.join(base_path, QOI_RESULTS_FILE)
    qoi_results_t_path = os.path.join(base_path, QOI_RESULTS_TIME_FILE)
    if os.path.exists(qoi_results_path):
        print(f'Warning: Analysis result file \"{qoi_results_path}\" already exists.')
        confirmation = input(f'         Really delete \"{qoi_results_path}\" to re-run analysis? (y/N)')
        if confirmation.lower() in ["y", "yes"]:
            os.remove(qoi_results_path)
            os.remove(qoi_results_t_path)

    if not os.path.exists(qoi_results_path):
        qoi_results = pd.DataFrame()
        qoi_results_t = pd.DataFrame()
        for config in configs:
            _analysis_path = SuqcRun(os.path.join(get_results_dir(base_path), config)).base_path
            _qoi_part_path = os.path.join(_analysis_path, f"qoi_part_{config}.csv")
            _qoi_t_part_path = os.path.join(_analysis_path, f"qoi_t_part_{config}.csv")
            if not os.path.exists(_qoi_part_path):  # or not os.path.exists(_qoi_t_part_path):
                # no existing data for this configuration - need to run analysis
                qoi, qoi_t = analyze_parameter_study(base_path, config, var_parameter(config), vectors_analyzed(config),
                                                     scalars_analyzed(config))
                qoi.to_csv(_qoi_part_path, sep=" ")
                if qoi_t is not None:
                    qoi_t.to_csv(_qoi_t_part_path, sep=" ")
            else:
                # have existing data from previous analysis - load it
                print(f"Loading: {_qoi_part_path}")
                if os.path.exists(_qoi_t_part_path):
                    qoi, qoi_t = (pd.read_csv(_qoi_part_path, sep=" ", index_col=[0, 1], header=[0, 1]),
                                  pd.read_csv(_qoi_t_part_path, sep=" ", index_col=[0, 1], header=[0, 1]))
                else:
                    qoi, qoi_t = (pd.read_csv(_qoi_part_path, sep=" ", index_col=[0, 1], header=[0, 1]), None)

            qoi_results = pd.concat([qoi_results, qoi], copy=False)
            if qoi_t is not None:
                qoi_results_t = pd.concat([qoi_results_t, qoi_t], copy=False)

        # save global analysis results (including data of all configs)
        qoi_results.to_csv(qoi_results_path, sep=" ")
        qoi_results_t.to_csv(qoi_results_t_path, sep=" ")
    else:
        qoi_results = pd.read_csv(qoi_results_path, sep=" ", index_col=[0, 1], header=[0, 1])
        qoi_results_t = pd.read_csv(qoi_results_t_path, sep=" ", index_col=[0, 1], header=[0, 1])

    # ------------------------------------------------------------------------------------------------------------
    # create plots (based on results of analysis)
    # ------------------------------------------------------------------------------------------------------------
    print(qoi_results)

    _SUMO_MM = "sumo: striping mobility model"
    _VADER_MM = "vadere: optimal steps mobility model"

    config_descriptions = {"scenario": "bottleneck_udp_sl",
                           "sumoBottleneck": _SUMO_MM,
                           "vadereBottleneck": _VADER_MM}

    plot_config(qoi_results, config_descriptions)
    plt.show()

    config_descriptions = {"scenario": "bottleneck_tcp_nr",
                           "sumoBottleneckTcp": _SUMO_MM,
                           "vadereBottleneckTcp": _VADER_MM}

    plot_config(qoi_results, config_descriptions)

    config_descriptions = {"scenario": "sidewalk_udp_sl",
                           "sumoSimple": _SUMO_MM,
                           "vadereSimple": _VADER_MM}

    plot_config(qoi_results, config_descriptions)
    plt.show()

    config_descriptions = {"scenario": "sidewalk_tcp_nr",
                           "sumoSimpleTcp": _SUMO_MM,
                           "vadereSimpleTcp": _VADER_MM}

    plot_config(qoi_results, config_descriptions)
    plt.show()

    config_descriptions = {"scenario": "sidewalk_only2",
                           "sumoOnly2": _SUMO_MM,
                           "vadereOnly2": _VADER_MM}

    plot_config(qoi_results, config_descriptions)
    plt.show()


def plot_config(qoi_results, config_descriptions):
    if any("Tcp" in s for s in config_descriptions.keys()):
        print("Plotting Tcp related plots...")
        if True:
            plot_pvar(qoi_results, config_descriptions, "rtt", x_label="Mean HTTP Response Size [kB]",
                      x_multiplier=10.0, reindex=-1,
                      y_label="TCP RTT [s]")

        if True:
            plot_pvar(qoi_results, config_descriptions, "rcvdSinrUl",
                      x_label="Mean HTTP Response Size [kB]", y_label="Uplink SINR",  # ylimits=[5.25, 9],
                      x_multiplier=10.0, reindex=-1, legend_loc="lower right")

        if True:
            plot_pvar(qoi_results, config_descriptions, "packetReceived:count",
                      x_label="Mean HTTP Response Size [kB]",
                      x_multiplier=10.0,
                      y_multiplier=0.001,
                      y_label="Number of Received Packets ($\\cdot 10^3$, application layer)",
                      reindex=-1,
                      ylimits=[0, 12],
                      with_errorbar=True)
    else:
        print("Plotting Udp/Sideling related plots..")
        if True:
            plot_pvar(qoi_results, config_descriptions, "rcvdPkLifetime", x_label="Inter-TX Time [ms]",
                      y_label="D2D Delay (application layer) [ms]", xlimits=[44.5, 104.5], ylimits=[0, 200],
                      x_multiplier=10.0, y_multiplier=1000.0, with_errorbar=True)

        if True:
            plot_pvar(qoi_results, config_descriptions, "rcvdSinrUl", x_label="Inter-TX Time [ms]",
                      x_multiplier=10.0,  legend_loc="lower right",  # ylimits=[40, 50],
                      y_label="Uplink SINR")

        if True:
            plot_pvar(qoi_results, config_descriptions, "receivedPacketFromLowerLayer:count",
                      x_label="Inter-TX Time [ms]",
                      x_multiplier=10.0, y_multiplier=0.001,
                      y_label="Number of Received Packets ($\\cdot 10^3$, link layer)",
                      with_errorbar=True)

        if True:
            plot_pvar(qoi_results, config_descriptions, "packetReceived:count",
                      x_label="Inter-TX Time [ms]",
                      x_multiplier=10.0, y_multiplier=0.001,
                      y_label="Number of Received Packets ($\\cdot 10^3$, application layer)",
                      with_errorbar=True)


# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# --- generic utility functions - should be moved to roveranalyzer lateron
# TODO: move to roveranalyzer and remove duplicate code here and in analysis.py

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


def get_results_dir(base_path: string):
    study_name = os.path.basename(__file__).replace(".py", "")
    return os.path.abspath(base_path + "../results/" + study_name + "/")


def run_parameter_study(base_path: str, mobility_sim: MobilitySimulatorType, config: str,
                        par_var: List[Dict[str, Any]]):
    par_var = OmnetSeedManager(
        par_variations=par_var,
        rep_count=REPS,
        omnet_fixed=False,
        vadere_fixed=False,
        seed=time.time_ns(),
        # seed = 0 # only for debugging/study setup
    ).get_new_seed_variation()

    # Enviroment setup.
    # 
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = get_results_dir(base_path)
    if os.path.exists(os.path.join(base_dir, config)):
        print(f'Warning: {base_dir} already exists - skipping configuration {config}')
        return

    os.makedirs(base_dir, exist_ok=True)

    parameter_variation = ParameterVariationBase().add_data_points(par_var)

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
        handle_existing="force_replace",
        # handle_existing="ask_user_replace",
        scenario_provider_class=partial(
            VariationBasedScenarioProvider, par_var=parameter_variation
        ),

    )

    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[]
    )

    _rnd = ''.join(random.choices(string.ascii_lowercase + string.digits, k=6))

    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out=config + "_runscript.out",
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

    suq_run = SuqcRun(os.path.join(get_results_dir(base_path), config))
    par_to_sims = get_parameter_to_sims(suq_run, var_parameter["module"], var_parameter["name"])

    dfs_vector = pd.DataFrame()
    dfs_scalar = pd.DataFrame()
    if _MP_NR_PROCESSES > 1:
        # run multiple analysis processes in parallel - since calculation might take a while
        print(f'Analyzing all simulations for parameter values: {par_to_sims.keys()}')
        with Pool(_MP_NR_PROCESSES) as p:

            for param_value in par_to_sims.keys():
                f_params = []
                for sim in par_to_sims[param_value]:
                    f_params.append((sim, param_value, vectors, scalars))
                print(f'Starting analysis for {var_parameter["name"]} == {param_value} ...')
                vec_sca_data = p.starmap(_analyze_singe_simulation, f_params)

                # combine all analysis data in a single data frame
                print(f'Combining dataframes for {var_parameter["name"]} == {param_value} ...')
                _vec_data_all = pd.concat(list(zip(*vec_sca_data))[0])
                _sca_data_all = pd.concat(list(zip(*vec_sca_data))[1])
                print(f'Appending results for {var_parameter["name"]} == {param_value} ...')
                # TODO: replace by list operations (cheaper in terms of runtime)
                dfs_vector = pd.concat([dfs_vector, _vec_data_all], copy=False)
                dfs_scalar = pd.concat([dfs_scalar, _sca_data_all], copy=False)
                _vec_data_all = pd.DataFrame()
                _sca_data_all = pd.DataFrame()
    else:
        # use main thread for all calculations
        for param_value in par_to_sims.keys():
            print(f'Analyzing all simulations for parameter value: {param_value}')
            for sim in par_to_sims[param_value]:
                (new_vector_data, new_scalar_data) = _analyze_singe_simulation(sim, param_value, vectors, scalars)
                # TODO: replace by list operations (cheaper in terms of runtime)
                dfs_vector = pd.concat([dfs_vector, new_vector_data])
                dfs_scalar = pd.concat([dfs_scalar, new_scalar_data])

    # calculate aggregated statistics for each parameter value
    print(f'Calculating aggregated statistics over all runs...')
    aggregated_results = dfs_vector.groupby(level=["param"]).agg(["mean", "std", "max", "min"])
    additional_sca_results = dfs_scalar.groupby(level=["param"]).agg(["mean", "std", "max", "min"])
    aggregated_results = aggregated_results.merge(additional_sca_results, left_index=True, right_index=True,
                                                  how='outer')

    # save temporary file (so we can continue upon failure...)
    # print(f'Saving temporary files...')
    # aggregated_results.reset_index().to_feather('./aggregated_results_tmp.feather')
    # dfs_vector.reset_index().to_feather('./dfs_vector_tmp.feather')

    # add config as index level
    aggregated_results = pd.concat([aggregated_results], keys=[config], names=['config'])

    # calculate time-based averages (e.g. for plotting over time)
    # print(f'Calculating time-based averages...')
    time_avg_df = None
    # time_avg_df = _time_average(dfs_vector)

    if time_avg_df is not None:
        # TODO: call plot methods for aggregated time-intervals
        # for now, we do just some example plots
        if "Tcp" in config:
            data_to_plot = time_avg_df.xs('intWithUnit(exponential(10kB))')["rtt", "mean"]
            plt.scatter(data_to_plot.index, data_to_plot)
            data_to_plot = time_avg_df.xs('intWithUnit(exponential(100kB))')["rtt", "mean"]
            plt.scatter(data_to_plot.index, data_to_plot)
        else:
            data_to_plot = time_avg_df.xs('0.01s')["rcvdPkLifetime", "mean"]
            plt.scatter(data_to_plot.index, data_to_plot)
            data_to_plot = time_avg_df.xs('0.1s')["rcvdPkLifetime", "mean"]
            plt.scatter(data_to_plot.index, data_to_plot)
        plt.show()
        # add config as index level
        time_avg_df = pd.concat([time_avg_df], keys=[config], names=['config'])

    return aggregated_results, time_avg_df


def _analyze_singe_simulation(simulation, parameter_value, vectors, scalars):
    print(f'   Analyzing simulation for parameter {parameter_value}: {simulation.data_root}')
    try:
        vec_data = _retrieve_vector_data(simulation, vectors)
        sca_data = _retrieve_scalar_data(simulation, scalars)
    except SqlEmptyResult as result:
        print(f'_analyze_singe_simulation: error - no data available!')
        print(f'simulation label: {simulation.label}')
        print(f'vectors: {vectors}')
        print(f'scalars: {scalars}')
        print(f'{result}')
        raise result

    vec_data = pd.concat({simulation.label: vec_data}, names=['run_label'], copy=False)  # run_label as multi-level idx
    sca_data = pd.concat({simulation.label: sca_data}, names=['run_label'], copy=False)
    vec_data = pd.concat({parameter_value: vec_data}, names=['param'], copy=False)  # parameter value as multi-level idx
    sca_data = pd.concat({parameter_value: sca_data}, names=['param'], copy=False)
    return vec_data, sca_data


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
    _max_time = np.ceil(dfs.index.get_level_values("time").max())
    # dfs.reset_index(["time", "hostId"], inplace=True)
    # aggregated_time_series = dfs.groupby(level=["param", "run_label"], sort=False, observed=True)
    dfs.reset_index(inplace=True)
    aggregated_time_series = dfs.groupby(["param", "run_label"], sort=False, observed=True)
    dfs = dfs.iloc[0:0]  # drop all data
    gc.collect()  # trigger garbage collection
    # loop over all groups (runs) and calculate average over avg_interval [s]
    collected_time_series_list = list()

    for key, grp in aggregated_time_series:
        print(f"Time-based averaging {key} with {avg_interval}s intervals")
        avg = grp.groupby(pd.cut(grp["time"], np.arange(0, _max_time, avg_interval))).aggregate(np.nanmean)
        avg.index = np.arange(0, _max_time, avg_interval)[:len(np.arange(0, _max_time, avg_interval)) - 1]
        avg = pd.concat([avg], keys=[key[1]], names=['run_label'], copy=False)
        avg = pd.concat([avg], keys=[key[0]], names=['param'], copy=False)
        collected_time_series_list.append(avg)

    time_avg_df = pd.concat(collected_time_series_list, copy=False)
    time_avg_df.index.set_names('sampling_time', level=2, inplace=True)
    time_avg_df.index = time_avg_df.index.swaplevel('run_label', 'sampling_time')
    time_avg_df = time_avg_df.groupby(level=["param", "sampling_time"]).agg(["mean"])
    time_avg_df.sort_index(inplace=True)
    gc.collect()  # trigger garbage collection
    return time_avg_df


def plot_pvar(data: pd.DataFrame,
              configs: Dict[str, str], data_name: str,
              x_multiplier: float = 1.0,
              y_multiplier: float = 1.0,
              x_label: str = None, y_label: str = None,
              legend_loc="upper right",
              xlimits=None, ylimits=None,
              reindex=0,
              with_errorbar=False) -> (matplotlib.figure.Figure,
                                       matplotlib.axes.Axes):
    _symbols = ["o", "*", "v", "s"]

    plt.rc('font', size=20)
    fig, ax = plt.subplots()
    if xlimits:
        ax.set_xlim(xlimits)
    if ylimits:
        ax.set_ylim(ylimits)
    fig.set_size_inches(9, 9)
    _plot_nr = 0
    for config in configs.keys():
        if config == "scenario":
            continue
        selected_data = data.loc[config]
        x_values = [float(x) * x_multiplier for x in range(1, len(selected_data.index) + 1, 1)]
        y_values = y_multiplier * selected_data[data_name, "mean"]
        y_err_values = y_multiplier * selected_data[data_name, "std"]
        if reindex != 0:
            y_values = y_values.reindex(index=np.roll(y_values.index, reindex))
            y_err_values = y_err_values.reindex(index=np.roll(y_err_values.index, reindex))
        if with_errorbar:
            plt.errorbar(x_values, y_values,
                         yerr=y_err_values,
                         label=configs[config], fmt=_symbols[_plot_nr], markersize=15,
                         capsize=10, elinewidth=3)
        else:
            plt.scatter(x_values, y_values,
                        label=configs[config], marker=_symbols[_plot_nr], s=60)
        _plot_nr = _plot_nr + 1

    plt.legend(loc=legend_loc)
    if x_label:
        ax.set_xlabel(x_label)
    if y_label:
        ax.set_ylabel(y_label)

    scenario_name = configs["scenario"]
    data_name = data_name.replace(":", "_")

    plt.savefig(f"{scenario_name}_{data_name}.pdf")
    plt.savefig(f"{scenario_name}_{data_name}.png")
    plt.savefig(f"{scenario_name}_{data_name}.svg")

    # add small info text so we know which graphic this is
    fig.suptitle(f'File: {scenario_name}_{data_name}.png', fontsize=10)
    return fig, ax


if __name__ == "__main__":
    main("./")
