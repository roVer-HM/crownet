# %%
import folium
from folium.plugins import TimestampedGeoJson, TimeSliderChoropleth, HeatMapWithTime
from matplotlib import projections
from matplotlib.backends.backend_pdf import PdfPages
import pandas as pd
from pyproj import Proj
from roveranalyzer.analysis.omnetpp import PlotUtil
from roveranalyzer.simulators.crownet.dcd.dcd_builder import DcdProviders
from roveranalyzer.simulators.crownet.dcd.dcd_map import DcdMap2D
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
from typing import Tuple

from roveranalyzer.utils import Project
import contextily as ctx
from roveranalyzer.analysis import OppAnalysis, DensityMap, HdfExtractor
import roveranalyzer.simulators.crownet.dcd as Dmap
import roveranalyzer.simulators.opp as OMNeT
import roveranalyzer.utils.plot as Plot
from roveranalyzer.utils.logging import logger, timing, logging
from IPython.display import display

from os.path import join, abspath

def run_data2(data_root, hdf_file="data.h5"):
    builder = Dmap.DcdHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)
    
    sql = OMNeT.CrownetSql(
            vec_path=f"{data_root}/vars_rep_0.vec", 
            sca_path=f"{data_root}/vars_rep_0.sca", 
        network="World")
    return data_root, builder, sql


def run_data(data_base_path, out, parameter_id, run_id=0, hdf_file="data.h5", **kwargs) -> Tuple[str, Dmap.DcdHdfBuilder, OMNeT.CrownetSql]:
    if "data_root" in kwargs:
        data_root = kwargs["data_root"]
    else:
        data_root = join(data_base_path, "simulation_runs/outputs", f"Sample_{parameter_id}_{run_id}_old", out)
    builder = Dmap.DcdHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)
    
    sql = OMNeT.CrownetSql(
            vec_path=f"{data_root}/vars_rep_0.vec", 
            sca_path=f"{data_root}/vars_rep_0.sca", 
        network="World")
    return data_root, builder, sql

def create_plots(data_root: str, builder: Dmap.DcdHdfBuilder, sql: OMNeT.CrownetSql, selection:str = "yml"):

    dmap = builder.build_dcdMap(selection=selection)
    with PdfPages(join(data_root, "common_output.pdf")) as pdf:
        dmap.plot_count_diff(savefig=pdf)

        tmin, tmax = builder.count_p.get_time_interval()
        time = (tmax - tmin) / 4
        intervals = [slice(time * i, time * i + time) for i in range(4)]
        for _slice in intervals:
            dmap.plot_error_histogram(time_slice=_slice, savefig=pdf)  

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

def beacon_loss(data_root: str, builder: Dmap.DcdHdfBuilder, sql: OMNeT.CrownetSql):
    return OppAnalysis.get_received_packet_loss(sql, sql.m_app0())


if __name__ == "__main__":
    data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/", 
    "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_ymf/subwayDynamic_multiEnb_compact_density_out")
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/", 
    # "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_old/subwayDynamic_multiEnb_compact_density_out")
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/", 
    # "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_Sample_0_0_ymfDistTTL/subwayDynamic_multiEnb_compact_density_out")
    data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/", 
    "circle/simulation_runs/outputs/Sample_0_0/subwayDynamic_multiEnb_compact_density_out")

    data_root, builder, sql = run_data2(
        data_root,
        "data.h5"
    )
    create_plots(data_root, builder, sql, selection="ymfPlusDist")
    HdfExtractor.extract_packet_loss(join(data_root, "packet_loss.h5"), "beacon", sql, app=sql.m_app0())
    HdfExtractor.extract_trajectories(join(data_root, "trajectories.h5"), sql)

    # i  = pd.IndexSlice
    # cells = builder.map_p.geo(Project.OpenStreetMaps)[i[:, 225., 250., :, 13817 ]]
    # m, cells, nodes = main(data_root, builder, sql)
    # with BaseHdfProvider(join(data_root, "pkt_loss.hdf")).read as ctx:
    #     loss = ctx.select("pkt_loss_raw", where="hostId == 13817")
    #     loss = loss.reset_index().set_index(["time", "srcHostId", "eventNumber"]).sort_index()
    # print("done")
    # out = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/subwayDynamic_multiEnb_compact_density_002/")
    # for i in np.arange(0, 8):
    #     create_plots(out, i)
# %%
