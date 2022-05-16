





from roveranalyzer.analysis.common import RunContext, Simulation
from roveranalyzer.simulators.opp.scave import CrownetSql


def main(ctx: RunContext):
    sim: Simulation  = Simulation.from_context(ctx)
    sql: CrownetSql =  sim.sql
    enbs = sql.vector_ids_to_host(
        sql.m_phy(), "servingCell:vector", name_columns=["hostId"], pull_data=True
    )
    enbs = enbs.drop(columns=["vectorId"]).rename(columns={"value": "eNB"})
    enbs = enbs.set_index(["hostId", "time"]).sort_index()
    print("done")
    


if __name__ == "__main__":
    main(ctx=RunContext.from_path("/mnt/data1tb/results/mf_stationary_m/simulation_runs/Sample__5_0/runContext.json"))