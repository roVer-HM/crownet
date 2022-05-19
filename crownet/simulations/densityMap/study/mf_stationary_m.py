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

from suqc.utils.general import get_env_name



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

    par_var = [
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_10_0",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..3].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..3].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_10_1",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..3].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..3].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_10_2",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..3].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..3].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_50_0",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..15].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..15].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_50_1",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..15].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..15].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
        {
            "omnet": {
                "extends": "_stationary_m_base, misc_pos_50_2",
                "sim-time-limit": time,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist,
                "*.misc[0..15].app[0].app.stopTime" : UnitValue.s(40),
                "*.misc[0..15].app[1].app.stopTime" : UnitValue.s(40),
                },
        },
    ]

    par_var = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None).get_new_seed_variation()

    model = OmnetCommand()\
         .write_container_log() \
         .omnet_tag("latest") \
         .experiment_label("out")
    model.timeout = None
    model.qoi(["all"])
    model.verbose()


    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_stationary_m",
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""), # use omnet internal mobility models
        # mobility_sim=("vadere", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace"
    )
    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[]
        )

    _r = SeedManager.get_new_random_object()
    _rnd = ''.join(_r.choices(string.ascii_lowercase + string.digits, k=6))
    print(f"use random suffix: '{_rnd}'")
    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out"
    )
    print("setup done")
    par_var, data = setup.run(len(par_var))
    # par_var, data = setup.run(1)




if __name__ == "__main__":
    main("./")
