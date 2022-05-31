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
    reps = 1    # seed-set
    
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),    # debug only
        "cellAgeTTL", UnitValue.s(-1.0),    # -1.0 no TTL in 
        "alpha", 0.75,
        "idStreamType", QString("insertionOrder"),
        "stepDist", 150.0,
        "zeroStep", BoolValue.FALSE)
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog", BoolValue.TRUE,
        "mapTypeLog", QString("ymf"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "idStreamType", QString("insertionOrder"),
    )
    time = UnitValue.s(100.0)

    def var(ped, run):
        return  {"omnet": {
                "extends": f"_stationary_m_base_single_cell, misc_pos_{ped}_{run}",
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                }}

    par_var = [ var(ped, run) for ped, run in itertools.product([10, 26, 50, 76, 100, 126, 150, 176, 200], [0]) ]

    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None,
        seed=time_ns(),
        )
    par_var = seed_m.get_new_seed_variation()
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
        opp_config="final_stationary_mf",
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""), # use omnet internal mobility models
        # mobility_sim=("vadere", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
        scenario_provider_class=partial(VariationBasedScenarioProvider, par_var=parameter_variation)
    )
    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[]
        )

    _rnd = SeedManager.rnd_suffix(k=6)
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out"
    )
    print("setup done")
    # par_var, data = setup.run(len(par_var))
    par_var, data = setup.run(1)




if __name__ == "__main__":
    main("./")
