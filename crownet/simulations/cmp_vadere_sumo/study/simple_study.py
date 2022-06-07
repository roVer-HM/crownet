# Script for doing the simulation study.
#
# Launch this script via python3 or within your Python IDE to perform a simulation study.

import os, shutil, psutil
import string
from typing import List, Dict, Any
from enum import Enum
from pandas.core.indexes import base
from scipy import rand
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator, coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
import pprint
import random

# number of repetitions to be performed for each parameter set
REPS = 4

# estimated amount of memory per simulation
MEM_PER_SIM_VADERE_GB = 11
MEM_PER_SIM_SUMO_GB = 1
MEM_PER_SIM_OMNET_GB = 1


class MobilitySimulatorType(Enum):
    OMNET = 1
    VADERE = 2
    SUMO = 3


def max_parallel_sims(mobility_sim: MobilitySimulatorType) -> int:
    estimated_mem_per_sim = MEM_PER_SIM_OMNET_GB
    if mobility_sim == MobilitySimulatorType.SUMO:
        estimated_mem_per_sim = estimated_mem_per_sim + MEM_PER_SIM_SUMO_GB
    elif mobility_sim == MobilitySimulatorType.VADERE:
        estimated_mem_per_sim = estimated_mem_per_sim + MEM_PER_SIM_VADERE_GB

    max_sims = int(((psutil.virtual_memory()[4]) / (1024 * 1024 * 1024)) / estimated_mem_per_sim)
    return max_sims


def main(base_path):
    time = UnitValue.s(600.0)

    # only2: SUMO
    par_var = [
        {
            "omnet": {
                "sim-time-limit": time,
            },
        },
    ]
    run_parameter_study(base_path, MobilitySimulatorType.SUMO, "sumoOnly2", par_var)

    # only2: VADERE
    par_var = [
        {
            "omnet": {
                "sim-time-limit": time,
            },
        },
    ]
    run_parameter_study(base_path, MobilitySimulatorType.VADERE, "vadereOnly2", par_var)



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
    study_name = __file__.replace(".py", "")
    base_dir = os.path.abspath("../results/" + study_name + "/")
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
    nr_parallel = max_parallel_sims (mobility_sim);
    print(f'estimated maximum number of parallel simulations: {nr_parallel}')
    par_var, data = setup.run(nr_parallel)

if __name__ == "__main__":
    main("./")
