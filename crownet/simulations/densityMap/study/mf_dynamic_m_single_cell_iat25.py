from __future__ import annotations
from copy import deepcopy
from functools import partial
from glob import glob
from itertools import product
import json
import os
from os.path import join
import shutil
import timeit as it
from time import time_ns
from suqc.CommandBuilder.JarCommand import JarCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import coupled_creator, opp_creator
from suqc.parameter.postchanges import PostScenarioChangesBase
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CrownetRequest, DictVariation
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
import pandas as pd

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider


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
scenario_ped_180 = QString("vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario")
# scenario_ped_120 = QString("vadere/scenarios/mf_m_dyn_const_4e20s_15x8_120.scenario")
scenario_exp_25 = QString("vadere/scenarios/mf_dyn_exp_25.scenario")
scenario_exp_15 = QString("vadere/scenarios/mf_dyn_exp_15.scenario")
scenario_exp_5 = QString("vadere/scenarios/mf_dyn_exp_05.scenario")
t = UnitValue.s(800.0)
# t = UnitValue.s(2.0)
source_end_time = 400.0
source_id_range = range(1117, 1131)

alpha = [0.5, 0.65, 0.80, 0.95]
# alpha = [0.05, 0.15, 0.25, 0.35]
# dist = [20, 80, 120, 200]
dist = [80, 120, 160, 200]
alpha_dist = list(product(alpha, dist))
alpha_dist.append([1.0, 999])  # distance has no effect thus just one


def par_var_with_vadere():
    return "final_dynamic_m_vadere", [
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
                "**.vadereScenarioPath": scenario_exp_15,  # iter arrival time of 5s (for each source)
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


def par_var_bonn_motion():
    return "final_dynamic_m_bonn_motion", [
        {
            "omnet": {
                "sim-time-limit": t,
                "**.vadereScenarioPath": scenario_exp_25,  # iter arrival time of 25s (for each source)
                "*.bonnMotionServer.traceFile": "to/be/changed",
                "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
            }
        },
        # {
        #     "omnet": {
        #         "sim-time-limit": t,
        #         "**.vadereScenarioPath": scenario_exp_15,  # iter arrival time of 25s (for each source)
        #         "*.bonnMotionServer.traceFile": "to/be/changed",
        #         "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
        #         "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
        #     },
        # },
    ]


def create_variation_with_bonn_motion(seed):
    opp_config, par_var = par_var_bonn_motion()

    par_var_tmp = []
    for run in par_var:
        for alpha, dist in alpha_dist:
            _run = deepcopy(run)  # copy
            _run["omnet"]["*.misc[*].app[1].app.mapCfg"] = mapCfgYmfDist.copy(
                "alpha", alpha, "stepDist", dist
            )
            par_var_tmp.append(_run)

    par_var = par_var_tmp

    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=False,
        seed=seed,
    )
    # todo: replace vadere with bonn motion here

    par_var = seed_m.get_new_seed_variation()
    for var in par_var:
        del var["vadere"]
        opp = var["omnet"]
        bm_file_name = f"trace/trace_{os.path.basename(opp['**.vadereScenarioPath'].value)}_{opp['*.traci.launcher.seed']}.bonnMotion"
        var["omnet"]["*.bonnMotionServer.traceFile"] = QString(bm_file_name)
        for k in [
            "*.traci.launcher.useVadereSeed",
            "*.traci.launcher.seed",
            "**.vadereScenarioPath",
        ]:
            del var["omnet"][k]

    return opp_config, ParameterVariationBase().add_data_points(par_var), seed_m


def create_variation_with_vadere(seed):

    opp_config, par_var = par_var_with_vadere()

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
        # seed=time_ns(),
        seed=seed
        # seed=0        # use for study debug/setup
    )

    par_var = seed_m.get_new_seed_variation()

    return opp_config, ParameterVariationBase().add_data_points(par_var), seed_m


