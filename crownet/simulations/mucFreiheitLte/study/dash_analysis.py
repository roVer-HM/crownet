# %%
from importlib import import_module
from itertools import groupby
from posixpath import basename, dirname
from string import ascii_letters
from typing import Dict, List
import folium
from folium.plugins import TimestampedGeoJson, TimeSliderChoropleth, HeatMapWithTime
from hjson import OrderedDict
from matplotlib import projections
from matplotlib.backends.backend_pdf import PdfPages
import pandas as pd
from pyproj import Proj
from roveranalyzer.analysis.dashapp import OppModel
from roveranalyzer.analysis.omnetpp import PlotUtil
from roveranalyzer.simulators.crownet.dcd.dcd_builder import DcdProviders
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.simulators.vadere.plots import scenario
from roveranalyzer.utils.general import DataSource
from roveranalyzer.utils.plot import Style
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
import glob

from roveranalyzer.utils import Project
from roveranalyzer.analysis import OppAnalysis, DensityMap, HdfExtractor, DashBoard
import roveranalyzer.simulators.crownet.dcd as Dmap
import roveranalyzer.simulators.opp as OMNeT
from roveranalyzer.utils.logging import logger, timing, logging
from roveranalyzer.simulators.vadere.plots.scenario import Scenario 
from roveranalyzer.analysis.flaskapp.application.model import get_measurements
from roveranalyzer.analysis.common import Simulation
from roveranalyzer.analysis.flaskapp.wsgi import run_app
from omnetinireader.config_parser import ObjectValue


from os.path import join, abspath

def main(data_root: str, builder: Dmap.DcdHdfBuilder, sql: OMNeT.CrownetSql):
    beacon_apps = sql.m_app0()
    map_apps = sql.m_app1()

    # x: DcdMap2D = builder.build_dcdMap(selection="ymfPlusDist")
    # x.plot_count_diff(savefig=join(data_root, "count_diff.pdf"))
    # x.plot_error_histogram()

    # fig, ax = plt.subplots(nrows=5, ncols=1, figsize=(16,9*5))
    # x.plot_error_quantil_histogram(savefig=join(data_root, "error_hist_q.pdf"), ax=ax)
    i = pd.IndexSlice

    # cells = builder.position_p.geo(Project.OpenStreetMaps)[i[101., :]]
    cells = builder.map_p.geo(Project.OpenStreetMaps)[i[101., :, :, :, 13817 ]]

    pos_hdf = BaseHdfProvider(join(data_root, "postion.h5"))
    with pos_hdf.query as ctx:
        nodes = ctx.select("trajectories", where="time == 101.2")
        nodes = sql.apply_geo_position(nodes, Project.UTM_32N, Project.OpenStreetMaps)
    m = DensityMap.get_interactive(cells=cells,nodes=nodes)
    return m, cells, nodes

