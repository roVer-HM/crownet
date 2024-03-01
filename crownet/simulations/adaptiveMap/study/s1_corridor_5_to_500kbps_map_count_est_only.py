"""
Simulation study S1: Corridor scenario comparing DPMM application with different
bandwidth limits, ranging form 5 to 500 kbps. In this study we only use map
based count estimates.

Each parameter variation is executed N=20 times with different seeds. 
"""
from __future__ import annotations
from copy import deepcopy
from functools import partial
import os
import timeit as it
from typing import Tuple, List
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.environment import (
    CrownetEnvironmentManager,
)
from suqc.parameter.create import opp_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CrownetRequest
from omnetinireader.config_parser import (
    ObjectValue,
    UnitValue,
    QString,
    BoolValue,
)
import crownetutils.vadere.bonnmotion_traces as BmTrace

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider


mapCfgYmfDist = ObjectValue.from_args(
    "crownet::MapCfgYmfPlusDistStep",
    "writeDensityLog",
    BoolValue.TRUE,
    "mapTypeLog",
    QString("ymfPlusDistStep"),
    # "mapTypeLog", QString("all"),
    "cellAgeTTL",
    UnitValue.s(15.0),
    "alpha",
    0.90,
    "idStreamType",
    QString("insertionOrder"),
    "stepDist",
    60.0,
    "appendRessourceSharingDomainId",
    BoolValue.TRUE,
    "logType",
    QString("sql"),
)
mapCfgYmf = ObjectValue.from_args(
    "crownet::MapCfgYmf",
    "writeDensityLog",
    BoolValue.TRUE,
    "mapTypeLog",
    QString("ymf"),
    # "mapTypeLog", QString("all"),
    "cellAgeTTL",
    UnitValue.s(15.0),
    "idStreamType",
    QString("insertionOrder"),
)
t = UnitValue.s(900.0)


def par_var_bonn_motion():
    return "final_dynamic_m_bonn_motion", [
        {
            "omnet": {
                "sim-time-limit": t,
                "*.bonnMotionServer.traceFile": "trace/trace_corridor_2x5m_d20_5perSpawn_ramp_down_SEED.bonnMotion",
                "*.misc[*].app[1].scheduler.generationInterval": "2000ms",
                "*.misc[*].app[0].scheduler.generationInterval": "500ms",
                "*.misc[*].app[*].scheduler.neighborhoodSizeProvider": QString(
                    "^.^.app[1].app"  # map based neighborhoodSizeProvider!
                ),
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.misc[*].app[*].scheduler.typename": QString(
                    "DynamicMaxBandwidthScheduler"
                ),
            }
        },
    ]


# adapting ...
#  neighborhoodSizeProvider (map, vs beacons)
# ...app[*].scheduler.neighborhoodSizeProvider = "^.^.app[1].app" vs "^.^.nTable"
#  maxBandwidth used
# ...app[0].scheduler.maxApplicationBandwidth = 50kbps  (fixed for beacon)
# ...app[1].scheduler.maxApplicationBandwidth = [...]  (interval for beacon)
#  no limit on Bandwidth (other scheduler...)


def create_variation_with_bonn_motion(seed_paring: List[Tuple[int, int]]):
    opp_config, par_var = par_var_bonn_motion()

    par_var_tmp = []
    for run in par_var:
        for target_bit_rate in [5, 10, 25, 50, 100, 150, 250, 325, 425, 500]:
            _run = deepcopy(run)  # copy
            _run["omnet"][
                "*.misc[*].app[1].scheduler.maxApplicationBandwidth"
            ] = f"{target_bit_rate}kbps"
            par_var_tmp.append(_run)

    par_var = []
    for _var in par_var_tmp:
        for opp_seed, trace_seed in seed_paring:
            var = deepcopy(_var)
            var = BmTrace.update_trace_config(var, opp_seed, trace_seed)
            par_var.append(var)

    return opp_config, ParameterVariationBase().add_data_points(par_var)


def main_bonn_motion():
    trace_dir = os.path.abspath("corridor_trace_5perSpawn_ramp_down.d")
    # trace_dir = os.path.abspath("corridor_trace_5perSpawn.d")
    # trace_dir = os.path.abspath("corridor_trace.d")
    opp_config, parameter_variation = create_variation_with_bonn_motion(
        seed_paring=BmTrace.get_seed_paring(trace_dir)
    )
    model = (
        OmnetCommand().write_container_log().omnet_tag("latest").experiment_label("out")
    )
    model.timeout = None
    model.qoi(["all"])
    model.verbose()

    # Enviroment setup.
    #
    ini_file = os.path.abspath("../omnetpp.ini")
    # base_dir = "/mnt/data1tb/results/arc-dsa_single_cell/"
    base_dir = "/mnt/ssd_local/arc-dsa_single_cell/"
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
    # par_var, data = setup.run(8)
    # par_var, data = setup.run(1)
    print(f"Study: took {(it.default_timer() - ts)/60:2.4f} minutes")


if __name__ == "__main__":
    main_bonn_motion()
