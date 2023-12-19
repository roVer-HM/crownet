from __future__ import annotations
from dataclasses import dataclass, field
import sys
import os
from typing import Iterator, List
from crownetutils.analysis.hdf.provider import BaseHdfProvider, HdfSelector
from crownetutils.analysis.hdf_providers.node_position import NodePositionWithRsdHdf
from crownetutils.analysis.hdf_providers.node_rx_data import NodeRxData
from crownetutils.analysis.hdf_providers.node_tx_data import NodeTxData
from crownetutils.analysis.hdf_providers.sql_app_proxy import SqlAppProxy
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.utils.plot import FigureSaver, FigureSaverSimple, PlotUtil
from crownetutils.utils.dataframe import combine_stats, index_or_col
from crownetutils.utils.styles import STYLE_SIMPLE_169, load_matplotlib_style
from crownetutils.analysis.common import RunMap, Simulation, SuqcStudy
from matplotlib import pyplot as plt
from matplotlib.ticker import EngFormatter, MultipleLocator
import numpy as np
import pandas as pd
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfg, DpmmCfgCsv, DpmmCfgDb, MapType
from crownetutils.analysis.plot import PlotEnb, PlotAppTxInterval, PlotDpmMap

from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)


from run_script import SimulationRun, get_density_cfg, get_entropy_cfg

load_matplotlib_style(STYLE_SIMPLE_169)


def get_sim(path):
    return Simulation.from_dpmm_cfg(path)

def get_RunMap(path):
    study = SuqcStudy(base_path=path)
    study.get_sim()


def plot_delay_for_rsd_7and8(node_rx, node_tx, saver):
    apps = node_tx.apps
    fig, ax = plt.subplots(nrows=3, ncols=1, figsize=(16,16), sharex=True)
    for i, app in enumerate(apps):
        a: plt.Axes = ax[i]
        for rsd in [7, 8]:
            data = node_rx.rcvd_data(app).select(where=f"servingEnb={rsd}")
            a.scatter(data.index.get_level_values("time"), data["delay"], label=f"rsd{rsd}", alpha=.3)
        a.legend()
        PlotUtil.auto_major_minor_locator(a)
        a.set_title(app.name)
        a.set_xlabel("time in seconds")
        a.set_ylabel("dealy in seconds")
    fig.tight_layout
    saver(fig, "dealy_for_rsd_7and8.png")

@dataclass
class Sample:
    lbl: str
    data_dir: str 
    runner: SimulationRun = field(init=False)
    cfg_d: DpmmCfgDb = field(init=False)
    cfg_e: DpmmCfgDb = field(init=False)

@dataclass
class SampleGroup:
    all_interference: Sample
    no_interference: Sample
    only_d2d_interference: Sample
    more_dBm_all_interference: Sample
    more_dBm_no_interference: Sample
    more_dBmw_only_d2d_interference: Sample
    macro_all_interference: Sample
    macro_no_interference: Sample
    macro_only_d2d_interference: Sample

    saver: FigureSaverSimple
    time_interval: slice

    def samples(self) -> List[Sample]:
        return [
            self.all_interference,
            self.only_d2d_interference,
            self.no_interference,
            self.more_dBm_all_interference,
            self.more_dBm_no_interference,
            self.more_dBmw_only_d2d_interference,
            self.macro_all_interference,
            self.macro_no_interference,
            self.macro_only_d2d_interference
            ]
    
    def __iter__(self) -> Iterator[Sample]: 
        return iter(self.samples())

    def iter_rx_sample(self, columns=None, where=None):
        for sample_idx, sample in enumerate(self):
            rx_data: NodeRxData = sample.runner.create_node_rx_hdf()
            for app_idx, app in enumerate([rx_data.apps[0]]):
                selector = HdfSelector(
                    hdf=rx_data.rcvd_data(app),
                    group=app.group_by_app("rcvd_stats"),
                    columns=["delay", "jitter"],
                    where=f"{where} and time >= {self.time_interval.start} and time <= {self.time_interval.stop}",
                )
                with selector.hdf.ctx() as c:
                    df = c.select(**selector.select_args())
                yield sample_idx, sample, app_idx, app, df, rx_data

def get_host_from_vec(vec, pos):
    p = pos.ue.select(where=f"vecIdx={vec}")
    return p.iloc[0,0]

