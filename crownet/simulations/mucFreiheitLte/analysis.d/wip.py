# %%
import folium
from folium.plugins import TimestampedGeoJson, TimeSliderChoropleth, HeatMapWithTime
from matplotlib import projections
import pandas as pd
from pyproj import Proj
from crownetutils.analysis.omnetpp import PlotUtil
from crownetutils.analysis.dpmm.builder import DpmmProviders
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os
from typing import Tuple

from crownetutils.utils.misc import Project
import contextily as ctx
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.analysis.plot.app_misc import PlotAppMisc
import crownetutils.analysis.dpmm as Dmap
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.omnetpp.scave import CrownetSql
import crownetutils.utils.plot as Plot
from crownetutils.utils.logging import logger, timing, logging
from IPython.display import display


def run_data(
    run, sim, hdf_file="data.h5", **kwargs
) -> Tuple[DpmmHdfBuilder, CrownetSql]:
    if "data_root" in kwargs:
        data_root = kwargs["data_root"]
    else:
        data_root = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/{sim}/analysis.d/results/{run}"
    builder = DpmmHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)

    sql = CrownetSql(
        vec_path=f"{data_root}/vars_rep_0.vec",
        sca_path=f"{data_root}/vars_rep_0.sca",
        network="World",
    )
    return data_root, builder, sql


beacon_app = ".app[1].app"
density_map = ".app[0].app"


def subway_test():
    root, b1, sql1 = run_data(
        "subwayStationary_multiEnb_20211213-13:05:44", "mucFreiheitLte"
    )

    p1 = b1.build(override_hdf=False)
    dcd = b1.build_dcdMap()

    beacon_df = OppAnalysis.get_packet_source_distribution(sql1, app_path=".app[1].app")

    fig, ax = PlotAppMisc.plot_packet_source_distribution(beacon_df)
    ax.set_title("Beacon packet source distribution")
    fig.savefig(f"{root}/out/beacon_pkt_source_dis.pdf")


def static_test():
    logger.setLevel(logging.INFO)
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/suqc/test_002/simulation_runs/outputs/Sample_0_0/subwayStationary_multiEnb_out/"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/subwayStationary_multiEnb_20220118-12:43:11/"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/subwayStationary_multiEnb_entropy_20220120-16:52:34"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/subwayStationary_multiEnb_entropy_20220121-10:15:40/"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/roVerMeeting001_20220124-10:36:35/"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/roVerMeeting001_20220124-10:57:34/"
    r = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/mucFreiheitLte/results/roVerMeeting001_20220125-10:17:58/"
    root, b, sql = run_data("", "", data_root=r)
    provider: DpmmProviders = b.build(override_hdf=False)
    sql: OMNeT.CrownetSql = sql
    module_name = sql.OR(
        [
            f"{sql.network}.{i}[%].cellularNic.channelModel[0]"
            for i in sql.module_vectors
        ]
    )

    # map = DensityMap.create_interactive_map(
    #     sql=sql,
    #     global_p = provider.global_p,
    #     time= 50,
    #     epsg_code_base=Project.UTM_32N,
    #     epsg_code_to=Project.OpenStreetMaps
    # )
    # map.save("rover_meeting_map_001.html")

    make_interactive(sql)
    # module_name = sql.OR(
    #     [f"{sql.network}.{i}[%].cellularNic.channelModel[0]" for i in sql.module_vectors]
    # )

    # v = "servingCell:vector"
    # names = {
    #     "measuredSinrUl:vector": {
    #         "name": "mSinrUl",
    #         "dtype": float,
    #     },
    #     "measuredSinrDl:vector": {
    #         "name": "mSinrDl",
    #         "dtype": float
    #     }
    # }
    # m = sql.vec_data_pivot(sql.m_channel(), names)
    # m = m.sort_index()

    # s = sql.vector_ids_to_host(
    #     sql.m_phy(),
    #     v,
    #     name_columns=["hostId"],
    #     pull_data=True)
    # s = (
    #     s.drop(columns=["vectorId"])
    #     .rename(columns={"value": "eNB"})
    # )
    # s = s.set_index(["hostId", "time"]).sort_index()

    # df = pd.merge(m, s, on=["hostId", "time"], how="outer").sort_index()
    # for group, _df in df.groupby(level=["hostId"]):
    #     df.loc[_df.index, "eNB"] = _df["eNB"].ffill()
    # df = df.dropna()
    # df = OppAnalysis.get_measured_sinr_ul_dl(sql)
    # print(df.head())
    # todo https://pandas.pydata.org/docs/reference/api/pandas.merge_asof.html

    # df = OppAnalysis.get_packet_age(sql, "app[0].app")
    # print(df.head())
    # display(map)
    return None