_suqc = [os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/"]
_repo = [os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/results/"]
_ext = ["/mnt/data1tb/results/"]


# data = [
#     Simulation(#0
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-12:46:32"),
#         label="0224-12:46:32 (B) | 130.0s | single source; mf_circle; 30 ped; ymfPlusDist=0.5; logSelected",
#     ),
#     Simulation(#1
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-13:56:46"),
#         label="0224-13:56:46 (B) | 130.0s | dual source; mf_circle; 30+(30static) ped; ymfPlusDist=0.5; logSelected",
#     ),
#     Simulation(#2
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-15:50:17"),
#         label="0224-15:50:17 (B) | 250.0s | dual source; mf_circle; 30+(30quick removed) ped; ymfPlusDist=0.5; logSelected; mI=1s",
#     ),
#     Simulation(#2
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220225-09:06:57_ymf"),
#         label="0225-09:06:57_ymf (A) | 250.0s  | dual source; mf_circle; 30+(30quick removed) ped;  ymf; logAll"
#     ),
#     Simulation(#10
#         data_root=join(*_repo, "ymf_4s_2ttl_new_20220310T134303.326544"),
#         label="ymf_4s_new_exact_dist (TTL) | 250.0s  | triple source; mf_circle; 30+30+10 ped; ymf,ttl20s; logAll; mI=4s"
#         ),
#     Simulation(#4
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220225-10:57:21_ymfPlus_4s_1"),
#         label="ymfPlus_4s_1 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
#     ),
#     Simulation(#5
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220228-09:20:17_ymfPlus_4s_2"),
#         label="ymfPlus_4s_2 (C,A) | 250.0s  | triple source; mf_circle2; 30+30+10 ped; ymfPlusDist=0.5; logAll; mI=4s !!!"),
#     Simulation(#6
#         data_root=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220228-14:56:11_ymfPlus_4s_3"),
#         label="ymfPlus_4s_3 (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s"
#         ),
#     Simulation(#7
#         data_root=join(*_repo, "ymfPlusDist_4s_1_new_20220309T160413.482974"),
#         label="ymfPlus_4s_1_new (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
#     ),
#     Simulation(#7
#         data_root=join(*_repo, "ymfPlusDist_4s_1_new_20220311T113814.135169"),
#         label="ymfPlus_4s_1_new2 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
#     ),
#     Simulation(#7
#         data_root=join(*_ext, "ymfPlusDist_4s_1_new_20220311T124331.584364"),
#         label="ymfPlus_4s_1_new3 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
#     ),
#     Simulation(#8
#         data_root=join(*_repo, "ymfPlusDist_4s_2_new_20220309T160733.541658"),
#         label="ymfPlus_4s_2_new (C,A) | 250.0s  | triple source; mf_circle2; 30+30+10 ped; ymfPlusDist=0.5; logAll; mI=4s !!!"
#         ),
#     Simulation(#9
#         data_root=join(*_repo, "ymfPlusDist_4s_3_new_20220309T160511.882464"),
#         label="ymfPlus_4s_3_new (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s"
#         ),
#     Simulation(#10
#         data_root=join(*_repo, "ymfPlusDist_4s_3_new_20220310T133600.160946"),
#         label="ymfPlus_4s_3_new_exact_dist (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s"
#         ),
#     Simulation(#7
#         data_root=join(*_ext, "ymfD_4s_long_ttl_20220311T152956.089002"),
#         label="ymfPlus_4s_1_new long with ttl",
#     ),
#     Simulation(#7
#         data_root=join(*_ext, "ymfD_4s_long_20220311T153230.245624"),
#         label="ymfPlus_4s_1_new long without ttl",
#     ),
# ]
# data = {s.label: s for s in data}

def data_dict(path, prefix="", suqc=False):
    runs = glob.glob(path)
    runs.sort()
    ret = []

    for i, p in enumerate(runs):
        if os.path.isdir(p):
            if suqc:
                ret.append(Simulation.from_suqc_result(
                    abspath(p), label=prefix
                ))
            else:
                pass
                # label = _n(i, basename(p))
                # ret.append(
                #     Simulation(abspath(p), label)
                # )
    return ret

def update_hdf(fn, sims: Dict[str, Simulation]):

    import multiprocessing as mp
    args = [s[1] for s in sims.items()]
    # fn(args[0])
    with mp.Pool(processes=8) as pool:
        ret = pool.map(fn, args)
    
    print("done")


@timing
def count_diff(path, sims: Dict[str, Simulation]):

    # collect data for each simulation
    import multiprocessing as mp
    args = [s[1] for s in sims.items()]

    # fn(args[0])
    with mp.Pool(processes=8) as pool:
        ret = np.array(pool.map(OppAnalysis.get_data_001, args))

    # sort data based on statistics
    data = ret.T.tolist()
    for  r in data:
        r.sort(key = lambda x : x.source.data_root)

    # plot data
    s = Style()
    s.font_dict = {
        "title": {"fontsize": 14},
        "xlabel": {"fontsize": 10},
        "ylabel": {"fontsize": 10},
        "legend": {"size": 14},
        "tick_size": 10,
    }
    s.create_legend = False
    
    sample_tbl = [i.get_run_description_0001() for i in args]
    sample_tbl = pd.concat(sample_tbl)
    tbl_fig, ax = plt.subplots(figsize=(16,9))
    PlotUtil.df_to_table(sample_tbl, ax)
    tbl_fig.suptitle("Sample Variation")
    
    data = {i[0].name: i for i in data } 
    fig1 = OppAnalysis.diff_plot(s, data["count_diff"])
    fig2 = OppAnalysis.err_box_plot(s, data["err_box"])
    fig3 = OppAnalysis.err_hist_plot(s, data["err_hist"])

    PlotUtil.fig_to_pdf(path, [tbl_fig, fig1, fig2, fig3])



if __name__ == "__main__":

    # _d = data
    # data_root, builder, sql = OppAnalysis.builder_from_output_folder(_d[-1]["value"])
    _d = []
    # _d = data_dict("/mnt/data1tb/results/updated_dist/*")
    # _d.extend(data_dict("/mnt/data1tb/results/transmit_wErr_dist/*"))
    # _d.extend(data_dict("/mnt/data1tb/results/transmit_dist/*"))

    _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg/simulation_runs/outputs/*/final_out",
    suqc=True, prefix="step_err_ymf"))
    
    # _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg2/simulation_runs/outputs/*/final_out",
    # suqc=True, prefix="_ymfDistStep2"))
    
    
    # _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg3/simulation_runs/outputs/*/final_out",
    # suqc=True, prefix="ymfDistStep3"))
    

    # _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg4/simulation_runs/outputs/*/final_out",
    #     suqc=True, prefix="ymfDistStep4"))

    # _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg5/simulation_runs/outputs/*/final_out",
    #   suqc=True, prefix="ymfDistStep5"))

    # _d.extend(data_dict("/mnt/data1tb/results/updated_dist_long/*"))
    # _d.sort(key= lambda x: x.label)
    _d = {s.label: s for s in _d}
    _d = OrderedDict(sorted(_d.items(), key=lambda x: x[0]))

    sim = next(iter(_d.values()))
    out_path = abspath(join(sim.data_root, "../../run_output.pdf"))
    count_diff(out_path, _d)


    # update_hdf(OppAnalysis.append_count_diff_to_hdf, _d)
    
    # run_app(_d)
    # sim = [s for s in _d if "Sample_5_0" in s.data_root ][0]
    # get_measurements(sim, 83., 2476, 115)

    print("done")
# %%