def foo(samples: SampleGroup):

    for _, sample, _ , app, df, rx in samples.iter_rx_sample(where="delay > 1.0 and time < 400"):
        df2 = df.reset_index(["eventNumber", "time"])[["time"]]
        df2["left"] = df2["time"] - 5.0
        df2["right"] = df2["time"] + 5.0
        max_delay_senders = df.groupby("srcHostId")["delay"].count().sort_values(ascending=False)
        max_delay_senders.name = "count"
        sender_id = int(max_delay_senders.index[0])
        print(f"For Sample {sample.lbl} and application {app.name} the top3 sender nodes most delays above 1.0s:")
        print(max_delay_senders.head(3))
        print("-----")

        max_delay_receivers = df.loc[pd.IndexSlice[:, sender_id],:].reset_index().groupby("hostId")["delay"].count().sort_values(ascending=False)
        max_delay_receivers.name="count"
        receiver_id = int(max_delay_receivers.index[0])
        print(f"For Sample {sample.lbl}, application {app.name} and the max delay sender '{sender_id}' the top3 receiver fo delayed packets:")
        print(max_delay_receivers.head(3))
        print("-----")

        pos: NodePositionWithRsdHdf = sample.runner.create_position_hdf()
        tx = sample.runner.create_node_tx_hdf()
        sender_id = get_host_from_vec(51, pos) # 7392 / 7407
        receiver_id = get_host_from_vec(766, pos) #131076 / 131498
        print(f"use sender {sender_id} and reciever {receiver_id}")

        delays =  rx.rcvd_data(app).select(where=f"hostId={receiver_id} and srcHostId={sender_id}")
        print(f"For Sampel {sample.lbl}, application {app.name} and packet reception between hostId {receiver_id} <- srcHostId {sender_id}:")
        print(f"First reception between pair at {delays.index.get_level_values('time').min()}")
        print(f"First reception between pair with delay > 1.0 s at {delays[delays['delay']>1.0].index.get_level_values('time').min()}")
        print("-----")

        save_trace(samples.saver, pos, [receiver_id, sender_id], app.name, sample.lbl)

        sender_pos = pos.ue.select(where=f"hostId={sender_id} and time < 400")
        print(f"Sender pos:\n{sender_pos.head(3)}")
        sender_interval = tx.tx_interval(app).select(where=f"hostId={sender_id} and time < 400")
        sender_bytes = tx.tx_bytes(app).select(where=f"hostId={sender_id} and time < 400")
        receiver_pos = pos.ue.select(where=f"hostId={receiver_id} and time < 400") 
        print(f"Receiver pos:\n{receiver_pos.head(3)}")

        p = "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/compare_delay_with_interference.d"
        sender_pos.to_csv(f"{p}/sender_pos_{app.name}_{sample.lbl}.csv")
        receiver_pos.to_csv(f"{p}/receiver_pos_{app.name}_{sample.lbl}.csv")
        sender_interval.to_csv(f"{p}/sender_interval_{app.name}_{sample.lbl}.csv")
        sender_bytes.to_csv(f"{p}/sender_bytes_{app.name}_{sample.lbl}.csv")
        delays.to_csv(f"{p}/delays_{app.name}_{sample.lbl}.csv")

        print("hi")
    
def plot_outliers(samples: SampleGroup, filter=None):
    pass

