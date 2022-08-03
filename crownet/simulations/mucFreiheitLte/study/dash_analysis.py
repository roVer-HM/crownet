# %%
from importlib import import_module
import sys
from itertools import groupby
from posixpath import basename, dirname
from re import A
from string import ascii_letters
from typing import Dict, List
import folium
from folium.plugins import TimestampedGeoJson, TimeSliderChoropleth, HeatMapWithTime
from hjson import OrderedDict
from matplotlib import projections
from matplotlib.backends.backend_pdf import PdfPages
from pyparsing import col
from roveranalyzer.simulators.opp.provider.hdf.DcDGlobalPosition import DcdGlobalDensity
import seaborn as sb
import pandas as pd
from pyproj import Proj
from roveranalyzer.analysis.dashapp import OppModel
from roveranalyzer.analysis.omnetpp import PlotUtil
from roveranalyzer.simulators.crownet.dcd.dcd_builder import DcdProviders
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
from roveranalyzer.simulators.vadere.plots import scenario
from roveranalyzer.utils.general import DataSource
from roveranalyzer.utils.plot import Style, check_ax
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
from roveranalyzer.analysis.flaskapp.application.model import (
    get_beacon_df,
    get_beacon_entry_exit,
    get_measurements,
    local_measure,
)
from roveranalyzer.analysis.common import Simulation, SuqcStudy
from roveranalyzer.analysis.flaskapp.wsgi import run_app
from omnetinireader.config_parser import ObjectValue


from os.path import join, abspath


def main(data_root: str, builder: Dmap.DcdHdfBuilder, sql: OMNeT.CrownetSql):
    beacon_apps = sql.m_app0()
    map_apps = sql.m_app1()

    i = pd.IndexSlice

    # cells = builder.position_p.geo(Project.OpenStreetMaps)[i[101., :]]
    cells = builder.map_p.geo(Project.OpenStreetMaps)[i[101.0, :, :, :, 13817]]

    pos_hdf = BaseHdfProvider(join(data_root, "postion.h5"))
    with pos_hdf.query as ctx:
        nodes = ctx.select("trajectories", where="time == 101.2")
        nodes = sql.apply_geo_position(nodes, Project.UTM_32N, Project.OpenStreetMaps)
    m = DensityMap.get_interactive(cells=cells, nodes=nodes)
    return m, cells, nodes


_suqc = [
    os.environ["HOME"],
    "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
]
_repo = [
    os.environ["HOME"],
    "repos/crownet/crownet/simulations/mucFreiheitLte/results/",
]
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
                ret.append(Simulation.from_suqc_result(abspath(p), label=prefix))
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
    for r in data:
        r.sort(key=lambda x: x.source.data_root)

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
    tbl_fig, ax = plt.subplots(figsize=(16, 9))
    PlotUtil.df_to_table(sample_tbl, ax)
    tbl_fig.suptitle("Sample Variation")

    data = {i[0].name: i for i in data}
    fig1 = OppAnalysis.diff_plot(s, data["count_diff"])
    fig2 = OppAnalysis.err_box_plot(s, data["err_box"])
    fig3 = OppAnalysis.err_hist_plot(s, data["err_hist"])

    PlotUtil.fig_to_pdf(path, [tbl_fig, fig1, fig2, fig3])


def make_pics(s: SuqcStudy):
    sim = s.get_run_as_sim(key=(6, 0))
    dcd = sim.get_dcdMap()

    fig1, ax = dcd.plot_map_count_diff(
        savefig=join(sim.data_root, "new_count_diff.pdf")
    )
    fig2, ax = dcd.plot_err_box_over_time(bin_width=10.0)
    # ax.set_ylim(-8, 5)
    fig2.savefig(join(sim.data_root, "new_err.pdf"))
    print("done")


