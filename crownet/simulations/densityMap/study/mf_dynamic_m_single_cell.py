from functools import partial
import os, shutil
import string
from time import time_ns
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

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider



def main(base_path):
    reps = 1    # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(30.0),
        "alpha", 0.75,
        "idStreamType", QString("insertionOrder"),
        "stepDist", 150.0,
        "zeroStep", BoolValue.FALSE)
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymf"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(30.0),
        "idStreamType", QString("insertionOrder"),
    )
    scenario_ped_180 = QString("vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
    # scenario_ped_120 = QString("vadere/scenarios/mf_m_dyn_const_4e20s_15x8_120.scenario") 
    # scenario_poisson = QString("vadere/scenarios/mf_m_dyn_poisson_02.scenario") 
    t = UnitValue.s(200.0)

    source_id_range = range(1117, 1132)

    par_var = [
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath" : scenario_ped_180,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("cellAgeTTL", UnitValue.s(-1.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **{ f"sources.[id=={id}].spawnNumber": 5 for id in source_id_range},
                **{ f"sources.[id=={id}].maxSpawnNumberTotal": 10 for id in source_id_range},
            }
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath" : scenario_ped_180,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("cellAgeTTL", UnitValue.s(60.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **{ f"sources.[id=={id}].spawnNumber": 3 for id in source_id_range},
                **{ f"sources.[id=={id}].maxSpawnNumberTotal": 6 for id in source_id_range},
            }
        },
    ]
    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=False,
        seed=time_ns(),
        # seed=0        # use for study debug/setup
        )
    
    par_var = seed_m.get_new_seed_variation()

    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    
    model = VadereOppCommand()\
         .write_container_log() \
         .omnet_tag("latest") \
         .experiment_label("out")
    model.timeout = None
    model.qoi(["all"])
    model.verbose()
    model.set_seed_manager(seed_m) # log used creation seed


    # Enviroment setup.
    #
    ini_file = os.path.abspath("../omnetpp.ini")
    # base_dir = os.path.abspath("../suqc")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_dynamic_m_vadere",
        opp_basename="omnetpp.ini",
        # mobility_sim=("omnet", ""), # use omnet internal mobility models
        mobility_sim=("vadere", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
        scenario_provider_class=partial(VariationBasedScenarioProvider, par_var=parameter_variation)
    )
    env.copy_data(base_ini_file=ini_file)


    _rnd = SeedManager.rnd_suffix()
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
        creator=coupled_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out"
    )
    print("setup done")
    par_var, data = setup.run(len(par_var))
    # par_var, data = setup.run(1)y



if __name__ == "__main__":
    main("./")
