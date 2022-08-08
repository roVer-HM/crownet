from __future__ import annotations
from copy import deepcopy
from functools import partial
import os
import timeit as it
from time import time_ns
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import coupled_creator, opp_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider
import roveranalyzer.simulators.vadere.bonnmotion_traces as BmTrace
import roveranalyzer.simulators.vadere.vadere_variation_helper as v


s_5_20_const = QString("vadere/scenarios/mf_1d_m_const_2x5m_d20m.scenario")
t = UnitValue.s(5000.0)

source_top_left = 102
source_top_right = 103
source_bottom_left = 100
source_bottom_right = 101


def main(trace_dir: str):
    reps = 20  # seed-set
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
        0.95,
        "idStreamType",
        QString("insertionOrder"),
        "stepDist",
        80.0,
    )

    par_var_tmp = [
        {
            "omnet": {
                "sim-time-limit": t,
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.misc[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                "*.bonnMotionServer.traceFile": "trace/trace_mf_1d_m_const_2x5m_d20m_iat_25_SEED.bonnMotion",
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
                "*.bonnMotionServer.traceFile": "trace/trace_mf_1d_m_const_2x5m_d20m_iat_50_SEED.bonnMotion",
            },
        },
    ]

    seed_paring = BmTrace.get_seed_paring(trace_dir)
    par_var = []
    for _var in par_var_tmp:
        for opp_seed, trace_seed in seed_paring:
            var = deepcopy(_var)
            var = BmTrace.update_trace_config(var, opp_seed, trace_seed)
            par_var.append(var)

    parameter_variation = ParameterVariationBase().add_data_points(par_var)

    model = (
        OmnetCommand().write_container_log().omnet_tag("latest").experiment_label("out")
    )
    model.timeout = None
    model.qoi(["all"])
    model.verbose()

    # Enviroment setup.
    #
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_1d_bonn_motion",
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

    # copy seed setup file
    seed_file = BmTrace.seed_json_path(trace_dir)
    extra_files = [(seed_file, os.path.basename(seed_file))]
    env.copy_data(base_ini_file=ini_file, extraFiles=extra_files)

    _rnd = SeedManager.rnd_suffix()
    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=partial(opp_creator, copy_f=[BmTrace.get_copy_fn(trace_dir)]),
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out",
    )
    print("setup done")

    ts = it.default_timer()
    # par_var, data = setup.run(min(8, len(par_var)))
    # par_var, data = setup.run(1)y
    print(f"Study: took {(it.default_timer() - ts)/60:2.4f} minutes")


def generate_traces(trace_dir: str | None = None):

    if trace_dir is None:
        os.path.dirname(__file__), f"{os.path.basename(__file__)[:-3]}.d"

    # [ (scenario_path, name, par_var), ...]
    traces = [
        (
            os.path.abspath(os.path.join("..", s_5_20_const.value)),
            "mf_1d_m_const_2x5m_d20m_iat_25",
            {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 25.0
                ),
            },
        ),
        (
            os.path.abspath(os.path.join("..", s_5_20_const.value)),
            "mf_1d_m_const_2x5m_d20m_iat_50",
            {
                **v.source_max_spawn([source_top_left, source_bottom_right], num=0),
                **v.source_max_spawn([source_top_right, source_bottom_left], num=-1),
                **v.source_dist_par(
                    [source_top_right, source_bottom_left], "updateFrequency", 50.0
                ),
            },
        ),
    ]

    # must be same for all scenarios used.
    seed = time_ns()
    seed_mgr = OmnetSeedManager(par_variations=[], rep_count=20, seed=seed)
    paring = seed_mgr.get_seed_paring()
    BmTrace.write_seed_paring(seed, paring, trace_dir)

    for scenario, scenario_name, par_var in traces:
        BmTrace.generate_traces(
            scenario=scenario,
            scenario_name=scenario_name,
            par_var_default=par_var,
            base_output_path=trace_dir,
            keep_files=["trace.bonnMotion", "positions.csv", "postvis.traj"],
            jobs=5,
            vadere_seeds=paring[0],
            remove_output=True,
        )


if __name__ == "__main__":
    main("traces_mf_1d_bm.d")
    # generate_traces("traces_mf_1d_bm.d")
