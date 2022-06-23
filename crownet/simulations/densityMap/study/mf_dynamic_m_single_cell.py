from copy import deepcopy
from functools import partial
from itertools import product
import os
import timeit as it
from time import time_ns
from timeit import repeat
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


def main(base_path):
    reps = 10  # seed-set
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
        "zeroStep",
        BoolValue.FALSE,
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
    scenario_ped_180 = QString(
        "vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario"
    )
    # scenario_ped_120 = QString("vadere/scenarios/mf_m_dyn_const_4e20s_15x8_120.scenario")
    scenario_exp_25 = QString("vadere/scenarios/mf_dyn_exp_25.scenario")
    scenario_exp_5 = QString("vadere/scenarios/mf_dyn_exp_05.scenario")
    t = UnitValue.s(800.0)
    # t = UnitValue.s(2.0)
    source_end_time = 400.0
    source_id_range = range(1117, 1131)

    par_var = [
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": scenario_exp_25,  # iter arrival time of 25s (for each source)
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **{
                    f"sources.[id=={id}].endTime": source_end_time
                    for id in source_id_range
                },
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": scenario_exp_5,  # iter arrival time of 5s (for each source)
                "*.pNode[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.pNode[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
            },
            "vadere": {
                **{
                    f"sources.[id=={id}].endTime": source_end_time
                    for id in source_id_range
                },
            },
        },
    ]

    alpha = [0.5, 0.65, 0.80, 0.95]
    # dist = [20, 80, 120, 200]
    dist = [80, 120, 160, 200]
    alpha_dist = list(product(alpha, dist))
    alpha_dist.append([1.0, 999])  # distance has no effect thus just one
    par_var_tmp = []
    for run in par_var:
        for alpha, dist in alpha_dist:
            _run = deepcopy(run)  # copy
            _run["omnet"]["*.pNode[*].app[1].app.mapCfg"] = mapCfgYmfDist.copy(
                "alpha", alpha, "stepDist", dist
            )
            par_var_tmp.append(_run)
        print("done")

    par_var = par_var_tmp

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
    # base_dir = os.path.abspath("../suqc")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_dynamic_m_vadere",
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

    ts = it.default_timer()
    par_var, data = setup.run(min(8, len(par_var)))
    # par_var, data = setup.run(1)y
    print(f"Study: took {(it.default_timer() - ts)/60:2.4f} minutes")


if __name__ == "__main__":
    main("./")
