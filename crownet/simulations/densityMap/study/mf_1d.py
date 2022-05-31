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

def source_max_spawn(ids, num=0):
    return { f"sources.[id=={id}].maxSpawnNumberTotal": num for id in ids}
def source_spawn(ids, num=0):
    return { f"sources.[id=={id}].spawnNumber": num for id in ids}
def source_dist_par(ids, name, value):
    return { f"sources.[id=={id}].distributionParameters.{name}": value for id in ids}
def source(ids, distribution, params, start,  end, spawnNumber, maxSpawnNumber):
    ret = {}
    for id in ids:
        ret[f"sources.[id=={id}].spawnNumber.interSpawnTimeDistribution"] = distribution
        for k, v in params.items():
            ret.update(**source_dist_par([id], k, v))
        ret[f"sources.[id=={id}].spawnNumber.startTime"] = start
        ret[f"sources.[id=={id}].spawnNumber.endTime"] = end
        ret.update(**source_spawn([id], spawnNumber))
        ret.update(**source_max_spawn([id], maxSpawnNumber))
    return ret
        


def main(base_path):
    reps = 1    # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "alpha", 0.75,
        "idStreamType", QString("insertionOrder"),
        "stepDist", 50.0,
        "zeroStep", BoolValue.TRUE)
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymf"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "idStreamType", QString("insertionOrder"),
    )
    s_5_20_poisson = QString("vadere/scenarios/mf_1d_m_poisson_2x5m_d20m.scenario")
    s_5_20_const = QString("vadere/scenarios/mf_1d_m_const_2x5m_d20m.scenario")
    s_5_280_poisson = QString("vadere/scenarios/mf_1d_m_poisson_2x5m_d280m.scenario")
    s_5_280_const = QString("vadere/scenarios/mf_1d_m_const_2x5m_d280m.scenario")
    t = UnitValue.s(320.0)

    source_top_left = 102
    source_top_right = 103
    source_bottom_left = 100
    source_bottom_right = 101
    source_range = [100, 101, 102, 103]

    opp_config = "final_1d"
    ext_single_cell = "_1d"

    par_var = [
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_280_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=1),
            }
        },
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_280_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **source_dist_par([source_top_right, source_bottom_left], "updateFrequency", 50.0),
            }
        },
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_280_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **source_dist_par([source_top_right, source_bottom_left], "updateFrequency", 20.0),
            }
        },
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=1),
            }
            
        },
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **source_dist_par([source_top_right, source_bottom_left], "updateFrequency", 50.0),
            }
        },
        {
            "omnet": {
                "extends": ext_single_cell,
                "sim-time-limit": t,
                "**.vadereScenarioPath" : s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg":
                    mapCfgYmfDist.copy("stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE, "cellAgeTTL", UnitValue.s(30.0)),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                },
            "vadere": {
                **source_max_spawn([source_top_left, source_bottom_right], num=0),
                **source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **source_dist_par([source_top_right, source_bottom_left], "updateFrequency", 25.0),
            }
        },
    ]

    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None,
        seed = time_ns()
        )
    par_var = seed_m.get_new_seed_variation()

    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    
    # Model := Call to run_script.py
    # This will define how the simulation is run.
    # These settings here will override defaults set during CrownetRequest setup.
    # model = OmnetCommand() \
    #     .write_container_log() \
    #         .omnet_tag("latest") \
    #             .experiment_label("out")
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
