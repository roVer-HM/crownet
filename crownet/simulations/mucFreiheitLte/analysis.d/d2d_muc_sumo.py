from roveranalyzer.simulators.crownet.dcd.dcd_builder import DcdHdfBuilder
from roveranalyzer.simulators.crownet.dcd.util import owner_dist_feature, create_error_df
from roveranalyzer.simulators.opp.provider.hdf.DcDGlobalPosition import DcDGlobalPosition, \
    DcDGlobalDensity, pos_density_from_csv
from roveranalyzer.simulators.opp.provider.hdf.DcdMapProvider import DcdMapProvider
from roveranalyzer.simulators.opp.provider.hdf.CountMapProvider import CountMapProvider

import os
import glob


def create_error_df_hdf(hdf_path):
    _maps = DcdMapProvider(hdf_path)
    _global = DcDGlobalDensity(hdf_path)
    error_df = create_error_df(_maps.get_dataframe(), _global.get_dataframe())
    print("foo")


def from_csv(path):
    dcd_provider = DcdMapProvider(os.path.join(path, "06_map.h5"))
    # count_provider = CountMapProvider(os.path.join(path, "00_map.h5"))
    map_paths = glob.glob(os.path.join(path, "dcdMap_*.csv"))
    if not dcd_provider.contains_group(dcd_provider.group):
        print("read from csv")
        dcd_provider.create_from_csv(map_paths)

    g_pos, g_density = pos_density_from_csv(os.path.join(path, "global.csv"),
                                                  os.path.join(path, "00_map.h5"))


def main(path):
    # from_csv(path)
    hdf_file = os.path.join(path, "09_map.h5")
    hdf_builder = DcdHdfBuilder(
        hdf_file,
        glob.glob(os.path.join(path, "dcdMap_*.csv")),
        os.path.join(path, "global.csv")
    )
    # creates ALL provider if necessary (map, count, position, global)
    # use the given slices on ALL providers as necessary
    # count_map data frame is still lacy loaded. The slicers are passed
    # to the DcdMap2D
    dcd = hdf_builder.build_dcd_map(time_slice=slice(2.0, 10.0),
                                    id_slice=slice(None),
                                    x_slice=slice(800.0, 1000.0))
    print(dcd.map.index.get_level_values("simtime").unique())
    print(dcd.map.index.get_level_values("x").unique())
    print(dcd.location_df.index.get_level_values("simtime").unique())
    print(dcd.count_map.head(3))

    print("done")


if __name__ == "__main__":
    main("../results/mucSumo_2_20210712-10:10:22/")
