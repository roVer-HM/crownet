from functools import partial
import itertools
import os
import string
from pandas.core.indexes import base
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator, coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
import random
from suqc.utils.SeedManager.SeedManager import SeedManager
from time import time_ns

from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider




def main(base_path):
    # Enviroment setup.
    #
    reps = 5   # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),    # debug only
        "cellAgeTTL", UnitValue.s(30.0),    # -1.0 no TTL in 
        "alpha", 0.75,
        "idStreamType", QString("insertionOrder"),
        "stepDist", 150.0,
        "zeroStep", BoolValue.FALSE)
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymf"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "idStreamType", QString("insertionOrder"),
    )
    time = UnitValue.s(100.0)
    opp_config = "final_stationary_mf"

    # def var(ped, opp_seed, pos_seed, cfg_tpl="misc_pos_"):
    #     return  {"omnet": {
    #             "extends": f"_stationary_m_base_single_cell, {cfg_tpl}{ped}_{pos_seed}",
    #             "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist,
    #             "seed-set": str(opp_seed),
    #             }}
    def var(ped, opp_seed, pos_seed, cfg_tpl="misc_pos_"):
        return  {"omnet": {
                "extends": f"_stationary_m_base_single_cell, {cfg_tpl}{ped}_{pos_seed}",
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist,
                "seed-set": str(opp_seed),
                "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 100ms)"
                }}

    seed_m = OmnetSeedManager(
        par_variations={},
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None,
        seed=time_ns(),
        )

    # choose reps number of random position setups between [0..5]  and set 
    # a random seed for each.
    opp_seeds = seed_m.random.sample(range(1, 255), k=reps)
    position_seeds = range(0, reps)
    seed = list(zip(opp_seeds, position_seeds))
    

    par_var = [ var(ped, opp_seed=s[0], pos_seed=s[1]) for ped, s in itertools.product([10, 26, 50, 76, 100, 126, 150, 176, 200], seed) ]
    par_var.extend([var(density, opp_seed=s[0], pos_seed=s[1], cfg_tpl="full_") for density, s in itertools.product(range(5), seed )])
    par_var.extend([var(density, opp_seed=s[0], pos_seed=s[1], cfg_tpl="quarter_") for density, s in itertools.product(range(5), seed )])
    parameter_variation = ParameterVariationBase().add_data_points(par_var)

    model = OmnetCommand()\
         .write_container_log() \
         .omnet_tag("latest") \
         .experiment_label("out")
    model.timeout = None
    model.qoi(["all"])
    model.verbose()
    model.set_seed_manager(seed_m) # log used creation seed


    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config=opp_config,
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""), # use omnet internal mobility models
        # mobility_sim=("vadere", "latest"), # use omnet internal mobility models
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
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out"
    )
    print("setup done")
    par_var, data = setup.run(min(10, len(par_var)))
    # par_var, data = setup.run(1)




if __name__ == "__main__":
    main("./")
