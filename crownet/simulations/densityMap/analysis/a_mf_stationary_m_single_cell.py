from cProfile import label
import os
import itertools
from itertools import product
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
    if any (df.loc[pd.IndexSlice[:, :, "map_glb_count"], "std"] != 0):
        raise ValueError("WARNING: global number of nodes differs between merged runs")
    
    df = df.rename(columns={"p_50": "median"})
    df = df.unstack('data').swaplevel(-2, -1, axis=1)
    df = df.sort_index(axis=1)
    df = df.drop(columns=[('map_glb_count', 'std'), ('map_glb_count', 'median')])
    df = df.rename(columns={'map_mean_count':'map_count'})
    df.columns = [f"{l1}_{l2}" for l1, l2 in df.columns]
    df = df.rename(columns={'map_glb_count_mean':'map_glb_count'})
    
    g = 'map_glb_count'
    m = 'map_count_mean'
    gn = 'map_glb_count_n'
    mn = 'map_count_mean_n'
    mm = 'map_count_median'
    mmn = 'map_count_median_n'
    df[gn] = min_max_norm(df[g])
    df[mn] = min_max_norm(df[m], max=df[g].max())
    df[mmn] = min_max_norm(df[mm], max=df[g].max())
    
    
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


def main(study: SuqcRun):
    r = Rep()
    run_map = {}
    densities = [
        (0.000611583389395144, "6.1e-4", 100),
        (0.00076447923674393, "7.6e-4", 128),
        (0.0009173750840927161, "9.2e-4", 152),
        (0.001070270931441502, "1.1e-3", 176),
        (0.001223166778790288, "1.2e-3", 200)
    ]
    
    run_map.update(
        { f"ped_{run}_0": dict(rep=r(5), lbl="") for run in [10, 26, 50, 76, 100, 126, 150, 176, 200] }
    )
    run_map.update(
        { f"415x394_{c}": dict(rep=r(5), lbl="", density=d) for d, d_lbl, c in densities }
    )
    run_map.update(
        { f"207x196_{int(c/4)}": dict(rep=r(5), lbl="", density=d) for d, d_lbl, c in densities }
    )


    merged_norm_path = os.path.join(study.base_path, "merged_normalized_map_measure.h5")
    if os.path.exists(merged_norm_path):
        map = pd.DataFrame(pd.HDFStore(merged_norm_path, mode="r").get("map_measure_norm_static"))
    else:

        _runs = run_map.items()
        # _runs = [('full_9.2e-4', run_map['full_9.2e-4'])]
        data=["map_glb_count", "map_mean_count"]
        print("XXX")
        kwargs_iter = [
            {'study': s, 'scenario_lbl':run[0], 'rep_ids': run[1]["rep"], 'data':d, 'frame_consumer':c}
            for s, run, d, c in 
            zip(repeat(study), _runs, repeat(data), repeat(process_relative_err) )
        ]
        with get_context("spawn").Pool(10) as pool:
            maps = pool.starmap(kwargs_map, zip(repeat(OppAnalysis.merge_maps), kwargs_iter))
        # map = OppAnalysis.merge_maps(**kwargs_iter[0])

        for ret, map in maps:
            if not ret:
                print(map)
        
        maps = [map for ret, map in maps if ret]
        map: pd.DataFrame = pd.concat(maps, axis=0, verify_integrity=True)
        map.to_hdf(merged_norm_path, key="map_measure_norm_static", mode="a")
     

    fig, axes = plt.subplots(3,1,figsize=(16,3*9))


    for ax, lbl_filter in zip(axes, ["ped", "415", "207"]):
        ax.set_ylim(0.0, 1.1)
        ax.set_ylabel("norm. number of pedestrians")
        ax.set_xlabel("time in [s]")
        ax.set_xlim(0, 100)
        ax.xaxis.set_major_locator(MaxNLocator(10))
        ax.yaxis.set_major_locator(MaxNLocator(11))
        if lbl_filter in ["ped", "415"]:
            ax.set_title("Normalized number of pedestrians over time in Area [415x394]")
        else:
            ax.set_title("Normalized number of pedestrians over time in Area [207x196]")

        _first_run = map.index.get_level_values(0)[0]
        _glb = map.loc[pd.IndexSlice[_first_run, :, "map_glb_count_n"]]
        ax.plot(_glb.index.get_level_values("simtime"), _glb, label="ground_truth", color="black")
        for sim in map.index.get_level_values("sim").unique():
            if lbl_filter in sim:
                _df = map.loc[pd.IndexSlice[sim,:, "map_count_mean_n"]]
                ax.plot( _df.index, _df, label=sim)
        ax.legend(loc="center right")


    fig.tight_layout()
    fig.savefig(os.path.join(study.base_path, 'normalized_pedestrian_count.pdf'))
    fig.show()

if __name__ == "__main__":

    study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_1/")
    study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_2/")
    main(study)
