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
import crownetutils.analysis.dpmm as Dmap
from crownetutils.omnetpp.scave import CrownetSql
import crownetutils.utils.plot as Plot
from crownetutils.utils.logging import logger, timing, logging
from IPython.display import display


def run_data(
    run, sim, hdf_file="data.h5", **kwargs
) -> Tuple[DpmmHdfBuilder, OMNeT.CrownetSql]:
    if "data_root" in kwargs:
        data_root = kwargs["data_root"]
    else:
        data_root = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/{sim}/analysis.d/results/{run}"
    builder = DpmmHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)

    sql = OMNeT.CrownetSql(
        vec_path=f"{data_root}/vars_rep_0.vec",
        sca_path=f"{data_root}/vars_rep_0.sca",
        network="World",
    )
    return data_root, builder, sql


beacon_app = ".app[1].app"
density_map = ".app[0].app"


def static_test():
    logger.setLevel(logging.INFO)
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
    _i = pd.IndexSlice
    df = provider.count_p[_i[:, 235.0, 280.0, 17106]]
    print(df.head())


if __name__ == "__main__":
    static_test()
