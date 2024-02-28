from itertools import product
import json
from functools import partial
import os
import sys
from typing import Any, List
from crownetutils.analysis.plot.rsd_grid_plots import RsdGridPlotter
import numpy as np
from crownetutils.analysis.common import RunMap, Simulation, SimulationGroup, SuqcStudy
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfgBuilder

from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.utils.plot import combine_images
from crownetutils.utils.logging import logger, timing, set_level
import logging
import timeit as it


from crownetutils.analysis.hdf_providers.node_position import (
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


@timing
def plot_rsd_overview_grids(data_root, analysis_out, seed_list, all=True):

    factory = FilteredSeedGroupFactory(seed_list)
    run_map = RunMap.load_or_create(
        create_f=partial(factory.get_sim_group, data_root=data_root),
        output_path=analysis_out,
    )

    # just any Simulation will do. It will only grab the enb color map
    if all:
        plotter = RsdGridPlotter.from_sim(sim=run_map[0][0])  # all
    else:
        plotter = RsdGridPlotter.from_sim(
            sim=run_map[0][0],
            rows=1,
            cols=5,
            rsd_filter=[6, 7, 8, 9, 10],
            single_fig_width=5.0,
        )

    parameters = [
        "orderByDistanceAscending_ttl15",
        "orderByDistanceDescending_ttl15",
        "insertionOrder_ttl15",
        "insertionOrder_ttl60",
        "orderByDistanceAscending_ttl60",
        "orderByDistanceDescending_ttl60",
        "orderByDistanceAscending_ttl3600",
        "orderByDistanceDescending_ttl3600",
        "insertionOrder_ttl3600",
    ]

    seed = 0
    plotter.plot_map_size_over_time_total(run_map)
    for p in parameters:
        logger.info(f"process {p}")
        # plotter.plot_rsd_size_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_serving_enb_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_throughput_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_packet_delay_grid(run_map, bin_size=10.0, parameter=p, seed=seed)
        # plotter.plot_burst_information_ratio_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_map_age_over_distance_grid(run_map, parameter=p, seed=seed)
        # plotter.plot_map_number_measures_over_distance_grid(
        #     run_map, parameter=p, seed=seed
        # )
        # plotter.plot_map_size_over_distance_grid(run_map, parameter=p, seed=seed)
        plotter.plot_map_size_over_time(run_map, parameter=p, seed=seed)

    combine_figures(analysis_dir=analysis_out)


@timing
def combine_figures(analysis_dir):

    order_var = [
        "orderByDistanceAscending",
        "orderByDistanceDescending",
        "insertionOrder",
    ]
    ttl = ["ttl15", "ttl60", "ttl3600"]

    figure_template = [
        "{base}/{order}/rsd_size_grid_{order}_{ttl}_0.png",
        "{base}/{order}/enb_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_throughput_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_delay_grid_{order}_{ttl}_0.png",
        "{base}/{order}/e_map_information_transfer_grid_{order}_{ttl}_0.png",
        "{base}/{order}/age_of_distance_{order}_{ttl}_0.png",
        "{base}/{order}/num_measures_over_distance_{order}_{ttl}_0.png",
        "{base}/{order}/num_cells_over_distance_{order}_{ttl}_0.png",
        "{base}/{order}/map_size_over_distance_{order}_{ttl}_0.png",
    ]

    for o1, ttl in product(order_var, ttl):
        figures = [
            f.format(base=analysis_dir, order=o1, ttl=ttl) for f in figure_template
        ]
        out = os.path.join(analysis_dir, f"combined_{o1}_{ttl}.png")
        combine_images(
            images=[{"input": figures, "output_path": out}], direction="vertical"
        )


if __name__ == "__main__":
    set_level(logging.DEBUG)
    logger.info("info")
    logger.info("debug")
    plot_rsd_overview_grids(
        data_root="/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/",
        analysis_out="/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/seed5only_one_row_only",
        seed_list=[5],
        all=False,
    )
    # plot_rsd_overview_grids(
    #     data_root="/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/",
    #     analysis_out="/mnt/data1tb/results/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/seed0only",
    #     seed_list=[1]
    # )
