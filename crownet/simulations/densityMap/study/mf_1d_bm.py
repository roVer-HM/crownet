from functools import partial
import os
from time import time_ns
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider
from roveranalyzer.simulators.vadere.bonnmotion_traces import generate_traces
import roveranalyzer.simulators.vadere.vadere_variation_helper as v


s_5_20_const = QString("vadere/scenarios/mf_1d_m_const_2x5m_d20m.scenario")
t = UnitValue.s(5000.0)

source_top_left = 102
source_top_right = 103
source_bottom_left = 100
source_bottom_right = 101


def main(base_path):
    reps = 5  # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL",
        UnitValue.s(30.0),
        "alpha",
        0.75,
        "idStreamType",
        QString("insertionOrder"),
        "stepDist",
        150.0,
    )
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("ymf"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL",
        UnitValue.s(30.0),
        "idStreamType",
        QString("insertionOrder"),
    )

    par_var = [
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 50.0
                ),
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 50.0
                ),
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.pNode[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 25.0
                ),
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": s_5_20_const,
                "*.pNode[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 25.0
                ),
            },
        },
    ]

    for v in par_var:
        # -7492697142818052001
        v["vadere"]["attributesSimulation.useFixedSeed"] = True
        v["vadere"]["attributesSimulation.fixedSeed"] = -7492697142818052001
        v["omnet"]["*.traci.launcher.useVadereSeed"] = BoolValue.TRUE

    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None,  # set manually
        seed=time_ns(),
    )
    par_var = seed_m.get_new_seed_variation()

    parameter_variation = ParameterVariationBase().add_data_points(par_var)

    model = (
        VadereOppCommand()
        .write_container_log()
        .omnet_tag("latest")
        .experiment_label("out")
    )
    model.timeout = None
    model.qoi(["all"])
    model.verbose()
    model.set_seed_manager(seed_m)  # log used creation seed

    # Enviroment setup.
    #
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_1d",
        opp_basename="omnetpp.ini",
        # mobility_sim=("omnet", ""), # use omnet internal mobility models
        mobility_sim=("vadere", "latest"),  # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
        scenario_provider_class=partial(
            VariationBasedScenarioProvider, par_var=parameter_variation
        ),
    )
    env.copy_data(base_ini_file=ini_file)

    _rnd = SeedManager.rnd_suffix()
    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=coupled_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out",
    )
    print("setup done")
    par_var, data = setup.run(min(4, len(par_var)))
    # par_var, data = setup.run(1)y


def generate_bonnmotion():

    par_var = [
        {
            **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
            **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
            **v.source_dist_par(
                [source_top_right, source_bottom_left], "updateFrequency", 25.0
            ),
        },
        {
            **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
            **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
            **v.source_dist_par(
                [source_top_right, source_bottom_left], "updateFrequency", 50.0
            ),
        },
    ]

    base_output_path = os.path.join(
        os.path.dirname(__file__), f"{os.path.basename(__file__)[:-3]}.d"
    )

    generate_traces(
        scenarios=[os.path.abspath(os.path.join("..", s_5_20_const.value))],
        par_var_default=par_var,
        base_output_path=base_output_path,
        jobs=1,
        seeds=20,
        override_output=False,
    )


if __name__ == "__main__":
    # main("./")
    generate_bonnmotion()
