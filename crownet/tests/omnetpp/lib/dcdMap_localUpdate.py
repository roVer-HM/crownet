import pandas as pd
from pandas import IndexSlice as I
import os
import sys

import crownetutils.analysis.dpmm.builder as Builders
from crownetutils.analysis.dpmm.hdf.dpmm_provider import DpmmProvider
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfgCsv, MapType


def dcd_map_localUpdate(hostId, data_root):
    print(data_root)
    data_root = os.path.abspath(data_root)
    print(data_root)
    cfg = DpmmCfgCsv(
        base_dir=data_root,
        hdf_file="data.h5",
        map_type=MapType.DENSITY,
        module_vectors=["misc"],
        beacon_app_path="app[0]",
        map_app_path="app[1]",
    )
    builder = Builders.DpmmHdfBuilder.get(cfg)
    map: DpmmProvider = builder.build().map_p

    host_df = map[I[:, :, :, :, hostId], ["count", "selection"]]
    host_df = host_df[host_df["selection"] != 0].copy(deep=True)
    host_df = host_df.reset_index().drop(columns=["source"])
    host_df = host_df.pivot_table(index=["simtime"], columns=["x", "y"], values="count")
    cell_idx = [f"[{x}, {y}]" for x, y in host_df.columns]
    host_df.columns = host_df.columns.to_flat_index()
    host_df.columns = pd.Index(cell_idx)
    host_df.columns.name = None
    host_df = host_df.reset_index()
    host_df.to_csv(os.path.join(data_root, "test1.csv"))


if __name__ == "__main__":
    dcd_map_localUpdate(hostId=sys.argv[1], data_root=sys.argv[2])
