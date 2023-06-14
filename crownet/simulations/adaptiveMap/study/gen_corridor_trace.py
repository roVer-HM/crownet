import os

from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
import crownetutils.vadere.bonnmotion_traces as BmTrace


def main():
    # path = "../vadere/scenarios/corridor_2x5m_d20.scenario"
    # path =  "../vadere/scenarios/corridor_2x5m_d20_5perSpawn.scenario",
    path = "../vadere/scenarios/corridor_2x5m_d20_5perSpawn_ramp_down.scenario"
    scenario = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__),
            path,
        )
    )
    scenario_name = os.path.basename(path)
    print(scenario)
    trace_dir = os.path.join(
        os.path.dirname(__file__),
        # "corridor_trace.d"
        # "corridor_trace_5perSpawn.d",
        "corridor_trace_5perSpawn_ramp_down.d",
    )
    # seed = 1677158327268100695
    seed = 1678963691444362243

    seed_mgr = OmnetSeedManager(par_variations=[], rep_count=20, seed=seed)
    paring = seed_mgr.get_seed_paring()
    BmTrace.write_seed_paring(seed, paring, trace_dir)
    BmTrace.generate_traces(
        scenario=scenario,
        # scenario_name="corridor_2x5m_d20",
        # scenario_name="corridor_2x5m_d20_5perSpawn",
        scenario_name="corridor_2x5m_d20_5perSpawn_ramp_down",
        par_var_default=dict(),
        base_output_path=trace_dir,
        keep_files=[
            "trace.bonnMotion",
            "positions.csv",
            "postvis.traj",
            "numAgents.csv",
        ],
        vadere_seeds=paring[0],
        jobs=5,
        remove_output=False,
    )


if __name__ == "__main__":
    main()
