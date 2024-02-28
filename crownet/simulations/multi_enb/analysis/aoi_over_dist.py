from itertools import product
import json
from functools import partial
import os
import sys
from typing import Any, List
from crownetutils.analysis.dpmm.dpmm_sql import DpmmSql, TimeChunks
from crownetutils.analysis.hdf.provider import BaseHdfProvider
from crownetutils.analysis.hdf_providers.map_age_over_distance import (
    MapSizeAndAgeOverDistance,
    MapMeasurementsAgeOverDistance,
)
from crownetutils.analysis.hdf_providers.map_error_data import MapCountError
from crownetutils.analysis.omnetpp import OppAnalysis
from matplotlib.ticker import MultipleLocator
import numpy as np
from crownetutils.analysis.common import RunMap, Simulation, SimulationGroup, SuqcStudy
from crownetutils.analysis.dpmm.dpmm_cfg import (
    DpmmCfgBuilder,
    DpmmCfg,
    DpmmCfgCsv,
    DpmmCfgDb,
    MapType,
)

from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.utils.plot import (
    GridPlotIter,
    PlotUtil_,
    calc_box_stats,
    combine_images,
    percentile,
    GridPlot,
)
from crownetutils.utils.logging import logger, timing, set_level
import logging
import timeit as it


import matplotlib.pyplot as plt
import pandas as pd

from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)


class FilteredSeedGroupFactory:
    def __init__(self, seeds: List[int]) -> None:
        self.seed = seeds

    def __call__(
        self, sim: Simulation, data: List[Simulation], **kwds: Any
    ) -> SimulationGroup:
        attr = kwds.get("attr", {})

        data_filtered: List[Simulation] = list(np.array(data)[self.seed])
        for s in data_filtered:
            s.dpmm_cfg = DpmmCfgBuilder.load_density_cfg(s.data_root)
        sim: Simulation = data_filtered[0]
        entropy_cfg = sim.run_context.ini_get("*.misc[*].app[2].app.mapCfg")
        ttl = int(entropy_cfg.as_dict["cellAgeTTL"].replace(".0s", "").replace("s", ""))
        order = entropy_cfg.as_dict["idStreamType"]
        sg = SimulationGroup(
            group_name=f"{order}_ttl{ttl}", data=data_filtered, attr=attr
        )
        return sg

    def get_sim_group(self, output_dir, data_root) -> RunMap:

        r = RunMap(output_dir)
        study = SuqcStudy(base_path=data_root)
        study.update_run_map(
            run_map=r, sim_per_group=20, id_offset=0, sim_group_factory=self
        )

        return r


def get_node_pos(sim: Simulation):
    return NodePositionWithRsdHdf.get_or_create(
        sim=sim, hdf_path=sim.path("position.h5"), override_existing=False
    )


if __name__ == "__main__":

    # data_root = "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4"
    # analysis_out = "/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/seed0only"
    # factory = FilteredSeedGroupFactory([1])
    # run_map = RunMap.load_or_create(
    #     create_f=partial(factory.get_sim_group, data_root=data_root),
    #     output_path=analysis_out
    # )

    data_root = "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/simulation_runs/outputs/Sample_5_0/final_multi_enb_out/"
    e_cfg = DpmmCfgBuilder().load_density_cfg(path=data_root)
    sim = Simulation.from_dpmm_cfg(e_cfg)

    set_level(logging.DEBUG)
    logger.setLevel(logging.DEBUG)

    # sim = run_map["insertionOrder_ttl3600"][0]
    # print(sim.data_root)
    # pos: NodePositionWithRsdHdf = get_node_pos(sim)
    # e_cfg: DpmmCfgDb = DpmmCfgBuilder.load_entropy_cfg(sim.data_root)
    map_sql = DpmmSql(e_cfg)
    # m = MapMeasurementsAgeOverDistance(hdf_path=sim.path("entropy_age_over_distance_2.h5"))
    # m._build(map_sql=map_sql, start_time=460.0, time_bin=10) # ! must use float time!!!
    m = MapSizeAndAgeOverDistance(hdf_path=sim.path("fooo.h5"))
    m._build(map_sql=map_sql)
    print("hi")
