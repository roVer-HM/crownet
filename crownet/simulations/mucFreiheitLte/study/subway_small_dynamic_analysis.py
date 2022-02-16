import folium
from folium.plugins import TimestampedGeoJson, TimeSliderChoropleth, HeatMapWithTime
from matplotlib import projections
import pandas as pd
from pyproj import Proj
from roveranalyzer.analysis.omnetpp import PlotUtil
from roveranalyzer.simulators.crownet.dcd.dcd_builder import DcdProviders
from roveranalyzer.simulators.opp.provider.hdf.IHdfProvider import BaseHdfProvider
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
from typing import Tuple

from roveranalyzer.utils import Project
import contextily as ctx
from roveranalyzer.analysis import OppAnalysis, DensityMap
import roveranalyzer.simulators.crownet.dcd as Dmap
import roveranalyzer.simulators.opp as OMNeT
import roveranalyzer.utils.plot as Plot
from roveranalyzer.utils.logging import logger, timing, logging
from IPython.display import display

from os.path import join, abspath

def run_data(data_base_path, out, parameter_id, run_id=0, hdf_file="data.h5", **kwargs) -> Tuple[str, Dmap.DcdHdfBuilder, OMNeT.CrownetSql]:
    data_root = join(data_base_path, "simulation_runs/outputs", f"Sample_{parameter_id}_{run_id}", out)
    builder = Dmap.DcdHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)
    
    sql = OMNeT.CrownetSql(
            vec_path=f"{data_root}/vars_rep_0.vec", 
            sca_path=f"{data_root}/vars_rep_0.sca", 
        network="World")
    return data_root, builder, sql

def create_plots(out_base, run):
    data_root, builder, sql = run_data(
        join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/subwayDynamic_multiEnb_compact_density_002/"),
        "subwayDynamic_multiEnb_compact_density_out",
        run,
        0,
        "data.h5"
    )
    x = builder.build_dcdMap(selection="ymfPlusDist")
    x.plot_count_diff(savefig=join(out_base, f"Sample_{run}_0_count_diff.pdf"))


def main():
    data_root, builder, sql = run_data(
        join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/subwayDynamic_multiEnb_compact_density_002/"),
        "subwayDynamic_multiEnb_compact_density_out",
        0,
        0,
        "data.h5"
    )
    beacon_apps = sql.m_app0()
    map_apps = sql.m_app1
    # pkt_loss, raw = OppAnalysis.get_received_packet_loss(sql, beacon_apps)

    pkt_loss_d = BaseHdfProvider(join(data_root, "pkt_loss.hdf"))
    # hdf_store.write_frame("pkt_loss", pkt_loss)
    # hdf_store.write_frame("pkt_loss_raw", raw)
    # with pkt_loss_d.read as ctx:
        # data =  ctx.select("pkt_loss", columns=["packet_loss_ratio"])

    x = builder.build_dcdMap(selection="ymfPlusDist")
    x.plot_count_diff(savefig=join(data_root, "count_diff.pdf"))



    


if __name__ == "__main__":
    # main()
    out = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/subwayDynamic_multiEnb_compact_density_002/")
    for i in np.arange(0, 8):
        create_plots(out, i)