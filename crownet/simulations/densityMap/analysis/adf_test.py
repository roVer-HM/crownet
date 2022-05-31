from cProfile import label
from encodings import utf_8
import itertools
from math import floor
import os
import statsmodels.api as sm
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MaxNLocator



def adf_test(data: pd.Series):
    from statsmodels.tsa.stattools import adfuller
    print("Dickey-Fuller Test:")
    df_test = adfuller(data, autolag="AIC")
    out = pd.Series(df_test[0:4], index=['Test Statistic','p-value','#Lags Used','Number of Observations Used'])
    out["t_min"] = data.index.min()
    out["t_max"] = data.index.max()
    for k, v in df_test[4].items():
        out[f'Critical Value ({k})'] = v
    # print(out)
    return out

def main(ns: argparse.Namespace):
    output_base_dir = os.path.dirname(ns.input)
    data = pd.read_csv(ns.input, sep=" ", skiprows=1)
    data.columns = ["time", "value"]
    data["time"] *= 0.4
    data = data.set_index(["time"])
    fig, ax = plt.subplots(1, 1, figsize=(16,9))
    ax.plot(data.index, data["value"], label="number of agents", marker=None, color="black")
    ax.set_title("Number of Agents in Simulation")
    ax.set_ylabel("Number of Agents")
    ax.set_xlabel("Time in [s]")
    ax.set_ylim(0, 20)
    ax.set_xlim(0, 10000)
    ax.xaxis.set_major_locator(MaxNLocator(20))
    
    stat = data["value"].describe()
    _stats = []
    _labels = []
    _interval = []
    _adfs = []
    print(stat)
    for t_min, t_max in itertools.product([0.4, 450], [500, 1000, 1500, 2000, 2500, 3000, 5000, 10000]):
        print(f"### using [{t_min}:{t_max}]")
        _data= data.loc[t_min:t_max]
        _stat = _data['value'].describe()
        _stat.name = f"[{floor(t_min)}:{floor(t_max)}]_stat"

        _adf =  adf_test(_data)
        _adf.name = f"[{floor(t_min)}:{floor(t_max)}]_adf"
        _interval.append((t_min, t_max))
        _labels.append(_stat.name)
        _stats.append( _stat)
        _adfs.append(_adf)
                
    
    _stats = pd.concat(_stats, axis=1)
    _adfs = pd.concat(_adfs, axis=1)


    with open(os.path.join(output_base_dir, "stats.csv"), "w", encoding="utf-8") as fd:
        _stats.T.to_csv(fd)
    with open(os.path.join(output_base_dir, "adf.csv"), "w", encoding="utf-8") as fd:
        _adfs.T.to_csv(fd)
    
    fig.savefig(os.path.join(output_base_dir, "ped_count.pdf"))

    print("done.")




if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("-i", "--input", required=True, help="Input file")

    ns = p.parse_args([
        "-i", 
        "/home/vm-sts/repos/crownet/crownet/simulations/densityMap/vadere/output/mf_1d_m_const_2x5m_d20m_2022-05-31_14-15-54.174/numAgents.csv",
        ])
    main(ns)