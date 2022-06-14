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
import matplotlib.pyplot as plt


def min_max_norm(data, min=0, max=None):
    max = data.max() if max is None else max
    return (data - min) / (max - min)


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


def read_data(study: SuqcRun):
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
        data=["map_glb_count", "map_mean_count"]
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

    return map 

def get_convergence_time(map:pd.DataFrame, time_slice, err) -> dict:

    df = map.loc[pd.IndexSlice[:, time_slice, ("map_count_mean_n", "map_glb_count_n")]].unstack("data").copy(deep=True).droplevel(0, axis=1)
    mask = df["map_count_mean_n"] >= ( df["map_glb_count_n"] - err )
    mask = mask &  (df["map_count_mean_n"] <= ( df["map_glb_count_n"] + err ))
    df["convergence_mask"] = mask

    ret  = []
    for g, _df in df.groupby(["sim"]):
        ret.append([g, _df["convergence_mask"].idxmax()[-1]-time_slice.start , map.loc[pd.IndexSlice[g, :, "map_glb_count"], :].max()[0]])
    
    return pd.DataFrame.from_records(ret, columns=["sim", "convergence_time", "ped_count"])

def main(study: SuqcRun):

    map = read_data(study).sort_index()

    # get convergence time 
    df_convergence = get_convergence_time(map, time_slice = slice(41,None), err=0.05)
    df_convergence.to_csv(os.path.join(study.base_path, "convergence_time.csv"))

    # create figures
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

if __name__ == "__main__":

    study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_1/")
    # study = SuqcRun("/mnt/data1tb/results/mf_stationary_m_single_cell_2/")
    main(study)
