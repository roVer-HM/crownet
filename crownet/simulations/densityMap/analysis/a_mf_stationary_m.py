





from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun
from roveranalyzer.simulators.opp.scave import CrownetSql


def main(run: SuqcRun, idx=0):
    sim: Simulation = run.get_sim(idx)
    dmap = sim.get_dcdMap()
    fig, ax  = dmap.plot_count_diff()
    print("foo")
    


if __name__ == "__main__":
    run = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_3/")
    main(run=run , idx=0)