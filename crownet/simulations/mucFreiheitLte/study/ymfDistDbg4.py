import os, shutil
import string
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


def main(base_path):
    reps = 1  # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("all"),
        "cellAgeTTL",
        UnitValue.s(-1.0),
        "alpha",
        0.75,
        "idStreamType",
        QString("insertionOrder"),
        "stepDist",
        50.0,
    )
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("all"),
        "cellAgeTTL",
        UnitValue.s(-1.0),
        "idStreamType",
        QString("insertionOrder"),
    )
    scenario_3source = QString("vadere/scenarios/mf_circle2.scenario")
    t = UnitValue.s(600.0)

    par_var = [
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 15.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 50.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 75.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 90.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 90.0, "alpha", 0.75, "zeroStep", BoolValue.TRUE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "stepDist", 150.0, "alpha", 0.75, "zeroStep", BoolValue.FALSE
                ),
            }
        },
        {
            "omnet": {
                "extends": "ymfPlusDist_4s_multiple_packet",
                "sim-time-limit": t,
                "seed-set": "195",
                "**.vadereScenarioPath": scenario_3source,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmf,
            }
        },
    ]
    par_var = OmnetSeedManager(
        par_variations=par_var, rep_count=reps, omnet_fixed=True, vadere_fixed=None
    ).get_new_seed_variation()

    # Model := Call to run_script.py
    # This will define how the simulation is run.
    # These settings here will override defaults set during CrownetRequest setup.
    # model = OmnetCommand() \
    #     .write_container_log() \
    #         .omnet_tag("latest") \
    #             .experiment_label("out")
    model = (
        VadereOppCommand()
        .write_container_log()
        .omnet_tag("latest")
        .experiment_label("out")
    )
    model.timeout = None
    model.qoi(["all"])
    model.verbose()

    # Enviroment setup.
    #
    ini_file = os.path.abspath("../omnetpp_ymfd4s.ini")
    # base_dir = os.path.abspath("../suqc")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=__file__.replace(".py", ""),
        opp_config="final",
        opp_basename="omnetpp_ymfd4s.ini",
        # mobility_sim=("omnet", ""), # use omnet internal mobility models
        mobility_sim=("vadere", "latest"),  # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
    )
    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[
            os.path.abspath("../vadere/scenarios/mf_circle2.scenario"),
            os.path.abspath("../vadere/scenarios/mf_circle.scenario"),
        ],
    )

    _rnd = "".join(random.choices(string.ascii_lowercase + string.digits, k=6))
    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=coupled_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out",
    )
    print("setup done")
    par_var, data = setup.run(len(par_var))
    # par_var, data = setup.run(1)


if __name__ == "__main__":
    main("./")