def plot_delay(samples: SampleGroup, filter=None):
    _ncols = len(samples.samples())+1
    fig, axess = plt.subplots(nrows=3, ncols=_ncols, figsize=(16*_ncols, 9*3))

    stats_by_app = {}
    for col, sample in enumerate(samples):
        rx_data: NodeRxData = sample.runner.create_node_rx_hdf()
        axes = axess[:, col]
        for row, app in enumerate(rx_data.apps):
            selector = HdfSelector(
                hdf=rx_data.rcvd_data(app),
                group=app.group_by_app("rcvd_stats"),
                columns=["delay", "jitter"],
                where=f"time >= {samples.time_interval.start} and time <= {samples.time_interval.stop}",
            )
            with selector.hdf.ctx() as c:
                df = c.select(**selector.select_args()).reset_index()
            
            if filter is not None:
                df = filter(df)

            ax = axes[row]
            ax.scatter(
                "time",
                "delay",
                data=df,
                marker="x",
                alpha=0.5,
                label="delay")
            ax.scatter(
                "time",
                "jitter",
                data=df,
                marker="+",
                alpha=0.5,
                label="jitter")
            ax_ecdf = axess[row, -1]
            PlotUtil.plot_ecdf(df["delay"],ax=ax_ecdf, label=f"{sample.lbl}" )
            ax_ecdf.set_title(f"Delay ECDF for application {app.name}")
            ax_ecdf.legend(loc="lower right")
            _app_stats = stats_by_app.get(app.name, [])
            _app_stats.append(pd.Series(df["delay"], name=sample.lbl))
            stats_by_app[app.name] = _app_stats

            ax.legend(loc="upper right")
            ax.set_title(f"{sample.lbl} Delay and Jitter over time {app.name}")
            ax.set_ylabel("Delay/Jitter in seconds")
            PlotUtil.auto_major_minor_locator(ax)


    for app,  data in stats_by_app.items():
        data:list
        lbls =[s.lbl for s  in samples.samples()]
        data.sort(key=lambda x: lbls.index(x.name))
        
        _desc = combine_stats(
            [l.replace("interference", "int.").replace("more_dBm_", "dBm_").replace("only_", "") for l in lbls], *data
        )
        _desc = _desc.to_string(float_format=lambda x: f"{x:.2e}")
        app_id = [a.name for a in rx_data.apps].index(app)
        ax_ecdf: plt.Axes = axess[app_id, -1]
        t = ax_ecdf.text(
            3,
            0.5,
            f"Describe:\n{_desc}",
            transform=ax_ecdf.transData,
            horizontalalignment="left",
            verticalalignment="center",
            fontfamily="monospace",
            fontsize="x-large",
        )
        t.set_bbox(dict(facecolor='white', alpha=1.0, edgecolor='black'))
        
    

    for a in axess[-1,:-1]:
        a.set_xlabel("Simulation time in seconds")
    axess[-1,-1].set_xlabel("Delay in seconds")
    #ecdf
    for a in axess[:, -1]:
        a.set_xlim(-5, 55)
        a.set_ylim(0, 1.0)
        a.xaxis.set_major_locator(MultipleLocator(10))
        a.xaxis.set_minor_locator(MultipleLocator(2))
        a.yaxis.set_major_locator(MultipleLocator(.1))
        a.yaxis.set_minor_locator(MultipleLocator(.05))
    # delay TS
    for a in axess[:, :-1].flatten():
        a: plt.Axes
        a.yaxis.set_major_locator(MultipleLocator(10))
        a.yaxis.set_minor_locator(MultipleLocator(2))
        a.set_ylim(0, 50)
        a.set_xlim(samples.time_interval.start-5, samples.time_interval.stop+5)



    fig.tight_layout()
    samples.saver(fig, "delay.png")

def save_trace(saver, pos, nodes, app, sample, time=400):
    n = ",".join([str(i) for i in nodes])

    fig, _ = PlotEnb.plot_node_enb_association_map(
        rsd=pos,
        coord=CoordinateType.xy_no_geo_offset,
        base_cmap="tab20",
        inner_r=500.0,
        ue_where_clause=f"hostId in [{n}] and time < {time}"
    )
    fig.tight_layout()
    name = f"trace_{app}_{sample}_{n.replace(',', '_')}.png"
    saver(fig, name)


def main ():

    out_path = "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/compare_delay_with_interference.d"
    os.makedirs(out_path, exist_ok=True)
    saver: FigureSaverSimple = FigureSaverSimple(out_path)
    
    samples = SampleGroup(
        Sample(
            "all_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_7_0/final_multi_enb_out"
        ),
        Sample(
            "no_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_99971_0/final_multi_enb_out"
        ),
        Sample(
            "only_d2d_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_99972_0/final_multi_enb_out"
        ),
        Sample(
            "more_dBm_all_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_99975_0/final_multi_enb_out"
        ),
        Sample(
            "more_dBm_no_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_99973_0/final_multi_enb_out"
        ),
        Sample(
            "more_dBm_only_d2d_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_99974_0/final_multi_enb_out"
        ),
        Sample(
            "macro_all_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_999070_0/final_multi_enb_out"
        ),
        Sample(
            "macro_no_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_999071_0/final_multi_enb_out"
        ),
        Sample(
            "macro_only_d2d_interference",
            "/data/home/schuhbaeck/simulation_output/s2_ttl_and_stream_3/simulation_runs/outputs/Sample_999072_0/final_multi_enb_out"
        ),
        saver=saver,
        time_interval=slice(0, 250.0)
    )
    

    for sample in samples:
        runner, cfg_d, cfg_e = SimulationRun.postprocessing(
            result_dir=sample.data_dir, exec=False
        )
        sample.runner = runner
        sample.cfg_d = cfg_d
        sample.cfg_e = cfg_e


    samples.saver.with_suffix("macro_all_pkt_250s")
    plot_delay(samples)
    
    # fig, _ = PlotEnb.plot_node_enb_association_map(
    #     rsd=samples.all_interference.runner.create_position_hdf(),
    #     coord=CoordinateType.xy_no_geo_offset,
    #     base_cmap="tab20",
    #     inner_r=500.0,
    #     ue_where_clause="hostId in [24314, 67375, 8688]"
    # )
    # fig.tight_layout()
    # samples.saver(fig, f"trace_xxx_all_inter.png")


    # samples.saver.with_suffix("_only_single_rx")
    # foo(samples)


    

if __name__ == "__main__":
    main()
    print("done.")