from __future__ import annotations
from functools import partial
from glob import glob
import itertools
from nis import match
from optparse import Option
import os
import re
import shutil
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CrownetRequest
from suqc.requestitem import RequestItem
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
from suqc.utils.SeedManager.SeedManager import SeedManager
from time import time_ns
import random
from omnetinireader.config_parser import OppConfigFileBase, OppConfigType, QString
from typing import List

from suqc.utils.general import get_env_name
from suqc.utils.variation_scenario_p import VariationBasedScenarioProvider
from subprocess import check_output


def create_random_positions(
    base_dir: str, number_of_runs: int = 20, seed: int | None = None
):
    seed = (
        random.Random(time_ns()).randint(-2147483648, 2147483647)
        if seed is None
        else seed
    )
    cmd = os.path.join(os.environ["CROWNET_HOME"], "scripts/rnd_stationary_positions")

    num_nodes = [10, 26, 50, 76, 100, 126, 150, 176, 200]
    args_count = [
        "--vadere",
        os.path.abspath("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario"),
        "--seed",
        str(seed),
        "--seed-type",
        "pos",
        "--number-of-runs",
        str(number_of_runs),
        "--config-name",
        "position",
        "--output-dir",
        base_dir,
    ]
    for num in num_nodes:
        check_output([cmd, *args_count, "--number-of-positions", str(num)])

    densities = [
        0.0005871200538193380,
        0.0007828267384257843,
        0.0009296067518806189,
        0.0010763867653354536,
        0.001223166778790288,
    ]
    args_density = [
        "--vadere",
        os.path.abspath("../vadere/scenarios/mf_m_dyn_const_4e20s_15x12_180.scenario"),
        "--seed",
        str(seed),
        "--seed-type",
        "pos",
        "--number-of-runs",
        str(number_of_runs),
        "--output-dir",
        base_dir,
    ]
    for d in densities:
        check_output(
            [
                cmd,
                *args_density,
                "--const-density",
                str(d),
                "--config-name",
                "density_full",
            ]
        )
        check_output(
            [
                cmd,
                *args_density,
                "--const-density",
                str(d),
                "--offset",
                "0.25",
                "0.25",
                "0.5",
                "0.5",
                "--config-name",
                "density_quarter",
            ]
        )


config_pattern = re.compile(r".*?(?P<num>\d+)_(?P<run>\d+)_(?P<seed>\d+)\.ini")


class CopyPositionCfg:
    def __init__(self, trace_base_dir: str):
        self.trace_base_dir = trace_base_dir

    def copy_files(self, env_man: CrownetEnvironmentManager, item: RequestItem):
        ini_cfg: OppConfigFileBase = OppConfigFileBase.from_path(
            ini_path=item.scenario_path,
            config=env_man._opp_config,
            cfg_type=OppConfigType.EDIT_DEL_LOCAL,
        )
        file = ini_cfg["static-position-file"]
        pos_ini: OppConfigFileBase = OppConfigFileBase.from_path(
            ini_path=os.path.join(self.trace_base_dir, file),
            config=file.replace(".ini", ""),
            cfg_type=OppConfigType.EDIT_LOCAL,
        )
        section = pos_ini.section
        ini_cfg._root.add_section(section)
        ini_cfg["extends"] = f"{ini_cfg['extends']}, {section.split(' ')[-1]}"
        ini_cfg.update_selection_hierarchy()

        for option, value in pos_ini.items():
            ini_cfg._root.set(section, option, value)
        del ini_cfg["static-position-file"]

        with open(item.scenario_path, "w") as fd:
            ini_cfg.writer(fd)


def get_traces(trace_dir: str) -> dict[str, List[dict]]:
    traces = glob(os.path.join(trace_dir, "*.ini"), recursive=False)
    sim = {}
    for t in traces:
        base_name = os.path.basename(t)
        match = config_pattern.match(base_name)
        if not match:
            raise ValueError(f"ini file not matched: {base_name}")
        data = {}
        for key, value in match.groupdict().items():
            data[key] = int(value)
        data["cfg_base"] = base_name[0 : match.regs[1][0] - 1]
        data["cfg"] = base_name.replace(".ini", "")
        data["base_name"] = base_name
        data["id"] = f"{data['cfg_base']}_{data['num']}"
        runs = sim.get(data["id"], [])
        runs.append(data)
        sim[data["id"]] = runs

    for k in sim.keys():
        runs = sim[k]
        runs.sort(key=lambda x: x["run"])
        sim[k] = runs
    return sim