def create_traces():
    seed_m = OmnetSeedManager(
        par_variations=[],
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=False,
        # seed=time_ns(),
        seed=1656000207970109272
        # seed=0        # use for study debug/setup
    )
    vadere_s, omnet_s = seed_m.get_seed_paring()
    bm_base_dir = os.path.abspath("mf_dynamic_m_single_cell_7.d")

    par_var = [
        {
            "attributesSimulation.useFixedSeed": True,
            "attributesSimulation.fixedSeed": int(s),
        }
        for s in vadere_s
    ]

    jar_command = (
        JarCommand(
            jar_file=join(
                os.environ["CROWNET_HOME"],
                "vadere/VadereSimulator/target/vadere-console.jar",
            )
        )
        .add_option("-enableassertions")
        .main_class("suq")
    )
    for s in [
        join(bm_base_dir, "mf_dyn_exp_05.scenario"),
        join(bm_base_dir, "mf_dyn_exp_10.scenario"),
        join(bm_base_dir, "mf_dyn_exp_15.scenario"),
        join(bm_base_dir, "mf_dyn_exp_20.scenario"),
        join(bm_base_dir, "mf_dyn_exp_25.scenario"),
    ]:
        out = f"{os.path.basename(s).split('.')[0]}.out"
        out_f = bm_base_dir
        setup: DictVariation = DictVariation(
            scenario_path=s,
            parameter_dict_list=par_var,
            qoi="trace.bonnMotion",
            model=jar_command,
            scenario_runs=1,
            post_changes=None,
            output_path=out_f,
            output_folder=out,
            remove_output=False,
        )
        try:
            setup.run(5)
        except Exception:
            pass

        for f in glob(os.path.join(out_f, out, "vadere_output/*_output")):
            with open(join(f, s.split("/")[-1]), "r", encoding="utf-8") as fd:
                scenario = json.load(fd)

            seed = scenario["scenario"]["attributesSimulation"]["fixedSeed"]
            bm_name = f"trace_{s.split('/')[-1]}_{seed}.bonnMotion"
            shutil.copyfile(
                src=join(f, "trace.bonnMotion"), dst=join(bm_base_dir, bm_name)
            )

    print("done")


def main_vadere():
    # parameter_variation = create_parameter_variation(time_ns())
    opp_config, parameter_variation, seed_m = create_variation_with_vadere(
        1656000207970109272
    )

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
        opp_config=opp_config,
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


def main_bonn_motion():
    # parameter_variation = create_parameter_variation(time_ns())
    opp_config, parameter_variation, seed_m = create_variation_with_bonn_motion(
        1656000207970109272
    )

    model = (
        OmnetCommand().write_container_log().omnet_tag("latest").experiment_label("out")
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
        opp_config=opp_config,
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""),  # use omnet internal mobility models
        # mobility_sim=("vadere", "latest"),  # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
        scenario_provider_class=partial(
            VariationBasedScenarioProvider, par_var=parameter_variation
        ),
    )

    trace_files = (
        parameter_variation._points.stack()
        .stack()
        .loc[:, :, "*.bonnMotionServer.traceFile"]["Parameter"]
        .reset_index(drop=True)
    )
    trace_files = trace_files.apply(lambda i: i.value).unique()
    bm_base_dir = os.path.abspath("mf_dynamic_m_single_cell_7.d/")
    extra_files = []
    for f in trace_files:
        extra_files.append((join(bm_base_dir, os.path.basename(f)), f))

    print(extra_files)
    env.copy_data(base_ini_file=ini_file, extraFiles=extra_files)

    _rnd = SeedManager.rnd_suffix()
    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out",
    )
    print("setup done")

    ts = it.default_timer()
    # par_var, data = setup.run(8)
    # par_var, data = setup.run(1)
    print(f"Study: took {(it.default_timer() - ts)/60:2.4f} minutes")


if __name__ == "__main__":
    # create_traces()
    main_bonn_motion()
