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
from roveranalyzer.analysis import OppAnalysis, DensityMap, HdfExtractor, DashBoard
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

def error_cells(data_root: str, builder: Dmap.DcdHdfBuilder, sql: OMNeT.CrownetSql):
    pass

if __name__ == "__main__":
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/",
    # "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_ymf/subwayDynamic_multiEnb_compact_density_out")
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/",
    # "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_old/subwayDynamic_multiEnb_compact_density_out")
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/",
    # "subwayDynamic_multiEnb_compact_density_002/simulation_runs/outputs/Sample_0_0_Sample_0_0_ymfDistTTL/subwayDynamic_multiEnb_compact_density_out")
    # data_root = join(os.environ['HOME'], "repos/_sim_crownet/crownet/simulations/mucFreiheitLte/suqc/",
    # "circle/simulation_runs/outputs/Sample_0_0/subwayDynamic_multiEnb_compact_density_out")

    #cicle (120s)
    # data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
    # "subwayDynamic_multiEnb_compact_density_20220224-12:46:32")

    #cicle mit leuten die vorher aufhöhen(130s) <-- not absorbing (constant 60 peds)
    # data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
    # "subwayDynamic_multiEnb_compact_density_20220224-13:56:46")

    #cicle mit leuten die vorher aufhöhen(130s) <-- *with* absorbing
    # data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
    # "subwayDynamic_multiEnb_compact_density_20220224-15:50:17")

    #cicleSTOP mit leuten die vorher aufhöhen(130s) <-- *YMF* (all cells, not only selecetd)
    # data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
    # "subwayDynamic_multiEnb_compact_density_20220225-09:06:57_ymf")

    #circle2 (400s)
    data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle2/simulation_runs/Sample__0_0/results/",
    "subwayDynamic_multiEnb_compact_density_20220224-12:17:55")

    #circleSTOP mit leuten die vorher aufhören(400s)
    # data_root = join(os.environ["HOME"], "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
    # "subwayDynamic_multiEnb_compact_density_20220225-10:57:21_ymfPlus_4s")

    data_root, builder, sql = run_data2(
        data_root,
        "data.h5"
    )
    # create_plots(data_root, builder, sql, selection="ymf")
    # create_plots(data_root, builder, sql, selection="ymfPlusDist")
    # HdfExtractor.extract_packet_loss(join(data_root, "packet_loss.h5"), "beacon", sql, app=sql.m_app0())
    # HdfExtractor.extract_trajectories(join(data_root, "trajectories.h5"), sql)

    # c = builder.count_p.get_dataframe()
    # m = builder.map_p.get_dataframe()
    # b = pd.read_csv(join(data_root, "beacons.csv"), delimiter=";")
    i =pd.IndexSlice

    # list of cells with *full* error (meaning the cell should have a count 0)
    with builder.count_p.query as ctx:
        cwerr = ctx.select(key=builder.count_p.group, where="(ID>0) & (err > 0)")
    _mask = cwerr["count"] ==  cwerr["err"]
    cwerr = cwerr[_mask]
    cwerr_t = cwerr.loc[325.]

    app = DashBoard.cell_err_app(data_root, builder, sql)
    app.run_app()

    # livetime of a single cell
    with builder.count_p.query as ctx:
        ca = ctx.select(key=builder.count_p.group, where="(x == 160.0) & (y == 140.0)")
    ca = ca.reset_index(["x", "y"]).sort_index()
    ca_0 = ca.loc[i[:, 0], :]
    t_min = int(ca_0.index.get_level_values("simtime").min())
    t_max = int(ca_0.index.get_level_values("simtime").max())
    idx_all = [(float(t), 0) for t in range(t_min, t_max+1)]
    idx_diff = pd.Index(idx_all).difference(ca_0.index)
    _add_zero = pd.DataFrame(index=idx_diff, columns=ca_0.columns).fillna(0)
    ca_0 = ca_0.append(_add_zero)

    ca_data = ca.loc[i[:, 1:], :]
    ca_most_error_ids = ca_data.groupby(by=["ID"]).sum().sort_values("err", ascending=False).head()
    #                 x        y  count    err  sqerr    owner_dist
    # ID
    # 1996  52320.0  45780.0  209.0  100.0  134.0  22900.636498
    # 2476  52320.0  45780.0  178.0   69.0   87.0  22891.182098
    # 1516  52320.0  45780.0  170.0   61.0   77.0  20927.229759
    # 2236  52320.0  45780.0  170.0   61.0   79.0  22649.881691
    # 1156  52320.0  45780.0  170.0   61.0   79.0  20520.389977

    ax = ca_0.reset_index().plot("simtime", "count", c='red', label="true count for [160.,140.]", kind="scatter")
    ax.scatter("simtime", "count", data = ca_data.loc[i[:, 1996], :].reset_index(),c='blue', marker=".", label="1996")
    ax.legend()
    # cells = builder.count_p.geo(Project.OpenStreetMaps)[i[325., :, :, 0]]
    # nodes = sql.host_position(
    #     epsg_code_base=Project.UTM_32N,
    #     epsg_code_to=Project.OpenStreetMaps,
    #     time_slice=slice(324.8),
    # )
    # m = DensityMap.get_interactive(cells, nodes)
    print("done")

