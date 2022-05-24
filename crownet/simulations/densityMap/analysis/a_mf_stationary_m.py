from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.simulators.opp.scave import CrownetSql
import seaborn as sb
import pandas as pd

def main(run: SuqcRun, idx=0):
    sim: Simulation = run.get_sim(idx)
    delay_app0 = OppAnalysis.get_cumulative_received_packet_delay(
        sim.sql,
        sim.sql.m_app0(),
        index=None
    )
    delay_app1 = OppAnalysis.get_cumulative_received_packet_delay(
        sim.sql,
        sim.sql.m_app1(),
        index=None
    )
    delay = pd.concat([delay_app0["delay"], delay_app1["delay"]], axis=0, ignore_index=True)

    avg = OppAnalysis.get_avgServedBlocksUl(
        sim.sql,
        0
    )

    ax = sb.histplot(
        data = delay,
        cumulative= True,
        fill=False,
        stat="percent",
        element="step",

    )
    
    print(delay.head())



if __name__ == "__main__":
    run = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_2/")
    main(run=run , idx=0)