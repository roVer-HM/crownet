from functools import partial
import os
from time import time_ns
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import coupled_creator
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


def main(base_path):
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
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.misc[*].app[1].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "300ms + uniform(0s, 50ms)",
                # Hack to find correct trace ... (will be removed later)
                "HACK_trace_name": "mf_1d_m_const_2x5m_d20m_iat_25.scenario",
                "*.bonnMotionServer.traceFile": "to/be/changed",
            },
        },
        {
            "omnet": {
                "sim-time-limit": t,
                "*.misc[*].app[1].app.mapCfg": mapCfgYmfDist.copy(),
                "*.misc[*].app[1].scheduler.generationInterval": "4000ms + uniform(0s, 50ms)",
                "*.misc[*].app[0].scheduler.generationInterval": "1000ms + uniform(0s, 50ms)",
                # Hack to find correct trace ... (will be removed later)
                "HACK_trace_name": "mf_1d_m_const_2x5m_d20m_iat_50.scenario",
                "*.bonnMotionServer.traceFile": "to/be/changed",
            },
        },
    ]

    base_trace_dir = os.path.join(
        os.path.dirname(__file__), f"{os.path.basename(__file__)[:-3]}.d"
    )
    seed_cfg = BmTrace.read_seeds(base_trace_dir)
    seed_dict = seed_cfg["opp_to_vadere"]

    seed_m = OmnetSeedManager(
        par_variations=par_var,
        rep_count=seed_cfg["reps"],
        omnet_fixed=False,
        vadere_fixed=False,  # set manually
        seed=seed_cfg["seed"],
    )
    par_var = seed_m.get_new_seed_variation()

    trace_files = []
    for var in par_var:
        # match variation based on omnet seed with corresponding trace seed.
        del var["vadere"]
        opp = var["omnet"]

        # check seed pairing
        opp_seed = opp["seed-set"]
        vadere_seed = int(opp["*.traci.launcher.seed"])
        if seed_dict[opp_seed] != vadere_seed:
            raise ValueError("Seed mismatch")

        bm_file_name = f"trace_{os.path.basename(opp['HACK_trace_name'])}_{opp['*.traci.launcher.seed']}.bonnMotion"
        trace_files.append(bm_file_name)
        var["omnet"]["*.bonnMotionServer.traceFile"] = QString(
            os.path.join("trace", bm_file_name)
        )
        for k in [
            "*.traci.launcher.useVadereSeed",
            "*.traci.launcher.seed",
            "HACK_trace_name",
        ]:
            del var["omnet"][k]

    parameter_variation = ParameterVariationBase().add_data_points(par_var)

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
    base_dir = os.path.abspath("/home/sschuhbaeck/data")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=get_env_name(base_dir, __file__.replace(".py", "")),
        opp_config="final_1d",
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

    extra_files = [(os.path.join(base_trace_dir, f), f) for f in trace_files]
    env.copy_data(base_ini_file=ini_file, extraFiles=extra_files)

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


def generate_traces():

    trace_output_path = os.path.join(
        os.path.dirname(__file__), f"{os.path.basename(__file__)[:-3]}.d"
    )

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
    BmTrace.write_seed_paring(seed, paring, trace_output_path)

    for scenario, scenario_name, par_var in traces:
        BmTrace.generate_traces(
            scenario=scenario,
            scenario_name=scenario_name,
            par_var_default=par_var,
            base_output_path=trace_output_path,
            keep_files=["trace.bonnMotion", "positions.csv", "postvis.traj"],
            jobs=5,
            vadere_seeds=paring[0],
            remove_output=True,
        )


if __name__ == "__main__":
    # main("./")
    generate_traces()
