#!/usr/bin/env python3
from functools import partial
import sys, os
import io
from crownetutils.analysis.hdf.provider import BaseHdfProvider, HdfSelector
from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy

import pandas as pd
import numpy as np
from crownetutils.analysis.common import Simulation
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.dockerrunner.simulationrunner import (
    BaseSimulationRunner,
    process_as,
)
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from crownetutils.utils.plot import FigureSaverPdfPages, FigureSaverSimple
from crownetutils.analysis.vadere import VadereAnalysis
from crownetutils.analysis.omnetpp import HdfExtractor, OppAnalysis
from crownetutils.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from crownetutils.utils.logging import logger
from crownetutils.utils.misc import Project
from crownetutils.utils.styles import load_matplotlib_style, STYLE_SIMPLE_169
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, MapType
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


def get_density_cfg(base_dir):
    density_cfg = DpmmCfg(
        base_dir=base_dir,
        hdf_file="density_data.h5",
        map_type=MapType.DENSITY,
        global_map_csv_name="global.csv",
        beacon_app_path="app[0]",
        map_app_path="app[1]",
        module_vectors=["misc", "pNode"],
    )
    return density_cfg


def get_entropy_cfg(base_dir):
    density_cfg = DpmmCfg(
        base_dir=base_dir,
        hdf_file="entropy_data.h5",
        map_type=MapType.ENTROPY,
        global_map_csv_name="global_entropyMap.csv",
        node_map_csv_glob="entropyMap_*.csv",
        node_map_csv_id_regex=r"entropyMap_(?P<node>\d+)\.csv",
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

    @process_as({"prio": 999, "type": "post"})
    def build_hdf(self):

        cfg: DpmmCfg
        for idx, cfg in enumerate(get_cfg_list(self.result_base_dir())):
            builder, imputation_logger = self.get_builder(cfg)
            builder.build(self.ns.get("hdf_override", "False"))
            with open(cfg.path(f"impuation_{idx}.log"), "w", encoding="utf-8") as fd:
                buf: io.StringIO = imputation_logger.writer
                buf.seek(0)
                fd.write(buf.getvalue())

    @process_as({"prio": 995, "type": "post"})
    def create_position_hdf(self):
        # use any config. Position data is equal
        cfg = get_density_cfg(self.result_base_dir())
        sim = Simulation.from_dpmm_cfg(cfg)
        NodePositionWithRsdHdf.get_or_create(
            sim=sim, hdf_path=sim.path("position.h5"), override_existing=True
        )

    @process_as({"prio": 994, "type": "post"})
    def create_node_tx_hdf(self) -> NodeTxData:
        # use any config. Position data is equal
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        d_sim = Simulation.from_dpmm_cfg(d_cfg)
        e_cfg = get_entropy_cfg(_dir)
        e_sim = Simulation.from_dpmm_cfg(e_cfg)

        # pos is equal for both configs
        pos = NodePositionWithRsdHdf.get_or_create(
            d_sim,
            d_sim.path("position.h5"),
        )
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
        pos = NodePositionWithRsdHdf.get_or_create(
            d_sim,
            d_sim.path("position.h5"),
        )
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

    @process_as({"prio": 980, "type": "post"})
    def append_err_measure_hdf(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg)
            OppAnalysis.append_err_measures_to_hdf(sim)

    @process_as({"prio": 971, "type": "post"})
    def create_map_plot(self):
        # use any config. Position data is equal
        d_cfg = get_density_cfg(self.result_base_dir())
        d_sim = Simulation.from_dpmm_cfg(d_cfg)
        pos = NodePositionWithRsdHdf.get_or_create(
            d_sim,
            d_sim.path("position.h5"),
        )

        _out = d_cfg.makedirs_output("fig_out", exist_ok=True)
        fig, ax = PlotEnb.plot_node_enb_association_map(
            rsd=pos,
            coord=CoordinateType.xy_no_geo_offset,
            base_cmap="tab20",
            inner_r=650.0,
        )
        figpath = os.path.join(_out, "trace_map_2.png")
        logger.info(f"save {figpath}")
        fig.savefig(figpath)
        plt.close(fig)

    @process_as({"prio": 970, "type": "post"})
    def create_common_plots(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            result_dir, builder, sql = OppAnalysis.builder_from_cfg(cfg)
            _out = cfg.makedirs_output("fig_out", exist_ok=True)
            with PdfPages(self.result_dir(_out, "app_data.pdf")) as pdf:
                PlotEnb.plot_served_blocks_ul_all(
                    self.result_base_dir(), sql, FigureSaverPdfPages(pdf)
                )

            if sql.is_count_map():
                print("build count based default plots")
                s = FigureSaverSimple(
                    self.result_dir(
                        _out,
                    )
                )
                builder.only_selected_cells(
                    self.ns.get("hdf_cell_selection_mode", True)
                )
                PlotDpmMap.create_common_plots_density(
                    result_dir,
                    builder,
                    sql,
                    selection=builder.get_selected_alg(),
                    saver=s,
                )
            else:
                print("build entropy map based plots")

    @process_as({"prio": 965, "type": "post"})
    def add_plot_tx_interval_plots(self):
        _dir = self.result_base_dir()
        d_cfg = get_density_cfg(_dir)
        _out = d_cfg.makedirs_output("fig_out", exist_ok=True)
        saver = FigureSaverSimple(override_base_path=_out, figure_type=".png")
        tx = self.create_node_tx_hdf()
        for app in tx.apps:
            PlotAppTxInterval.plot_txinterval_from_hdf_all(
                data=tx, app_name=app.name, data_root=None, saver=saver
            )

    @process_as({"prio": 965, "type": "post"})
    def add_plots(self):
        cfg: DpmmCfg
        for cfg in get_cfg_list(self.result_base_dir()):
            sim = Simulation.from_dpmm_cfg(cfg, label="")
            p = cfg.makedirs_output("fig", exist_ok=True)
            saver = FigureSaverSimple(override_base_path=p, figure_type=".png")
            # agent count data
            print("app misc")
            if cfg.is_count_map():
                PlotAppMisc.plot_number_of_agents(sim, saver=saver)

            # application data
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

            # map specifics
            dmap = sim.get_dcdMap()
            if cfg.is_count_map():
                dmap.plot_map_count_diff(savefig=saver.with_name("Map_count.png"))
                dmap.plot_map_count_diff_by_rsd(saver=saver.with_name("Map_count.png"))

            msce = dmap.cell_count_measure(columns=["cell_mse"])

            # msce time series
            PlotDpmMap.plot_msce_ts(msce, savefig=saver.with_name("Map_msce_ts.png"))
            # msce ecdf
            PlotDpmMap.plot_msce_ecdf(
                msce["cell_mse"], savefig=saver.with_name("Map_msce_ecdf.png")
            )

    @process_as({"prio": 100, "type": "post"})
    def repack_1(self):
        h = BaseHdfProvider(get_density_cfg(self.result_base_dir()).hdf_path())
        print(h._hdf_path)
        h.repack_hdf(keep_old_file=False)

    @process_as(
        {"prio": 100, "type": "post"}
    )  # todo mark some post processing task parallel?
    def repack_2(self):
        h = BaseHdfProvider(get_entropy_cfg(self.result_base_dir()).hdf_path())
        print(h._hdf_path)
        h.repack_hdf(keep_old_file=False)

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
