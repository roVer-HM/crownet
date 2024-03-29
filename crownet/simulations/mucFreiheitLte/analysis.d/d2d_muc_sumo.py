from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.hdf.dpmm_global_positon_provider import (
    DpmmGlobalPosition,
    DpmmGlobal,
    pos_density_from_csv,
)
from crownetutils.analysis.dpmm.hdf.dpmm_provider import DpmmProvider
from crownetutils.analysis.dpmm.hdf.dpmm_count_provider import DpmmCount

import os
import glob


def from_csv(path):
    dcd_provider = DpmmProvider(os.path.join(path, "06_map.h5"))
    # count_provider = DcdMapCountProvider(os.path.join(path, "00_map.h5"))
    map_paths = glob.glob(os.path.join(path, "dcdMap_*.csv"))
    if not dcd_provider.contains_group(dcd_provider.group):
        print("read from csv")
        dcd_provider.create_from_csv(map_paths)

    g_pos, g_density = pos_density_from_csv(
        os.path.join(path, "global.csv"), os.path.join(path, "00_map.h5")
    )


def main(path):
    # from_csv(path)
    hdf_file = os.path.join(path, "09_map.h5")
    hdf_builder = DpmmHdfBuilder(
        hdf_file,
        glob.glob(os.path.join(path, "dcdMap_*.csv")),
        os.path.join(path, "global.csv"),
    )
    # creates ALL provider if necessary (map, count, position, global)
    # use the given slices on ALL providers as necessary
    # count_map data frame is still lacy loaded. The slicers are passed
    # to the DcdMap2D
    dcd = hdf_builder.build(
        time_slice=slice(2.0, 10.0), id_slice=slice(None), x_slice=slice(800.0, 1000.0)
    )
    print(dcd.map.index.get_level_values("simtime").unique())
    print(dcd.map.index.get_level_values("x").unique())
    print(dcd.position_df.index.get_level_values("simtime").unique())
    print(dcd.count_map.head(3))

    print("done")


if __name__ == "__main__":
    main("../results/mucSumo_2_20210712-10:10:22/")