# """
# c["err"].unique()
# array([ 0., -3.,  3., -1.,  1., -2.,  2.,  4., -4.])
# c["all_wrong"] = (c["count"] == c["err"]) & (c["err"] > 0)
# c3 = c.loc[pd.IndexSlice[325., :, :, 1:]]c.loc[pd.IndexSlice[325., :, :, 0]]["count"].sum()

# Cells with which are most of the time wrong! (count > 0 even if cell is empty!)
#              count   err  sqerr  owner_dist  all_wrong
# x     y
# 160.0 140.0   1551  1551   1551        1551       1551
# 185.0 130.0   1519  1519   1519        1519       1519
# 265.0 175.0   1262  1262   1262        1262       1262
# 260.0 185.0   1219  1219   1219        1219       1219


#       count   err  sqerr    owner_dist
# ID
# 736    51.0  21.0   43.0  17151.037321
# 796    52.0  22.0   28.0  13200.991064
# 856    39.0   9.0   11.0  13354.193158
# 916    40.0  10.0   28.0  17077.624291
# 976    52.0  22.0   26.0  13200.991064
# 1036   50.0  20.0   24.0  12850.795701
# 1096   43.0  13.0   29.0  16703.308976
# 1156   50.0  20.0   24.0  12850.795701
# 1216   50.0  20.0   26.0  13725.608736
# 1276   45.0  15.0   33.0  17353.766315
# 1336   45.0  15.0   29.0  17077.624291
# 1396   50.0  20.0   28.0  13507.112279
# 1456   51.0  21.0   25.0  13208.098831
# 1516   48.0  18.0   20.0  12367.016877
# 1576   50.0  20.0   26.0  13698.212819
# 1636   39.0   9.0   11.0  12475.586988
# 1696   49.0  19.0   25.0  13479.670863
# 1756   46.0  16.0   20.0  13725.608736
# 1816   48.0  18.0   22.0  13231.307033
# 1876   37.0   7.0   11.0  13574.433896
# 1936   42.0  12.0   20.0  14863.757801
# 1996   51.0  21.0   31.0  15050.891167
# 2056   52.0  22.0   30.0  14144.947951
# 2116   42.0  12.0   18.0  14221.985559
# 2176   45.0  15.0   19.0  12650.368294
# 2236   40.0  10.0   12.0  12839.811634
# 2296   48.0  18.0   22.0  12651.044370
# 2356   36.0   6.0   24.0  17031.802884
# 2416   48.0  18.0   26.0  13788.964470
# 2476   39.0   9.0   11.0  12876.356235

# f[f["err"] > 220]
#        count    err   sqerr     owner_dist
# ID
# 796   4123.0  223.0   829.0  401208.974216
# 1216  4138.0  238.0   814.0  406977.885787
# 1456  4138.0  238.0   772.0  410499.819022
# 1576  4141.0  241.0   827.0  416052.238830
# 1756  4142.0  242.0   816.0  422033.248291
# 1816  4129.0  229.0   801.0  419543.702503
# 1996  4124.0  224.0  1070.0  577881.216090
# 2416  4137.0  237.0  1037.0  531973.362155
# """

# %%
