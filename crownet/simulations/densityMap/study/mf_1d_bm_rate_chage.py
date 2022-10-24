from __future__ import annotations
from copy import deepcopy
from functools import partial
import itertools
from itertools import product, repeat
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

t = UnitValue.s(1500.0)


def main(trace_dir: str, fix_trace_seed: int = -1):
    """Create simulation runs

    Args:
        trace_dir (str): _description_
        fix_tace_seed (int):  If -1 use variable seed as given by the SeedMangaer.
        If 0 >= fix_trace_seed < reps use the given trace seed for all runs.
    """
    reps = 20  # seed-set
    # = crownet::RndOffsetPolynomialEntropy{ coefficients : [0.0, 1.0,], cellSelectionPropability : 1.0, minValue: 10.0, maxValue: 30.0}"

    entropy_provider = ObjectValue.from_args(
        "crownet::RndOffsetPolynomialEntropy",
        "coefficients",
        [0.0, 1.0],
        "cellSelectionPropability",
        1.0,
        "minValue",
        10.0,
        "maxValue",
        30.0,
    )
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL",
        UnitValue.s(-1.0),
        # UnitValue.s(30.0),
        "alpha",
        0.95,
        "idStreamType",
        QString("insertionOrder"),
        "stepDist",
        80.0,
    )

    iat = ["25"]
    alg = [lambda: mapCfgYmfDist.copy("alpha", "1.0", "stepDist", 999.0)]
    map_generation_time = [100, 300, 700, 1000, 5000, 10000]
    ground_truth_rate = [
        lambda: entropy_provider.copy("coefficients", [0.0, 0.01]),
        lambda: entropy_provider.copy("coefficients", [0.0, 0.1]),
        lambda: entropy_provider.copy("coefficients", [0.0, 0.5]),
        lambda: entropy_provider.copy("coefficients", [0.0, 1.0]),
    ]

    vars = list(product(iat, alg, map_generation_time, ground_truth_rate))
    par_var_tmp = []
    for v in vars:
        par_var_tmp.append(
            {
                "omnet": {
                    "sim-time-limit": t,
                    "*.misc[*].app[0].app.mapCfg": v[1](),
                    "*.misc[*].app[0].scheduler.generationInterval": f"{v[2]}ms + uniform(0s, 50ms)",
                    "*.bonnMotionServer.traceFile": f"trace/trace_mf_1d_m_const_2x5m_d20m_iat_{v[0]}_SEED.bonnMotion",
                    "*.globalDensityMap.entropyInterval": UnitValue(1.0, "s"),
                    "*.globalDensityMap.entropyProvider": v[3](),
                },
            }
        )

    seed_paring = BmTrace.get_seed_paring(trace_dir)
    # seed_paring = BmTrace.get_seed_paring(trace_dir)
    par_var = []
    for _var in par_var_tmp:
        for opp_seed, trace_seed in seed_paring:
            if fix_trace_seed >= 0:
                # Use the same trace seed for all parameter settings.
                # This will keep the mobility part constant and only changes the opp seed.
                t_seed = seed_paring[fix_trace_seed][1]
            else:
                t_seed = trace_seed
            var = deepcopy(_var)
            var = BmTrace.update_trace_config(var, opp_seed, t_seed)
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
        opp_config="final_bonn_motion_entropy",
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


if __name__ == "__main__":
    # fixed mobility seed (index 0)
    # main("traces_mf_1d_bm.d", fix_trace_seed=0)
    # use all mobility seeds (index < 0)
    main("traces_mf_1d_bm.d", fix_trace_seed=-1)
    # generate_traces("traces_mf_1d_bm.d")