def main(trace_dir: str, filter: str = "both", seed: int | None = None):
    # Enviroment setup.
    #
    reps = 20  # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("ymfPlusDistStep"),
        # "mapTypeLog", QString("all"),    # debug only
        "cellAgeTTL",
        UnitValue.s(30.0),  # -1.0 no TTL in
        "alpha",
        0.95,
        "idStreamType",
        QString("insertionOrder"),
        "stepDist",
        80.0,
    )
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf",
        "writeDensityLog",
        BoolValue.TRUE,
        "mapTypeLog",
        QString("ymf"),
        # "mapTypeLog", QString("all"),
        "cellAgeTTL",
        UnitValue.s(-1.0),
        "idStreamType",
        QString("insertionOrder"),
    )
    opp_config = "final_stationary_mf"

    def var(opp_seed, position_path):
        base = {
            "omnet": {
                "extends": "_stationary_m_base_single_cell",
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist,
                "seed-set": str(opp_seed),
                "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
                "static-position-file": position_path,
            }
        }
        return base

    def var_ymf(opp_seed, position_path):
        base = {
            "omnet": {
                "extends": "_stationary_m_base_single_cell",
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(
                    "alpha", 1.00, "stepDist", 999
                ),
                "seed-set": str(opp_seed),
                "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "700ms + uniform(0s, 50ms)",
                "static-position-file": position_path,
            }
        }
        return base

    seed_m = OmnetSeedManager(
        par_variations={},
        rep_count=reps,
        omnet_fixed=False,
        vadere_fixed=None,
        seed=time_ns() if seed is None else seed,
    )

    # choose reps number of random position setups between [0..5]  and set
    # a random seed for each.
    _, opp_seeds = seed_m.get_seed_paring()
    traces = get_traces(trace_dir)
    par_var = []
    if filter == "both":
        var_f = [var, var_ymf]
    elif filter == "ymf":
        var_f = [var_ymf]
    else:
        var_f = [var]
    for variation_name, runs in traces.items():
        if len(runs) != len(opp_seeds):
            raise ValueError(
                "stationary traces and configured repetitions do not match"
            )
        for _f in var_f:
            for run in runs:
                opp_seed = opp_seeds[run["run"]]
                par_var.append(_f(opp_seed, run["base_name"]))

    parameter_variation = ParameterVariationBase().add_data_points(par_var)

    model = (
        OmnetCommand().write_container_log().omnet_tag("latest").experiment_label("out")
    )
    model.timeout = None
    model.qoi(["all"])
    model.verbose()
    model.set_seed_manager(seed_m)  # log used creation seed

    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config=opp_config,
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""),  # use omnet internal mobility models
        # mobility_sim=("vadere", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace",
        # scenario_provider_class=partial(
        #     VariationBasedScenarioProvider, par_var=parameter_variation
        # ),
    )
    env.copy_data(base_ini_file=ini_file)

    _rnd = SeedManager.rnd_suffix()
    setup = CrownetRequest(
        env_man=env,
        parameter_variation=parameter_variation,
        model=model,
        creator=partial(opp_creator, copy_f=[CopyPositionCfg(trace_dir)]),
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out",
    )
    print("setup done")
    # par_var, data = setup.run(min(12, len(par_var)))
    # par_var, data = setup.run(1)


if __name__ == "__main__":
    # main("./mf_stationary_single_cell.d/")
    main("./mf_stationary_single_cell.d/", filter="ymf", seed=1659968091603987908)
    # create_random_positions("./mf_stationary_single_cell.d/", number_of_runs=20, seed=131313)
    # get_traces("./mf_stationary_single_cell.d/")
