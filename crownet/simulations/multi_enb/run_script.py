#!/usr/bin/env python3
import sys, os
import io
from crownetutils.analysis.dpmm.dpmm import DpmMap
from crownetutils.analysis.hdf.provider import BaseHdfProvider, HdfSelector
from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)
from crownetutils.analysis.hdf_providers.map_error_data import (
    MapCountError,
    CellCountError,
    CellEntropyValueError,
)
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.omnetpp.scave import CrownetSql

from crownetutils.analysis.common import Simulation
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from crownetutils.utils.plot import FigureSaverPdfPages, FigureSaverSimple
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from crownetutils.utils.logging import logger
from crownetutils.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169
from crownetutils.analysis.dpmm.dpmm_sql import DpmmSql
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, DpmmCfgCsv, DpmmCfgDb, MapType
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder
from crownetutils.analysis.dpmm.imputation import (
    ArbitraryValueImputation,
    DeleteMissingImputation,
    FullRsdImputation,
    ImputationIncidentLogger,
    ImputationStream,
    OwnerPositionImputation,
)

load_matplotlib_style(STYLE_SIMPLE_169)

# todo:
def get_density_cfg(base_dir):
    density_cfg = DpmmCfgDb(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        map_db_name="global_densityMap.db",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def get_entropy_cfg(base_dir):
    density_cfg = DpmmCfgDb(
        base_dir=base_dir,
        hdf_file="entropy_data.h5",
        map_type=MapType.ENTROPY,
        map_db_name="global_entropyMap.db",
        beacon_app_path=None,
        map_app_path="app[2]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def get_cfg_list(base_dir):
    # return [get_density_cfg(base_dir), get_entropy_cfg(base_dir)]
    return [get_density_cfg(base_dir)]


class SimulationRun(BaseSimulationRunner):
    @classmethod
    def postprocessing(cls, result_dir, qoi="all", exec: bool = True):
        r = cls(
            result_dir,
            [
                "post-processing",
                "--resultdir",
                result_dir,
                "--qoi",
                qoi,
                "selected-only",
                "-vv",
                "--debug",
            ],
        )
        if exec:
            r.run()
        return r, get_density_cfg(result_dir), get_entropy_cfg(result_dir)

    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)

    def fig_out(self, cfg: DpmmCfg = None) -> str:
        """provide output directory and create all parents if needed. When cfg is provided create named subfolder"""
        if cfg is None:
            p = os.path.join(self.result_base_dir(), "fig_out")
        else:
            p = os.path.join(self.result_base_dir(), "fig_out", cfg.map_type.value)
        os.makedirs(p, exist_ok=True)
        return p

    def simple_saver(
        self, sub_dir: str = "", cfg: DpmmCfg = None, fig_type: str = ".png"
    ):
        """provide saver instance at default result directory with given fig_type. If cfg is provided
        the configuration subdir is used. Any sub_dir string provided will create additional subdirs and
        ensures that all parent folders exist."""
        p = self.fig_out(cfg)
        if sub_dir != "":
            p = os.path.join(p, sub_dir)
            os.makedirs(p, exist_ok=True)
        return FigureSaverSimple(override_base_path=p, figure_type=fig_type)

    def get_builder(self, cfg: DpmmCfg):

        sim: Simulation = Simulation.from_dpmm_cfg(cfg)
        sim.sql.append_index_if_missing()

        b: DpmmHdfBuilder = DpmmHdfBuilder.get(cfg, override_hdf=False)
        b.only_selected_cells(self.ns.get("hdf_cell_selection_mode", True))
        rsd_origin_position = sim.sql.get_resource_sharing_domains(
            apply_offset=False, bottom_left_origin=True
        )
        l = ImputationIncidentLogger(io.StringIO())
        if cfg.is_count_map():
            stream = ImputationStream()
            stream.append(ArbitraryValueImputation(fill_value=0).set_incident_logger(l))
            # stream.append(DeleteMissingImputation())
            stream.append(
                FullRsdImputation(
                    rsd_origin_position=rsd_origin_position
                ).set_incident_logger(l)
            )
            stream.append(OwnerPositionImputation().set_incident_logger(l))
            b.set_imputation_strategy(stream)
        else:
            stream = ImputationStream()
            stream.append(DeleteMissingImputation().set_incident_logger(l))
            stream.append(
                FullRsdImputation(
                    rsd_origin_position=rsd_origin_position
                ).set_incident_logger(l)
            )
            stream.append(OwnerPositionImputation().set_incident_logger(l))
            b.set_imputation_strategy(stream)

        return b, l

    # todo: add pre and post processing to apply for each simulation run.
    # Post processing might inlcude result striping or aggreagtion.
    # add as many methods as needed and add the process_as annotation.
    # Use `prio` to ensure exeuction order (Bigger number higher prio)

    # @process_as({"prio": 20, "type": "post"})
    # def foo(self):
    # pass

    # @process_as({"prio": 10, "type": "pre"})
    # def bar(self):
    # pass

    @process_as({"prio": 1000, "type": "post"})
    def build_sql_index(self):
        sim = Simulation.from_dpmm_cfg(get_density_cfg(self.result_base_dir()))
        sim.sql.append_index_if_missing()
        sim.sql.debug_load_host_id_map_from_data()

    @process_as({"prio": 999, "type": "post"})
    def create_position_hdf(self) -> NodePositionWithRsdHdf:
        # use any config. Position data is equal
        cfg = get_density_cfg(self.result_base_dir())
        sim = Simulation.from_dpmm_cfg(cfg)
        pos = NodePositionWithRsdHdf.get_or_create(
            sim=sim, hdf_path=sim.path("position.h5"), override_existing=False
        )
        return pos

    @process_as({"prio": 996, "type": "post"})
    def build_density_map(self):

        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        builder, imputation_logger = self.get_builder(cfg)
        builder.build(self.ns.get("hdf_override", "False"))
        # add rsd provider to include rsd association to postion frame
        builder.set_rsd_association_provider(self.create_position_hdf())
        with open(
            cfg.path(f"impuation_{cfg.map_type}.log"), "w", encoding="utf-8"
        ) as fd:
            buf: io.StringIO = imputation_logger.writer
            buf.seek(0)
            fd.write(buf.getvalue())

    @process_as({"prio": 995, "type": "post"})
    def build_entropy_map(self):

        cfg: DpmmCfg = get_entropy_cfg(self.result_base_dir())
        builder, imputation_logger = self.get_builder(cfg)
        builder.build(self.ns.get("hdf_override", "False"))
        # add rsd provider to include rsd association to postion frame
        builder.set_rsd_association_provider(self.create_position_hdf())
        with open(
            cfg.path(f"impuation_{cfg.map_type}.log"), "w", encoding="utf-8"
        ) as fd:
            buf: io.StringIO = imputation_logger.writer
            buf.seek(0)
            fd.write(buf.getvalue())

    @process_as({"prio": 994, "type": "post"})
    def create_node_tx_hdf(self) -> NodeTxData:
        # use any config. Position data is equal
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        d_sim = Simulation.from_dpmm_cfg(d_cfg)
        e_cfg = get_entropy_cfg(_dir)
        e_sim = Simulation.from_dpmm_cfg(e_cfg)

        # pos is equal for both configs
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        # load tx data for three applications
        ret = NodeTxData.get_or_create(
            d_sim.path("node_tx_data.h5"),
            [
                SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
                SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
                SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim),
            ],
            pos,
        )
        return ret

    @process_as({"prio": 994, "type": "post"})
    def create_node_rx_hdf(self) -> NodeRxData:
        # use any config. Position data is equal
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        d_sim = Simulation.from_dpmm_cfg(d_cfg)
        e_cfg = get_entropy_cfg(_dir)
        e_sim = Simulation.from_dpmm_cfg(e_cfg)

        # pos is equal for both configs
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        # load tx data for three applications
        ret = NodeRxData.get_or_create(
            d_sim.path("node_rx_data.h5"),
            [
                SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim),
                SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
                SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim),
            ],
            pos,
        )
        return ret

    @process_as({"prio": 979, "type": "post"})
    def append_map_cell_count_over_time(self):
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        if isinstance(d_cfg, DpmmCfgDb):
            DpmmSql(d_cfg).create_cell_count_by_host_id_over_time_if_missing()
        e_cfg = get_entropy_cfg(_dir)
        if isinstance(e_cfg, DpmmCfgDb):
            DpmmSql(e_cfg).create_cell_count_by_host_id_over_time_if_missing()

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
        )
        return hdf

    @process_as({"prio": 983, "type": "post"})
    def create_entropy_cell_count_error_hdf(self) -> CellEntropyValueError:
        cfg: DpmmCfg = get_entropy_cfg(self.result_base_dir())
        dmap = Simulation.from_dpmm_cfg(cfg).get_dcdMap()

        hdf = CellEntropyValueError.get_or_create(
            hdf_path=self.result_dir(f"entropy_cell_count_error.h5"),
            count_p=dmap.count_p,
            with_rsd=True,
        )
        return hdf

    @process_as({"prio": 980, "type": "post"})
    def append_err_measure_hdf(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg)
            OppAnalysis.append_err_measures_to_hdf(sim)

    @process_as({"prio": 590, "type": "post"})
    def plot_node_traces(self):
        # use any config. Position data is equal
        d_cfg = get_density_cfg(self.result_base_dir())
        pos: NodePositionWithRsdHdf = self.create_position_hdf()

        fig, _ = PlotEnb.plot_node_enb_association_map(
            rsd=pos,
            coord=CoordinateType.xy_no_geo_offset,
            base_cmap="tab20",
            inner_r=500.0,
        )
        figpath = os.path.join(self.fig_out(), "trace_map.png")
        logger.info(f"save {figpath}")
        fig.savefig(figpath)
        plt.close(fig)

    @process_as({"prio": 585, "type": "post"})
    def plot_serving_enb_stats(self):
        cfg = get_density_cfg(self.result_base_dir())
        sql = CrownetSql.from_dpmm_cfg(cfg)
        _out = os.path.join(self.fig_out(), "enb_stats.pdf")
        with PdfPages(_out) as pdf:
            PlotEnb.plot_served_blocks_ul_all(
                self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
            )

    @process_as({"prio": 581, "type": "post"})
    def plot_density_map_count_stats(self):
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        dmap = Simulation.from_dpmm_cfg(cfg).get_dcdMap()
        rsd_list = dmap._map_p.select(key="rsd_id")

        saver = self.simple_saver(sub_dir="test", cfg=cfg, fig_type=".png")
        saver_rsd = self.simple_saver("test/dMap_count_rsd", cfg=cfg, fig_type=".png")
        map_err = self.create_density_map_count_error_hdf()
        PlotDpmMap.plot_map_count_diff(
            data=map_err, savefig=saver.with_name("dMap_count.png")
        )
        # by rsd local and total
        PlotDpmMap.plot_map_count_diff_by_rsd(
            data=map_err, rsd_list=rsd_list, saver=saver_rsd.with_name("dMap_count.png")
        )

    @process_as({"prio": 580, "type": "post"})
    def plot_density_map_count_and_error_stats(self):
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        sim: Simulation = Simulation.from_dpmm_cfg(cfg)
        saver = self.simple_saver(cfg=cfg, fig_type=".png")
        saver_rsd = self.simple_saver("dMap_count_rsd", cfg=cfg, fig_type=".png")

        # Map count
        dmap: DpmMap = sim.get_dcdMap()
        dmap.plot_map_count_diff(savefig=saver.with_name("dMap_count.png"))
        dmap.plot_map_count_diff_by_rsd(
            saver=saver_rsd.with_name("dMap_count.png")
        )  # will add suffix for rsd_id

        # Cell error
        msce = dmap.cell_count_measure(columns=["cell_mse"])
        # msce time series
        PlotDpmMap.plot_msce_ts(msce, savefig=saver.with_name("dMap_msce_ts.png"))
        # msce ecdf
        PlotDpmMap.plot_msce_ecdf(
            msce["cell_mse"], savefig=saver.with_name("dMap_msce_ecdf.png")
        )

    @process_as({"prio": 580, "type": "post"})
    def plot_neighborhood_table_state(self):
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        sim: Simulation = Simulation.from_dpmm_cfg(cfg)
        saver = self.simple_saver(fig_type=".png")
        saver_rsd = self.simple_saver(sub_dir="nt_map_node_rsd", fig_type=".png")

        PlotAppMisc.plot_nt_map_comparison(sim, saver=saver)

        pos = self.create_position_hdf()
        PlotAppMisc.plot_nt_map_comparision_by_rsd(sim, pos=pos, saver=saver_rsd)

    @process_as({"prio": 575, "type": "post"})
    def plot_application_tx_stats(self):
        tx: NodeTxData = self.create_node_tx_hdf()
        for app in tx.apps:
            saver = self.simple_saver(f"{app.name}", fig_type=".png")
            PlotAppTxInterval.plot_txinterval_from_hdf_all(
                data=tx, app_name=app.name, data_root=None, saver=saver
            )

    @process_as({"prio": 570, "type": "post"})
    def plot_application_rx_stats(self):
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())  # any cfg will work
        sim = Simulation.from_dpmm_cfg(cfg, label="")
        saver = self.simple_saver(fig_type=".png")

        rx = self.create_node_rx_hdf()
        for app in rx.apps:
            selector = HdfSelector(
                hdf=rx.rcvd_data(app),
                group=app.group_by_app("rcvd_stats"),
                columns=["delay", "jitter"],
                where=None,
            )
            PlotAppMisc.plot_application_delay_jitter(
                sim, hdf_selector=selector, saver=saver.with_suffix(f"_{app.name}")
            )

    @process_as({"prio": 565, "type": "post"})
    def plot_application_tx_throughput(self):
        cfg: DpmmCfg = get_density_cfg(self.result_base_dir())
        saver = self.simple_saver(fig_type=".png")
        rsd_ids = CrownetSql.from_dpmm_cfg(cfg).get_resource_sharing_domains(
            ids_only=False
        )["rsd_id"]
        tx_data: NodeTxData = self.create_node_tx_hdf()
        PlotAppTxInterval.plot_app_tx_throughput(
            hdf=tx_data,
            target_rates_in_Bps=tx_data.get_target_rates(
                bps_to_multiplier=1 / 8
            ),  # need rate in Bps! not bps
            rsd_ids=list(rsd_ids.values),
            bin_size=10.0,
            saver=saver,
        )

    @process_as({"prio": 560, "type": "post"})
    def plot_application_tx_time_hist_ecdf(self):
        saver = self.simple_saver(fig_type=".png")
        tx_data: NodeTxData = self.create_node_tx_hdf()
        PlotAppTxInterval.plot_application_tx_time_hist_ecdf(
            tx_data=tx_data, saver=saver
        )

    @process_as({"prio": 555, "type": "post"})
    def plot_application_debug_matrix(self):
        saver = self.simple_saver(sub_dir="dbg", fig_type=".png")
        node_rx: NodeRxData = self.create_node_rx_hdf()
        node_tx: NodeTxData = self.create_node_tx_hdf()
        pos: NodePositionWithRsdHdf = self.create_position_hdf()
        d_sim: Simulation = Simulation.from_dpmm_cfg(
            get_density_cfg(self.result_base_dir())
        )
        e_sim: Simulation = Simulation.from_dpmm_cfg(
            get_entropy_cfg(self.result_base_dir())
        )

        apps = [
            SqlAppProxy("d_map", d_sim.dpmm_cfg.m_map, d_sim).add_callback(
                cb_name="map_size",
                func=DpmmSql(d_sim.dpmm_cfg).get_cell_count_by_host_id_over_time,
            ),
            SqlAppProxy("d_beacon", d_sim.dpmm_cfg.m_beacon, d_sim),
            SqlAppProxy("e_map", e_sim.dpmm_cfg.m_map, e_sim).add_callback(
                cb_name="map_size",
                func=DpmmSql(e_sim.dpmm_cfg).get_cell_count_by_host_id_over_time,
            ),
        ]
        sim_id = ""
        rsd_filter = list(range(1, 16))
        PlotAppMisc.plot_by_app_overview(
            saver, sim_id, node_rx, node_tx, pos, d_sim, apps, rsd_filter
        )
        PlotAppMisc.plot_planed_throughput_in_rsd(
            saver, sim_id, node_tx, apps, rsd_filter
        )
        PlotAppMisc.plot_received_bursts(
            node_rx,
            figuer_title=f"Simulation_{sim_id} fixed until time 400.0 seconds",
            saver=saver.with_suffix(f"_{sim_id}"),
            where="time < 400.0",
        )

    @process_as({"prio": 100, "type": "post"})
    def repack_1(self):
        h = BaseHdfProvider(get_density_cfg(self.result_base_dir()).hdf_path())
        if h.hdf_file_exists:
            logger.info(h._hdf_path)
            h.repack_hdf(keep_old_file=False)
        else:
            logger.warn(f"file does not exist. No repacking...")

    @process_as(
        {"prio": 100, "type": "post"}
    )  # todo mark some post processing task parallel?
    def repack_2(self):
        h = BaseHdfProvider(get_entropy_cfg(self.result_base_dir()).hdf_path())
        if h.hdf_file_exists:
            logger.info(h._hdf_path)
            h.repack_hdf(keep_old_file=False)
        else:
            logger.warn(f"file does not exist. No repacking...")

    # @process_as({"prio": 100, "type": "post"})
    # def remove_density_map_csv(self):
    #     cfg: DpmmCfg
    #     for cfg in get_cfg_list(self.result_base_dir()):
    #         builder = DpmmHdfBuilder.get(cfg)
    #         for f in builder.map_paths:
    #             os.remove(f)


if __name__ == "__main__":

    settings = []
    # settings = [
    #     "post-processing",
    #     "--qoi",
    #     "all",
    #     "--override-hdf",
    #     "--resultdir",
    #     # "results/S1_bonn_motion_dev_20221007-13:43:08",
    #     "results/S1_bonn_motion_dev_20221010-08:51:11",
    # ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        print("hi")
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))
    runner.run()