@timing
def make_interactive(sql):
    pdf = sql.node_position(
        epsg_code_base=Project.UTM_32N, epsg_code_to=Project.OpenStreetMaps
    )
    pdf = pdf.to_crs(crs=Project.WSG84_lat_lon)
    pdf["lat"] = pdf["geometry"].y
    pdf["lon"] = pdf["geometry"].x
    _filter = (pdf["time"] * 10).mod(4) == 0.0

    import random

    data = []
    for g, _df in pdf[_filter].groupby(by=["time"]):
        tmp = []
        for _, row in _df.iterrows():
            tmp.append([row["lat"], row["lon"]])
            if random.random() < 0.5:
                for _ in range(10):
                    tmp.append([row["lat"], row["lon"]])
        data.append(tmp)

    map = folium.Map(
        [48.161701, 11.5862764], tiles="Stamen Toner", zoom_start=18, max_zoom=22
    )
    HeatMapWithTime(
        data,
        radius=10,
        auto_play=True,
        position="bottomright",
        min_opacity=0.9,
        max_opacity=1.0,
        # scale_radius= True,
        gradient={
            0.0: "#0000ff",
            0.2: "#cc0033",
            0.4: "#990066",
            0.6: "#660099",
            0.8: "#3300CC",
            1: "#ff0000",
        },
    ).add_to(map)

    folium.LayerControl().add_to(map)
    map.save("rover_meeting_005.html")
    print("done")
    # 69
    # print(df.head())


def per_node_delay(sql, delay_resolution=1.0, describe: bool = True):
    module_name = sql.OR(
        [f"{sql.network}.{i}[%].app[0].app" for i in sql.module_vectors]
    )

    # gdf = df[["hostId", "delay"]].groupby(by=["hostId"]).aggregate(["mean", "std"])
    gdf = df[["hostId", "delay"]].groupby(by=["hostId"]).describe()
    fig, ax = plt.subplots(1, 1, figsize=(16, 9))
    # fig, (ax_dmean, ax_dstd) = plt.subplots(2, 1, figsize=(16,9))
    args = dict()
    # _make_plot(gdf, ax_dmean, args, ('delay', 'mean'), "hosts", "mean delay [ms]" )
    # _make_plot(gdf, ax_dstd, args, ('delay', 'std'), "hosts", "std delay [ms]" )
    # ax.errorbar(
    #     gdf.index,
    #     gdf.loc[:, ('delay', 'mean')],
    #     yerr=gdf.loc[:, ('delay', 'std')],
    #     fmt="o"
    # )
    # ax.set_ylabel("Delay [ms]")
    # ax.set_xlabel("HostIds")
    # ax.set_ylim([10, 30]) #ms
    sns.boxplot(x="hostId", y="delay", hue="hostId", data=df, showfliers=True, ax=ax)
    ax.set_ylabel("Delay [ms]")
    # ax.set_ylim([10, 30]) #ms

    plt.show()


def _make_plot(gdf, ax: plt.Axes, args, value, xlbl, ylbl):
    sns.barplot(x=gdf.index, y=gdf.loc[:, value], ax=ax, **args)
    ax.set_xlabel(xlbl)
    ax.set_ylabel(ylbl)

    # global_delay(sql)


def global_delay(sql: OMNeT.CrownetSql):
    module_name = sql.OR(
        [f"{sql.network}.{i}[%].app[0].app" for i in sql.module_vectors]
    )
    df = sql.vec_data(module_name, "rcvdPkLifetime:vector", value_name="delay")
    fig, ax = PlotUtil.check_ax()
    sns.histplot(
        data=df["delay"], stat="count", fill=False, element="step", cumulative=True
    )
    ax.set_xlabel("delay [s]")
    ax.set_title("Beacon Delay over all nodes")
    plt.show()


ret = static_test()


# %%
