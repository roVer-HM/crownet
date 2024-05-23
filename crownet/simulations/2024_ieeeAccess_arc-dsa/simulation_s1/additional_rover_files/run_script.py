#!/usr/bin/env python3
import sys, os
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.dpmm_cfg import (
    DpmmCfg,
    DpmmCfgBuilder,
    DpmmCfgDb,
    MapType,
)
from crownetutils.analysis.dpmm.dpmm_sql import DpmmSql
from crownetutils.analysis.dpmm.imputation import (
    DeleteMissingArbitraryGlobalValueForImagined,
    FullRsdImputation,
    ImputationStream,
    OwnerPositionImputation,
)
from crownetutils.analysis.hdf_providers.map_error_data import (
    CellCountError,
    CellCountErrorBuilder,
    MapCountError,
)
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.omnetpp.scave import CrownetSql

import pandas as pd
import numpy as np
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
from matplotlib.backends.backend_pdf import PdfPages
from crownetutils.utils.logging import logger
from crownetutils.utils.plot import FigureSaverPdfPages, FigureSaverSimple
from crownetutils.analysis.vadere import VadereAnalysis
from crownetutils.analysis.omnetpp import HdfExtractor, OppAnalysis
from crownetutils.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from crownetutils.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169

load_matplotlib_style(STYLE_SIMPLE_169)


def _corridor_filter_target_cells(df: pd.DataFrame, *arg, **args) -> pd.DataFrame:
    # remove cells under target area
    xy = df.index.to_frame()
    mask = np.repeat(False, xy.shape[0])
    # for x in [400, 405, 410]: # remove target cells
    for x in [400, 405, 410, 0, 5, 10]:  # remove source/target cells
        y = 180.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    # for x in [0.0, 5.0, 10.0]:    # remove target cells
    for x in [0.0, 5.0, 10.0, 400, 405, 410]:  # remove source/target cells
        y = 205.0
        mask = mask | (xy["x"] == x) & (xy["y"] == y)

    ret = df[~mask].copy(deep=True)
    logger.debug(
        f"Remove source/target cells from analysis: {df.shape[0]-ret.shape[0]:,}/{df.shape[0]:,} rows"
    )
    return ret


def get_density_cfg(base_dir):
    density_cfg = DpmmCfgDb(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        map_db_name="global_densityMap.db",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc"],
    )
    return density_cfg


class SimulationRun(BaseSimulationRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    def get_cfg(self):
        return get_density_cfg(self.result_base_dir())

    def get_builder(self):

        cfg = get_density_cfg(self.result_base_dir())
        sim: Simulation = Simulation.from_dpmm_cfg(cfg)
        sim.sql.append_index_if_missing()

        b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
        b.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        rsd_origin_position = sim.sql.get_resource_sharing_domains(
            apply_offset=False, bottom_left_origin=True
        )
        stream = ImputationStream()
        # stream.append(ArbitraryValueImputation(fill_value=0.0))
        stream.append(DeleteMissingArbitraryGlobalValueForImagined(glb_fill_value=0.0))
        # stream.append(DeleteMissingImputation())
        stream.append(FullRsdImputation(rsd_origin_position=rsd_origin_position))
        stream.append(OwnerPositionImputation())
        b.set_imputation_strategy(stream)

        return b

    @process_as({"prio": 1010, "type": "post"})
    def build_sql_index(self):
        sim = Simulation.from_dpmm_cfg(get_density_cfg(self.result_base_dir()))
        sim.sql.append_index_if_missing()
        sim.sql.debug_load_host_id_map_from_data()

    @process_as({"prio": 1005, "type": "post"})
    def save_map_cfg(self):
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        b = DpmmCfgBuilder()
        b.save_in_root(d_cfg)

    @process_as({"prio": 1000, "type": "post"})
    def create_position_hdf(self) -> NodePositionWithRsdHdf:
        # use any config. Position data is equal
        cfg = get_density_cfg(self.result_base_dir())
        sim = Simulation.from_dpmm_cfg(cfg)
        pos = NodePositionWithRsdHdf.get_or_create(
            sim=sim, hdf_path=sim.path("position.h5"), override_existing=False
        )
        return pos

    @process_as({"prio": 999, "type": "post"})
    def build_hdf(self):
        builder = self.get_builder()
        builder.build(override_hdf=True, repack_on_build=True)

    @process_as({"prio": 994, "type": "post"})
    def create_node_tx_hdf(self) -> NodeTxData:
        # use any config. Position data is equal
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        d_sim = Simulation.from_dpmm_cfg(d_cfg)

        # pos is equal for both configs
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        # load tx data for three applications
        ret = NodeTxData.get_or_create(
            d_sim.path("node_tx_data.h5"),
            [
                SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
                SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
            ],
            pos,
            allow_empty_max_bandwidth=True,
        )
        return ret

    @process_as({"prio": 993, "type": "post"})
    def create_node_rx_hdf(self) -> NodeRxData:
        # use any config. Position data is equal
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        d_sim = Simulation.from_dpmm_cfg(d_cfg)

        # pos is equal for both configs
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        # load tx data for three applications
        ret = NodeRxData.get_or_create(
            d_sim.path("node_rx_data.h5"),
            [
                SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
                SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
            ],
            pos,
        )
        return ret

    @process_as({"prio": 992, "type": "post"})
    def append_map_cell_count_over_time_to_db(self):
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        if isinstance(d_cfg, DpmmCfgDb):
            sql = DpmmSql(d_cfg)
            if not sql.has_complete_dcd_map_cache():
                last_uid, last_time = sql.get_last_processed_uid_of_row_mapping_cache()
                logger.info(
                    f"create dcd_map row id cache starting from row {last_uid} from time {last_time} for db {sql.path}"
                )
                sql.create_dcd_map_row_mapping_cache(
                    chunk_size=10_000_000, initial_offset=last_uid
                )
            else:
                logger.info(f"found row id cache for density map. Nothing to do.")

    @process_as({"prio": 985, "type": "post"})
    def create_density_map_count_error_hdf(self) -> MapCountError:
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        dmap = Simulation.from_dpmm_cfg(cfg).get_dcdMap()
        map_count_error_hdf = MapCountError.get_or_create(
            hdf_path=self.result_dir("density_map_count_error.h5"),
            map_p=dmap._map_p,
            glb_pos=dmap.position_df,
            rsd_p=pos,
            with_rsd=True,
        )
        return map_count_error_hdf

    @process_as({"prio": 984, "type": "post"})
    def create_density_cell_count_error_hdf(self) -> CellCountError:
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        dmap = Simulation.from_dpmm_cfg(cfg).get_dcdMap()

        hdf = CellCountError.get_or_create(
            hdf_path=self.result_dir(f"density_cell_count_error.h5"),
            count_p=dmap.count_p,
            with_rsd=True,
            builder=CellCountErrorBuilder(fc=_corridor_filter_target_cells),
        )
        return hdf

    @process_as({"prio": 585, "type": "post"})
    def plot_serving_enb_stats(self):
        cfg = get_density_cfg(self.result_base_dir())
        sql = CrownetSql.from_dpmm_cfg(cfg)
        _out = os.path.join(self.fig_out(), "enb_stats.pdf")
        with PdfPages(_out) as pdf:
            PlotEnb.plot_served_blocks_ul_all(
                self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
            )


if __name__ == "__main__":

    settings = []

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
