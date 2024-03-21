import os

from crownetutils.analysis.common import Simulation
from crownetutils.analysis.dpmm.dpmm_cfg import DpmmCfgBuilder
from crownetutils.analysis.hdf_providers.node_position import (
    CoordinateType,
    NodePositionWithRsdHdf,
)
from crownetutils.utils.logging import timing
from crownetutils.utils.plot import PlotUtil, enb_with_hex

from crownetutils.utils.styles import load_matplotlib_style
from matplotlib import pyplot as plt
from matplotlib.cm import ScalarMappable
from matplotlib.collections import LineCollection
from matplotlib.offsetbox import AnnotationBbox, TextArea
from matplotlib.patches import BoxStyle
from matplotlib.ticker import MultipleLocator
import numpy as np

load_matplotlib_style(os.path.join(os.path.dirname(__file__), "paper_tex.mplstyle"))


@timing
def pull_data_from_simulation_results(sim_dir, cache_base_dir):
    cfg = DpmmCfgBuilder.load_density_cfg(sim_dir)
    sim = Simulation.from_dpmm_cfg(cfg)
    pos = NodePositionWithRsdHdf(sim, cfg.position.path)

    coord: CoordinateType = CoordinateType.xy_no_geo_offset
    base_cmap = "tab20"
    ue_where_clause: str | None = None

    _, segments, segment_colors, rsd_color_map, cmap, norm = pos.get_ue_traces(
        coord, cmap=base_cmap, ue_where_clause=ue_where_clause
    )
    obj = {
        "segments": segments,
        "segment_colors": segment_colors,
        "rsd_color_map": rsd_color_map,
        "norm": norm,
        "cmap": cmap,
    }
    enb = pos.enb.frame()
    enb["color"] = enb["rsd_id"].apply(lambda x: rsd_color_map[x])

    return obj, enb


def main():
    trace, enb = pull_data_from_simulation_results(
        "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/simulation_runs/outputs/Sample_5_0/final_multi_enb_out/",
        "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/",
    )
    out_base = "/mnt/ssd_local/arc-dsa_multi_cell/s2_ttl_and_stream_4/analysis_dir/"

    fig_size = PlotUtil.fig_size_mm(width=111, height=75)
    fig = plt.figure(constrained_layout=False, figsize=fig_size)
    gs1 = fig.add_gridspec(
        nrows=4, ncols=4, top=1.0, right=0.99, bottom=0.09, wspace=0.3, hspace=2.1
    )
    ax = fig.add_subplot(gs1[:, :])

    coord: CoordinateType = CoordinateType.xy_no_geo_offset
    inner_r: float = 500
    with_legend = False

    segments = trace["segments"]
    segment_colors = trace["segment_colors"]
    norm = trace["norm"]
    cmap = trace["cmap"]

    segments = segments / 1000 - np.array([2.1, 2.9])
    enb_pos = enb[coord.cols].values / 1000 - np.array([2.1, 2.9])

    # append enb hex
    enb_patches, hex_patches = enb_with_hex(
        origin=enb_pos, inner_r=inner_r / 1000, scale=250 / 1000, zorder=10
    )
    ax.add_collection(hex_patches)

    lc = LineCollection(
        segments=segments, colors=segment_colors, linewidths=1, alpha=0.5
    )
    fig.subplots_adjust(left=0.07, bottom=0.2)
    ax.add_collection(lc)
    if with_legend:
        cbar = ax.get_figure().colorbar(
            ScalarMappable(norm=norm, cmap=cmap), ticks=MultipleLocator(1), ax=ax
        )

    ax.add_collection(enb_patches)
    for row in range(enb_pos.shape[0]):
        xy = enb_pos[row]
        if row + 1 in [2, 4]:
            xytext = (xy[0] + 0.18, xy[1] - 0.07)
        else:
            xytext = (xy[0] + 0.18, xy[1] - 0.15)

        text = TextArea(
            f"{row+1}",
            textprops=dict(
                size="small",
                horizontalalignment="center",
                verticalalignment="baseline",
                color="black",
            ),
        )
        abox = AnnotationBbox(
            offsetbox=text,
            xy=(xy[0], xy[1]),
            xybox=xytext,
            boxcoords="data",
            frameon=True,
            bboxprops=dict(boxstyle=BoxStyle("Round", pad=0.06)),
            zorder=10,
        )
        ax.add_artist(abox)

    ax.xaxis.set_major_locator(MultipleLocator(0.500))
    ax.yaxis.set_major_locator(MultipleLocator(0.500))
    ax.xaxis.set_minor_locator(MultipleLocator(0.100))
    ax.yaxis.set_minor_locator(MultipleLocator(0.100))

    ax.set_aspect("equal")

    ax.set_xlim(0, 4.3)
    ax.set_ylim(0, 2.8)
    ax.get_figure().tight_layout()

    print("save")
    fig.savefig(os.path.join(out_base, "traces_s2_3.png"), dpi=900)
    # fig.savefig(os.path.join(out_base, "traces_s2_3.pdf"))


if __name__ == "__main__":
    main()
    print("done")
