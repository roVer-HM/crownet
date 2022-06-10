from cProfile import label
import itertools
from multiprocessing import get_context
from statistics import mean
from itertools import repeat
from typing import List
from roveranalyzer.analysis.common import RunContext, Simulation, SuqcRun
from roveranalyzer.analysis.omnetpp import OppAnalysis
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D, percentile
from roveranalyzer.simulators.opp.scave import CrownetSql
from roveranalyzer.utils.plot import check_ax
from matplotlib.ticker import MaxNLocator
import seaborn as sb
import pandas as pd
from sklearn.metrics import mean_squared_error
import matplotlib.pyplot as plt


def min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


def main(run: SuqcRun, idx=0, remove_time=40.0):

    ret = [(*extract_count(run, idx), idx) for  idx in range(0, 9-3)]

    df = pd.concat([i[0] for i in ret], axis=0, ignore_index=True)
    
    fig, ax = check_ax()
    ax.plot("simtime", "glb_count_norm",data=df[df["run"]==0], label="ground_truth")
    for r in df["run"].unique():
        d = df[df["run"]==r]
        ax.plot("simtime", "count_norm_mean", data = d, label=f"run_{r} N={d['glb_count'].max()}")


    ax.set_ylim([0.0, 1.0])
    ax.set_ylabel("Normalized pedestrian count")
    ax.set_xlabel("time in [s]")
    ax.set_xlim([0.0, 100.0])
    ax.xaxis.set_major_locator(MaxNLocator(20))
    ax.yaxis.set_major_locator(MaxNLocator(20))
    ax.set_title("Normalized mean pedestrian count in static scenario after removal of 50% of nodes.")
    fig.legend()

    fig.show()
    print("")


def extract_count(run, idx):
    sim: Simulation = run.get_sim(idx)
    map: DcdMap2D = sim.get_dcdMap()
    print(f"read index {idx}")
    count_raw = map.count_diff()
    c = count_raw.loc[:, ["glb_count", "count_mean", "count_p_50"]].copy(deep=True)
    g = c["glb_count"]
    c["run"] = idx
    c["glb_count_norm"] = min_max_norm(c["glb_count"])
    c["glb_count_h_max"] = (g.max()*(c.loc[40.0,["glb_count_norm"]] + 0.05))[0]
    c["glb_count_norm_h_max"] = (c.loc[40.0,["glb_count_norm"]] + 0.05)[0]

    c["glb_count_h_min"] = (g.max()* (c.loc[40.0,["glb_count_norm"]] - 0.05))[0]
    c["glb_count_norm_h_min"] =  (c.loc[40.0,["glb_count_norm"]] - 0.05)[0]
    
    c["count_norm_mean"] = min_max_norm(c["count_mean"], max=g.max())
    c["count_norm_median"] = min_max_norm(c["count_p_50"], max=g.max())
    c["in_band"] = (c["count_norm_mean"] > c["glb_count_norm_h_min"]) & (c["count_norm_mean"] < c["glb_count_norm_h_max"])

    return c.reset_index(), c["in_band"].idxmin()


def process_relative_err(df: pd.DataFrame):
    if any (df.loc[pd.IndexSlice[:, :, "glb"], "std"] != 0):
        raise ValueError("WARNING: global number of nodes differs between merged runs")
    
    df = df.rename(columns={"p_50": "median"})
    df = df.unstack('data').swaplevel(-2, -1, axis=1)
    df = df.sort_index(axis=1)
    df = df.drop(columns=[('glb', 'std')])
    df.columns = [f"{l1}_{l2}" for l1, l2 in df.columns]
    
    g = 'glb_mean'
    m = 'map_mean'
    gn = 'glb_mean_n'
    mn = 'map_mean_n'
    df[mn] = min_max_norm(df[m], max=df[g].max())
    df[gn] = min_max_norm(df[g])
    df["map_err_n"] = df[mn] - df[gn]
    df.columns.name = "data"
    df = df.stack().sort_index()
    return df

class Rep:
    def __init__(self):
       self.num = 0
    
    def __call__(self, count=3 ) -> List[int]:
        ret = range(self.num, self.num+count)
        self.num += count
        return list(ret)

