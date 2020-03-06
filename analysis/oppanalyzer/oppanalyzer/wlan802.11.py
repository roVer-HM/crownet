import itertools
import re
from pprint import pprint

import matplotlib as mp
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from numpy.ma import column_stack

from oppanalyzer import Config, ScaveTool
from oppanalyzer.rover_analysis import Opp

# todo [ ] stacked_bar with  percentage
# todo [ ] dynamically select :sum or :count for scalar values


def stacked_bar(
    ax,
    data,
    data_sum,
    c_map="PuBuGn",
    c_interval=[0.2, 0.7],
    bar_width=0.4,
    percentage=False,
    horizontal=False,
    with_table=False,
    tbl_bbox=(0.01, -0.25, 0.99, 0.2),
):
    index = np.arange(data.shape[1]) + 0.3
    offset = np.zeros(data.shape[1])
    c = plt.cm.get_cmap(c_map)(np.linspace(c_interval[0], c_interval[1], data.shape[0]))
    c = np.append(c, [[1.0, 1.0, 1.0, 1.0]], axis=0)

    if horizontal:
        ax.set_yticklabels(list(data.columns))
        ax.set_yticks(index)
    else:
        ax.set_xticklabels(list(data.columns))
        ax.set_xticks(index)

    cell_text = []
    for row in range(data.shape[0]):
        if horizontal:
            ax.barh(
                index, data.iloc[row, :], bar_width, left=offset, color=c[row],
            )
        else:
            ax.bar(
                index, data.iloc[row, :], bar_width, bottom=offset, color=c[row],
            )
        offset = offset + data.iloc[row, :]  # data.loc[:, "value"]
        if with_table:
            cell_text.append([f"{x}" for x in data.iloc[row, :]])

    if with_table:
        cell_text.append([f"{x}" for x in data_sum.iloc[0, :]])
        row_lables = list(data.index)
        row_lables.extend(list(data_sum.index))

        ax.table(
            cellText=cell_text,
            rowLabels=row_lables,
            rowColours=c,
            colLabels=data.columns,
            loc="bottom",
            bbox=list(tbl_bbox),
        )
    return ax


def mac_drop_ration(_df: pd.DataFrame):
    """
    rx_drop_ration = sum(rxDrop)/sum(rx)
    tx_drop_ration = sum(txDrop)/sum(tx)

    Note: packetDropOther:count is added to rxDrop. #TODO check code if this is correct.
    """
    stats = {
        "rxDrop": [
            "packetDropDuplicateDetected:count",
            "packetDropIncorrectlyReceived:count",
            "packetDropNotAddressedToUs:count",
            "packetDropOther:count",
        ],
        "rx": ["packetReceivedFromLower:count"],
        "txDrop": [
            "packetDropQueueOverflow:count",
            "packetDropRetryLimitReached:count",
        ],
        "tx": ["packetReceivedFromUpper:count"],
    }
    all_stats = list(stats.values())
    all_stats = list(itertools.chain.from_iterable(all_stats))
    _df = (
        _df.opp.filter()
        .scalar()
        .module_regex(".*mac$")
        .name_in(all_stats)
        .apply(columns=("name", "value", "module", "run"), copy=True)
    )
    runs = {run: idx for idx, run in enumerate(_df.run.unique())}
    _df["run"] = _df.run.apply(lambda x: runs[x])
    _df["module"] = _df.module.apply(lambda x: Opp.module_path(x, 1) + ".mac")
    _df = _df.pivot_table(index=["run", "module"], columns=["name"], values="value")
    _df["rx_drop_ration"] = _df.loc[:, stats["rxDrop"]].sum(axis=1) / _df.loc[
        :, stats["rx"]
    ].sum(axis=1)
    _df["tx_drop_ration"] = _df.loc[:, stats["txDrop"]].sum(axis=1) / _df.loc[
        :, stats["tx"]
    ].sum(axis=1)
    _df_network = _df.groupby("run").sum()
    _df_network["module"] = "network"
    _df_network.set_index(keys=["module"], append=True, inplace=True)
    _df_network["rx_drop_ration"] = _df_network.loc[:, stats["rxDrop"]].sum(
        axis=1
    ) / _df_network.loc[:, stats["rx"]].sum(axis=1)
    _df_network["tx_drop_ration"] = _df_network.loc[:, stats["txDrop"]].sum(
        axis=1
    ) / _df_network.loc[:, stats["tx"]].sum(axis=1)
    _df = _df.append(_df_network)

    return {"data": _df, "run_keys": runs}


