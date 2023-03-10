import os

from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
import roveranalyzer.simulators.vadere.bonnmotion_traces as BmTrace


def main():
    scenario = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__),
            # "../vadere/scenarios/corridor_2x5m_d20.scenario"
            "../vadere/scenarios/corridor_2x5m_d20_5perSpawn.scenario",
        )
    )
    print(scenario)
    trace_dir = os.path.join(
        os.path.dirname(__file__),
        # "corridor_trace.d"
        "corridor_trace_5perSpawn.d",
    )
    seed = 1677158327268100695

    seed_mgr = OmnetSeedManager(par_variations=[], rep_count=20, seed=seed)
    paring = seed_mgr.get_seed_paring()
    BmTrace.write_seed_paring(seed, paring, trace_dir)
    BmTrace.generate_traces(
        scenario=scenario,
        # scenario_name="corridor_2x5m_d20",
        scenario_name="corridor_2x5m_d20_5perSpawn",
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