def kwargs_map(fn, kwargs):
    try:
        print(f"Execute {kwargs['scenario_lbl']}")
        ret = fn (**kwargs)
        print(f"Execute {kwargs['scenario_lbl']} done.")
        return (True, ret)
    except Exception as e:
        print(f"Error in {kwargs['scenario_lbl']} message: {e}")
        return (False, f"Error in {kwargs['scenario_lbl']} message: {e}")
    

if __name__ == "__main__":

    df = pd.read_csv(
        "/mnt/data1tb/results/mf_stationary_m_single_cell/simulation_runs/outputs/Sample_34_0/final_stationary_mf_out/beacons.csv",
        delimiter=";",
        skiprows=1
    )
    df = df.set_index(['table_owner', 'source_node', 'event_number'])
    x = df.loc[37, 71, :]


    study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell/")
    s = study.get_run_as_sim(3)
    s.get_dcdMap()
    sim: Simulation

    r = Rep()
    run_map = {}
    densities = [
        (0.000611583389395144, "6.1e-4"),
        (0.00076447923674393, "7.6e-4"),
        (0.0009173750840927161, "9.2e-4"),
        (0.001070270931441502, "1.1e-3"),
        (0.001223166778790288, "1.2e-3")
    ]
    run_map.update(
        { f"ped_{run}_0": dict(rep=r(), lbl="") for run in [10, 26, 50, 76, 100, 126, 150, 176, 200] }
    )
    run_map.update(
        { f"full_{d_lbl}": dict(rep=r(), lbl="", density=d) for d, d_lbl in densities }
    )
    run_map.update(
        { f"quarter_{d_lbl}": dict(rep=r(), lbl="", density=d) for d, d_lbl in densities }
    )

    # map = OppAnalysis.merge_maps(
    #     study, "ped_50_0", run_map["ped_50_0"]["rep"],
    #     data=["glb_count", "count_mean"], #["glb_count","count_mean", "sqerr_mean"],
    #     columns_rename=dict(count_mean="map", glb_count="glb"),
    #     frame_consumer=process_relative_err
    # )

    _runs = run_map.items()
    # _runs = [('full_9.2e-4', run_map['full_9.2e-4'])]
    data=["glb_count", "count_mean"]
    columns_rename=dict(count_mean="map", glb_count="glb")
    print("XXX")
    kwargs_iter = [
        {'study': s, 'scenario_lbl':run[0], 'rep_ids': run[1]["rep"], 'data':d, 'columns_rename':r, 'frame_consumer':c}
        for s, run, d, r, c in 
        zip(repeat(study), _runs, repeat(data), repeat(columns_rename), repeat(process_relative_err) )
    ]
    with get_context("spawn").Pool(6) as pool:
        maps = pool.starmap(kwargs_map, zip(repeat(OppAnalysis.merge_maps), kwargs_iter))
    # map = OppAnalysis.merge_maps(**kwargs_iter[0])

    for ret, map in maps:
        if not ret:
            print(map)
    
    maps = [map for ret, map in maps if ret]
    map = pd.concat(maps, axis=0, verify_integrity=True)
    

    fig, ax = plt.subplots(1,1,figsize=(16,9))

    ax.set_ylim(-1.0, 1.0)
    ax.set_ylabel("relative error")
    ax.set_xlabel("time in [s]")
    ax.set_xlim(0, 100)
    ax.xaxis.set_major_locator(MaxNLocator(10))
    ax.set_title("Relative error over time")

    for sim in map.index.get_level_values("sim").unique():
        if "6.1e-4" in sim:
            _df = map.loc[pd.IndexSlice[sim,:, "map_err_n"]]
            ax.plot( _df.index, _df, label=sim)
        ax.hlines(y=0, xmin=0, xmax=100, color="black")


    # mse = mean_squared_error(map.loc[:, ('glb_count', 'mean')], map.loc[:, ('map_count', 'mean')])
    # ax = map["map_err_n"].reset_index().plot('simtime')
    # ax.get_figure().show()
    ax.legend()
    fig.show()
    print("foo")