def mac(_df: pd.DataFrame):

    stats = {
        "mac_pkt_drop": {
            "module_filter": ".*hostMobile.*\.mac$",
            "total": "packetDrop:count",
            "data": [
                "packetDropDuplicateDetected:count",  # rx
                "packetDropIncorrectlyReceived:count",  # rx
                "packetDropNotAddressedToUs:count",  # rx
                "packetDropOther:count",  # rx
                "packetDropQueueOverflow:count",  # tx
                "packetDropRetryLimitReached:count",  # tx
            ],
        },
        "mac_dcf_rx": {
            "module_filter": ".*hostMobile.*\.mac\.dcf$",
            "total": "packetReceivedFromPeer:count",
            "data": [
                "packetReceivedFromPeerBroadcast:count",
                "packetReceivedFromPeerMulticast:count",
                "packetReceivedFromPeerMulticast:count",
            ],
        },
        "mac_dcf_rx_retry": {
            "module_filter": ".*hostMobile.*\.mac\.dcf$",
            "total": "packetReceivedFromPeer:count",
            "data": [
                "packetReceivedFromPeerWithoutRetry:count",
                "packetReceivedFromPeerWithRetry:count",
            ],
        },
        "mac_dcf_tx": {
            "module_filter": ".*hostMobile.*\.mac\.dcf$",
            "total": "packetSentToPeer:count",
            "data": [
                "packetSentToPeerBroadcast:count",
                "packetSentToPeerMulticast:count",
                "packetSentToPeerUnicast:count",
            ],
        },
        "mac_dcf_tx_retry": {
            "module_filter": ".*hostMobile.*\.mac\.dcf$",
            "total": "packetSentToPeer:count",
            "data": [
                "packetSentToPeerWithoutRetry:count",
                "packetSentToPeerWithRetry:count",
            ],
        },
    }
    stat = stats["mac_pkt_drop"]
    data = (
        _df.opp.filter()
        .run_regex(".*Dimensional-0.*")
        .module_regex(stat["module_filter"], allow_number_range=True)
        .name_in(stat["data"])
        .apply(columns=("name", "value", "module"), copy=True)
    )
    data["module"] = data.module.apply(
        lambda x: "m" + Opp.module_path(x, 1, tuple_on_vector=True)[1]
    )
    data = data.pivot(index="name", columns="module", values="value")

    data_global = (
        _df.opp.filter()
        .run_regex(".*Dimensional-0.*")
        .module_regex(stat["module_filter"], allow_number_range=True)
        .name(stat["total"])
        .apply(columns=("name", "value", "module"), copy=True)
    )
    data_global = data_global.pivot(index="name", columns="module", values="value")

    fig = plt.figure(figsize=(15, 10))
    ax = fig.add_axes([0.19, 0.2, 0.80, 0.79,])
    ax = stacked_bar(ax, data, data_global, horizontal=True, with_table=True)
    ax.set_facecolor((0.5, 0.5, 0.5))
    fig.set_facecolor((0.4, 0.4, 0.4))
    fig.show()


if __name__ == "__main__":
    cfg = Config()
    scv = ScaveTool(cfg)
    csv = scv.create_or_get_csv_file(
        csv_path="/home/sts/repos/rover-main/rover/simulations/simple_detoure/results/res01.csv",
        input_paths=[
            "/home/sts/repos/rover-main/rover/simulations/simple_detoure/results/res01*/*.vec",
            "/home/sts/repos/rover-main/rover/simulations/simple_detoure/results/res01*/*.sca",
        ],
        scave_filter='module("*.mac.*") OR module("*.mac")',
        override=False,
        recursive=True,
    )
    df = scv.load_csv(csv)
    r = mac_drop_ration(df)
    print(r["data"].loc[:, ("rx_drop_ration", "tx_drop_ration")])
    pprint(r["run_keys"])

    data = (
        df.opp.filter()
        # .run_regex(".*Dimensional-0.*")
        .module_regex(
            ".*(hostMobile|station).*\.mac\.dcf\.channelAccess\.inProgressFrames$",
        )
        # .name_in(["queueingTime:vector", "queueLength:vector"])
        .name_in(["queueingTime:vector"]).apply(copy=True)
    )
    fig, ax = plt.subplots()
    df.opp.plot.create_histogram(
        ax, data.iloc[12], use_path_in_title=1, attr_override={"unit": "ms"}
    )
    fig.show()
