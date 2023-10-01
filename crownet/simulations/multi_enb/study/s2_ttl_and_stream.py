"""
Simulation study S0: Corridor scenario comparing DPMM application with and 
without bandwidth limits. In addition when bandwidth limit is applied we compare
two different RSD member estimations using either the neighborhood count or the
RSD limited map count. 

Each parameter variation is executed N=20 times with different seeds. 
"""
from __future__ import annotations
from copy import deepcopy
from functools import partial
from itertools import product
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
import random
import crownetutils.vadere.bonnmotion_traces as BmTrace
from suqc.utils.SeedManager import OmnetSeedManager

from suqc.utils.SeedManager.SeedManager import SeedManager
from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider


mapCfgYmfDist_Density = ObjectValue.from_args(
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
)
mapCfgYmfDist_Entropy = ObjectValue.from_args(
    "crownet::MapCfgYmfPlusDistStep",
    "writeDensityLog",
    BoolValue.TRUE,
    "mapTypeLog",
    QString("ymfPlusDistStep"),
    "cellAgeTTL",
    UnitValue.s(15.0),  # ~!!!!
    "alpha",
    0.90,
    "idStreamType",
    QString("insertionOrder"),
    "stepDist",
    60.0,
)

t = UnitValue.s(2000.0)  # ~30min


# insertionOrder, by distance, TTL


def par_var_bonn_motion():
    sumo_traces = "trace/SEED___muc.bonnmotion.gz"
    return "final_multi_enb", [
        {
            "omnet": {
                "sim-time-limit": t,
                "*.bonnMotionServer.traceFile": sumo_traces,
            }
        },
    ]


def create_variation_with_bonn_motion(seed_paring: List[Tuple[int, int]]):
    opp_config, par_var = par_var_bonn_motion()

    par_var_tmp = []
    for run in par_var:
        _variations = product(
            [UnitValue.s(15.0), UnitValue.s(60.0), UnitValue.s(3600)],
            [
                QString("insertionOrder"),
                QString("orderBySentLastAsc_DistAsc"),
                QString("orderBySentLastAsc_DistDesc"),
            ],
        )
        for _ttl, _id_stream in _variations:
            _run = deepcopy(run)
            _entropy_cfg = mapCfgYmfDist_Entropy.copy(
                "cellAgeTTL", _ttl, "idStreamType", _id_stream
            )
            _run["omnet"]["*.pNode[*].app[2].app.mapCfg"] = _entropy_cfg
            par_var_tmp.append(_run)

    par_var = []
    for _var in par_var_tmp:
        for opp_seed, trace_seed in seed_paring:
            var = deepcopy(_var)
            var = BmTrace.update_trace_config(
                var, opp_seed, trace_seed, seed_format="03"
            )
            par_var.append(var)

    return opp_config, ParameterVariationBase().add_data_points(par_var)


def create_sumo_crownet_seeds(trace_dir):
    # omnetSeedManager.json
    seed = 1696160551962
    rnd = random.Random(x=seed)
    opp_seed = [rnd.randint(1, 225) for _ in range(20)]
    sumo_seed_id = list(range(20))
    BmTrace.write_seed_paring(
        seed,
        [sumo_seed_id, opp_seed],
        trace_dir,
        comment="used sumo seed is part of sumocfg file.",
        mobility_name="sumo_seeds",
    )


def main_bonn_motion():
    sumo_traces = (
        "../sumo/munich/muc_cleaned/output/muc_steady_state_60min_cleaned/bonnmotion"
    )
    trace_dir = os.path.abspath(sumo_traces)
    create_sumo_crownet_seeds(trace_dir)

    opp_config, parameter_variation = create_variation_with_bonn_motion(
        seed_paring=BmTrace.get_seed_paring(trace_dir, mobility_seeds="sumo_seeds")
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
    base_dir = "/mnt/data1tb/results/arc-dsa_multi_cell"
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
    extra_files = [
        (seed_file, os.path.basename(seed_file)),
        (
            "../sumo/munich/muc_cleaned/muc.net.xml.gz",
            "additional_rover_files/sumo/munich/muc_cleaned/muc.net.xml.gz",
        ),
    ]
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
