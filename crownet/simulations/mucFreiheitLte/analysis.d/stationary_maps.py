import matplotlib as mpl
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from typing import Tuple
import contextily as ctx

from crownetutils.utils.misc import Project
import contextily as ctx
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.omnetpp.scave import CrownetSql


def run_data(run, sim, hdf_file="data.h5") -> Tuple[DpmmHdfBuilder, CrownetSql]:
    data_root = (
        f"{os.environ['HOME']}/repos/crownet/crownet/simulations/{sim}/results/{run}"
    )
    builder = DpmmHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)
    ctx.tile
    sql = CrownetSql(
        vec_path=f"{data_root}/vars_rep_0.vec",
        sca_path=f"{data_root}/vars_rep_0.sca",
        network="World",
    )
    return builder, sql


def main():
    b1, sql1 = run_data(
        "mucStationaryTest_lowerEnb_20211119-13:51:16", "mucFreiheitLte"
    )
    p = b1.build()
    dcd = b1.build_dcdMap()

    map = p.global_p.geo(to_crs=Project.OpenStreetMaps).get_dataframe()

    _i = pd.IndexSlice
    ff, g = DensityMap.get_base_grid_map(map.loc[_i[1.0], :])

    m = map.loc[_i[1.0], :]
    print("hi")
    m.explore()
    print("hi")

    def hover(event):
        if event.inaxes == g:
            cont, ind = g.contains(event)
            if len(ind) > 0:
                print("xxx")

    ff.canvas.mpl_connect("motion_notify_event", hover)

    plt.show()


if __name__ == "__main__":
    main()