if __name__ == "__main__":

    # _d = data
    # data_root, builder, sql = OppAnalysis.builder_from_output_folder(_d[-1]["value"])

    # _d.extend(data_dict("/mnt/data1tb/results/ymfDistDbg/simulation_runs/outputs/*/final_out",
    # suqc=True, prefix="step_err_ymf"))

    # ymf_dist_dbg_2 = SuqcRun("/mnt/data1tb/results/ymfDistDbg2")
    # ymf_dist_dbg_3 = SuqcRun("/mnt/data1tb/results/ymfDistDbg3")
    # ymf_dist_dbg_3 = SuqcRun("/mnt/data1tb/results/ymfDistDbg3")
    # ymf_dist_dbg_4 = SuqcRun("/mnt/data1tb/results/ymfDistDbg4")
    ymf_dist_dbg_5 = SuqcStudy("/mnt/data1tb/results/ymfDistDbg5")
    ymf_dist_dbg_6 = SuqcStudy("/mnt/data1tb/results/ymfDistDbg6")
    ymf_dist_dbg_7 = SuqcStudy("/mnt/data1tb/results/ymfDistDbg7")

    # dcdMap_muc01 = SuqcRun("/mnt/data1tb/results/dcdMap_muc01")
    # dcdMap_muc02 = SuqcRun("/mnt/data1tb/results/dcdMap_muc02")
    # dcdMap_muc04 = SuqcRun("/mnt/data1tb/results/dcdMap_muc04")
    # dcdMap_muc05 = SuqcRun("/mnt/data1tb/results/dcdMap_muc05")
    # dcdMap_muc07 = SuqcRun("/mnt/data1tb/results/dcdMap_muc07")
    # dcdMap_muc08 = SuqcRun("/mnt/data1tb/results/dcdMap_muc08")
    dcdMap_muc08 = SuqcStudy("/mnt/data1tb/results/dcdMap_muc08")
    dcdMap_muc09 = SuqcStudy("/mnt/data1tb/results/dcdMap_muc09")
    # data = dcdMap_muc08.get_simulation_dict(lbl_key=True)
    # data.update(dcdMap_muc09.get_simulation_dict(lbl_key=True))

    dcdMap_muc10 = SuqcStudy("/mnt/data1tb/results/dcdMap_muc10")
    dcdMap_muc11 = SuqcStudy("/mnt/data1tb/results/dcdMap_muc11")
    data = dcdMap_muc10.get_simulation_dict(lbl_key=True)
    data.update(dcdMap_muc11.get_simulation_dict(lbl_key=True))

    # study = dcdMap_muc02
    # study = dcdMap_muc04
    # study = dcdMap_muc05
    # study = dcdMap_muc10
    # study = ymf_dist_dbg_6
    # study = ymf_dist_dbg_5
    # study = ymf_dist_dbg_7
    # study = dcdMap_muc08

    # run_app(study.get_simulation_dict(lbl_key=True))
    run_app(data)

    # study ymf_dist_dbg_5
    # nid = 6746
    # cell = (110, 220)
    # sim = study.get_run_as_sim((2, 0))

    # study = dcdMap_muc04
    # nid = 13920
    # cell = (220, 230)
    # sim = study.get_run_as_sim((0, 0))
    # study = dcdMap_muc07
    # nid = 42841
    # cell = (145, 255)
    # sim = study.get_run_as_sim((0, 0))
    nid = 75997
    cell = (145, 255)
    sim = study.get_run_as_sim((0, 0))

    bs, _ = get_beacon_entry_exit(sim, nid, cell)
    x = (
        bs.set_index(["event_time", "event_id", "type"])["cumulated_count"]
        .sort_index()
        .reset_index(["event_id"], drop=True)
    )
    x = x.loc[x.index.drop_duplicates()]
    x_time_index = x.index.get_level_values("event_time")
    map = x.loc[:, "densityMap"].to_frame()
    map.columns = ["map_count"]
    nt = x.loc[:, "neighborhoodTable"].to_frame()
    nt_index_add = pd.Index(
        np.arange(np.floor(x_time_index.min()), np.ceil(x_time_index.max()), 1)
    ).difference(nt.index)
    nt_add = pd.DataFrame(columns=["cumulated_count"], index=nt_index_add)
    nt = pd.concat([nt_add, nt], axis=0).sort_index()
    nt.columns = ["nt_count"]
    nt = nt[~nt.index.duplicated()]
    nt = pd.concat([nt, map], axis=1)
    if nt.iloc[0, 0] is np.nan:
        nt.iloc[0, 0] = 0.0
    if nt.iloc[0, 1] is np.nan:
        nt.iloc[0, 1] = 0.0
    # nt = nt.ffill()

    df = local_measure(sim, nid, cell)
    df = df.reset_index(["source", "ID"])
    n = sim.builder.global_p[pd.IndexSlice[:, cell[0], cell[1]], :]
    fig, axes = plt.subplots(2, 2, figsize=(32, 18))
    axes = [a for aa in axes for a in aa]

    # pandas
    store: pd.HDFStore = pd.HDFStore(path="/tmp/data.h5", mode="r")
    df = store.select(
        key="dcd_global_density", where="x==120 AND y==100", columns=None  # all columns
    )
    store.close()

    # own interface (using context manager)
    hdf: DcdGlobalDensity = DcdGlobalDensity(hdf_path="/tmp/data.h5")
    # [[time, x, y], <columns>]
    df = hdf[pd.IndexSlice[:, 120, 100], :]

    df["count"].reset_index().plot(
        "simtime", "count", drawstyle="steps-post", marker=".", ax=axes[0]
    )
    axes[0].set_title(f"own measurements for for cell {cell}/ NT")
    bs.reset_index().plot(
        x="event_time",
        y="cumulated_count",
        drawstyle="steps-post",
        marker=".",
        ax=axes[0],
    )
    axes[1].set_title(f"neighborhood table cell {cell}")
    bs.reset_index().plot(
        "simtime", "count", drawstyle="steps-post", marker=".", ax=axes[2]
    )
    axes[2].set_title(f"ground truth cell {cell}")
    fig.show()

    print("done")
# %%
