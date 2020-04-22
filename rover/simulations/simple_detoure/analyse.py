import os

import roveranalyzer.oppanalyzer.wlan80211 as w80211
from roveranalyzer.oppanalyzer.utils import RoverBuilder
from roveranalyzer.uitls.mesh import SimpleMesh
from roveranalyzer.uitls.path import PathHelper
from roveranalyzer.vadereanalyzer.plots.plots import t_cmap, DensityPlots, PlotOptions


def mac_analysis(base_dir, fig_title, vec_input, prefix, out_input=None):
    builder = RoverBuilder(
        path=PathHelper(base_dir),
        analysis_name=f"{prefix}mac",
        analysis_dir="analysis.d",
        hdf_store_name="analysis.h5",
    )
    builder.set_scave_filter('module("*.hostMobile[*].*.mac")')
    builder.set_scave_input_path(vec_input)
    if out_input is not None:
        _log_file = builder.root.join(out_input)
    else:
        _log_file = ""
    w80211.create_mac_pkt_drop_figures(
        builder=builder,
        log_file=_log_file,
        figure_title=fig_title,
        figure_prefix=prefix,
        hdf_key=f"/df/{prefix}mac_pkt_drop_ts",
        show_fig=True,
    )

def stream_plot(base_dir, v_out, fig_title, save_as, time, print_time , max_density, norm):
    builder = RoverBuilder(
        path=PathHelper(base_dir),
        analysis_name=f"stream_video",
        analysis_dir="analysis.d",
        hdf_store_name="analysis.h5",
    )
    vadere_out = builder.vadere_output_from(v_out)
    cmap_dict = {
        "informed_peds": t_cmap(cmap_name="Reds", replace_index=(0, 1, 0.0)),
        "all_peds": t_cmap(
            cmap_name="Blues",
            zero_color=(1.0, 1.0, 1.0, 1.0),
            replace_index=(0, 1, 1.0),
        ),
    }
    density_plot = DensityPlots.from_mesh_processor(
        vadere_output=vadere_out,
        mesh_out_file="Mesh_trias1.txt",
        density_out_file="Density_trias1.csv",
        cmap_dict=cmap_dict,
        data_cols_rename=["all_peds", "informed_peds"],
    )
    density_plot.plot_density(
        time=time,
        option=PlotOptions.DENSITY,
        fig_path=os.path.join(vadere_out.output_dir, save_as),
        max_density=max_density,
        norm=norm,
        plot_data=("all_peds", "informed_peds"),
        color_bar_from=(0, 1),
        cbar_lbl=("DensityAll [#/m^2]", "DensityInformed [#/m^2]"),
        title=fig_title,
        print_time=print_time
    )


def stream_video(
    base_dir, v_out, fig_title, save_as, time_interval, slow_motion, max_density, norm
):
    builder = RoverBuilder(
        path=PathHelper(base_dir),
        analysis_name=f"stream_video",
        analysis_dir="analysis.d",
        hdf_store_name="analysis.h5",
    )
    vadere_out = builder.vadere_output_from(v_out)
    cmap_dict = {
        "informed_peds": t_cmap(cmap_name="Reds", replace_index=(0, 1, 0.0)),
        "all_peds": t_cmap(
            cmap_name="Blues",
            zero_color=(1.0, 1.0, 1.0, 1.0),
            replace_index=(0, 1, 1.0),
        ),
    }
    density_plot = DensityPlots.from_mesh_processor(
        vadere_output=vadere_out,
        mesh_out_file="Mesh_trias1.txt",
        density_out_file="Density_trias1.csv",
        cmap_dict=cmap_dict,
        data_cols_rename=["all_peds", "informed_peds"],
    )
    density_plot.set_slow_motion_intervals(slow_motion)
    density_plot.animate_density(
        PlotOptions.DENSITY,
        os.path.join(vadere_out.output_dir, save_as),
        animate_time=time_interval,
        max_density=max_density,
        norm=norm,
        plot_data=("all_peds", "informed_peds"),
        color_bar_from=(0, 1),
        cbar_lbl=("DensityAll [#/m^2]", "DensityInformed [#/m^2]"),
        title=fig_title,
    )


if __name__ == "__main__":
    _base_dir = os.path.join(os.path.dirname(__file__), "results")

    # packet  drop rate
    # mac_analysis(base_dir=os.path.join(os.path.dirname(__file__), "results/final_2020-04-03-1585946750"),
    #              fig_title="Mac package drop ratio (20% module penetration)",
    #              vec_input="vars_p1Rate0.2_p2Rate0.8_213_rep_0.vec",
    #              out_input="vars_p1Rate0.2_p2Rate0.8_213_rep_0.out",
    #              prefix="2080_213_")

    # packet  drop rate
    # mac_analysis(base_dir=os.path.join(os.path.dirname(__file__), "results", "final_2020-04-03-1585946750"),
    #              fig_title="Mac package drop ratio (80% module penetration)",
    #              vec_input="vars_p1Rate0.8_p2Rate0.2_213_rep_0.vec",
    #              out_input="vars_p1Rate0.8_p2Rate0.2_213_rep_0.out",
    #              prefix="8020_213_")

    # stream video
    # stream_video(
    #     base_dir=os.path.join(
    #         os.path.dirname(__file__), "results/final_2020-04-03-1585946750"
    #     ),
    #     v_out="vars_p1Rate0.2_p2Rate0.8_213_rep_0",
    #     fig_title="Mapped density (20% communication)",
    #     save_as="2080_213_stream.mp4",
    #     max_density=1.2,
    #     norm=(1.0, 0.2),
    #     time_interval=(400.0, 498.0),
    #     slow_motion=[(250.0, 257.0, 24)],
    # )
    stream_plot(
        base_dir=os.path.join(
            os.path.dirname(__file__), "results/final_2020-04-03-1585946750"
        ),
        v_out="vars_p1Rate0.2_p2Rate0.8_213_rep_0",
        fig_title="Mapped density (20% communication)",
        save_as="2080_213_stream.png",
        max_density=1.2,
        norm=(1.0, 0.2),
        time=480.0,
        print_time=False
    )

