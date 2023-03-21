from ast import List
from roveranalyzer.analysis.common import (
    RunContext,
    Simulation,
    SimulationGroup,
    SuqcStudy,
)
from roveranalyzer.analysis.omnetpp import HdfExtractor, OppAnalysis


def _read_target_rate_500_kbps():
    root_dir = "/mnt/data1tb/results/s1_corridor_ramp_down/"
    # 0-20 nTable
    # *.misc[*].app[*].scheduler.neighborhoodSizeProvider = "^.^.nTable"
    study = SuqcStudy(root_dir)
    nTable_f = lambda x: x[0] < 10
    ret = study.get_failed_missing_runs(nTable_f)
    if len(ret) > 0:
        raise ValueError(f"found erroneous sim: {ret}")
    sims: List[Simulation] = [
        study.get_run_as_sim(key) for key, _ in study.get_run_items(nTable_f)
    ]
    ctx: RunContext = sims[0].run_context
    meta = {
        "map_target_rate": ctx.ini_get(
            "*.misc[*].app[1].scheduler.maxApplicationBandwidth"
        ),
        "member_estimate": ctx.ini_get(
            "*.misc[*].app[*].scheduler.neighborhoodSizeProvider",
            apply=lambda x: x.replace("^.^.", ""),
        ),
    }
    sg = SimulationGroup(
        data=sims,
        group_name=f"{meta['member_estimate']}-{meta['map_target_rate']}",
        attr=meta,
    )
    df = OppAnalysis.sg_get_sent_packet_throughput_by_app(sg, freq=25.0)

    print("hi")


if __name__ == "__main__":
    _read_target_rate_500_kbps()
