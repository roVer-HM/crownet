# %%
from posixpath import basename
import pandas as pd
from crownetutils.crownet_dash.dashapp import OppModel
from crownetutils.analysis.hdf.provider import BaseHdfProvider
import seaborn as sns
import os
import glob

from crownetutils.utils.misc import Project
import contextily as ctx
from crownetutils.analysis.omnetpp import OppAnalysis
from crownetutils.crownet_dash.dashboard import DashBoard
from crownetutils.omnetpp.scave import CrownetSql
from crownetutils.analysis.dpmm.builder import DpmmHdfBuilder

from os.path import join, abspath


def main(data_root: str, builder: DpmmHdfBuilder, sql: CrownetSql):
    beacon_apps = sql.m_app0()
    map_apps = sql.m_app1()

    i = pd.IndexSlice

    # cells = builder.position_p.geo(Project.OpenStreetMaps)[i[101., :]]
    cells = builder.map_p.geo(Project.OpenStreetMaps)[i[101.0, :, :, :, 13817]]

    pos_hdf = BaseHdfProvider(join(data_root, "postion.h5"))
    with pos_hdf.query as ctx:
        nodes = ctx.select("trajectories", where="time == 101.2")
        nodes = sql.apply_geo_position(nodes, Project.UTM_32N, Project.OpenStreetMaps)
    m = DensityMap.get_interactive(cells=cells, nodes=nodes)
    return m, cells, nodes


_suqc = [
    os.environ["HOME"],
    "repos/crownet/crownet/simulations/mucFreiheitLte/suqc/circle/simulation_runs/Sample__0_0/results/",
]
_repo = [
    os.environ["HOME"],
    "repos/crownet/crownet/simulations/mucFreiheitLte/results/",
]
_ext = ["/mnt/data1tb/results/"]


data = [
    dict(  # 0
        value=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-12:46:32"),
        label="0224-12:46:32 (B) | 130.0s | single source; mf_circle; 30 ped; ymfPlusDist=0.5; logSelected",
    ),
    dict(  # 1
        value=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-13:56:46"),
        label="0224-13:56:46 (B) | 130.0s | dual source; mf_circle; 30+(30static) ped; ymfPlusDist=0.5; logSelected",
    ),
    dict(  # 2
        value=join(*_suqc, "subwayDynamic_multiEnb_compact_density_20220224-15:50:17"),
        label="0224-15:50:17 (B) | 250.0s | dual source; mf_circle; 30+(30quick removed) ped; ymfPlusDist=0.5; logSelected; mI=1s",
    ),
    dict(  # 2
        value=join(
            *_suqc, "subwayDynamic_multiEnb_compact_density_20220225-09:06:57_ymf"
        ),
        label="0225-09:06:57_ymf (A) | 250.0s  | dual source; mf_circle; 30+(30quick removed) ped;  ymf; logAll",
    ),
    dict(  # 10
        value=join(*_repo, "ymf_4s_2ttl_new_20220310T134303.326544"),
        label="ymf_4s_new_exact_dist (TTL) | 250.0s  | triple source; mf_circle; 30+30+10 ped; ymf,ttl20s; logAll; mI=4s",
    ),
    dict(  # 4
        value=join(
            *_suqc,
            "subwayDynamic_multiEnb_compact_density_20220225-10:57:21_ymfPlus_4s_1",
        ),
        label="ymfPlus_4s_1 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
    ),
    dict(  # 5
        value=join(
            *_suqc,
            "subwayDynamic_multiEnb_compact_density_20220228-09:20:17_ymfPlus_4s_2",
        ),
        label="ymfPlus_4s_2 (C,A) | 250.0s  | triple source; mf_circle2; 30+30+10 ped; ymfPlusDist=0.5; logAll; mI=4s !!!",
    ),
    dict(  # 6
        value=join(
            *_suqc,
            "subwayDynamic_multiEnb_compact_density_20220228-14:56:11_ymfPlus_4s_3",
        ),
        label="ymfPlus_4s_3 (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s",
    ),
    dict(  # 7
        value=join(*_repo, "ymfPlusDist_4s_1_new_20220309T160413.482974"),
        label="ymfPlus_4s_1_new (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
    ),
    dict(  # 7
        value=join(*_repo, "ymfPlusDist_4s_1_new_20220311T113814.135169"),
        label="ymfPlus_4s_1_new2 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
    ),
    dict(  # 7
        value=join(*_ext, "ymfPlusDist_4s_1_new_20220311T124331.584364"),
        label="ymfPlus_4s_1_new3 (C) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5; logAll; mI=4s",
    ),
    dict(  # 8
        value=join(*_repo, "ymfPlusDist_4s_2_new_20220309T160733.541658"),
        label="ymfPlus_4s_2_new (C,A) | 250.0s  | triple source; mf_circle2; 30+30+10 ped; ymfPlusDist=0.5; logAll; mI=4s !!!",
    ),
    dict(  # 9
        value=join(*_repo, "ymfPlusDist_4s_3_new_20220309T160511.882464"),
        label="ymfPlus_4s_3_new (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s",
    ),
    dict(  # 10
        value=join(*_repo, "ymfPlusDist_4s_3_new_20220310T133600.160946"),
        label="ymfPlus_4s_3_new_exact_dist (C,TTL) | 250.0s  | dual source; mf_circle; 30+(30quick removed)  ped; ymfPlusDist=0.5ttl20s; logAll; mI=4s",
    ),
    dict(  # 7
        value=join(*_ext, "ymfD_4s_long_ttl_20220311T152956.089002"),
        label="ymfPlus_4s_1_new long with ttl",
    ),
    dict(  # 7
        value=join(*_ext, "ymfD_4s_long_20220311T153230.245624"),
        label="ymfPlus_4s_1_new long without ttl",
    ),
]


def data_dict(path, prefix=""):
    runs = glob.glob(path)
    ret = []
    if prefix == "":
        _n = lambda i, x: x
    else:
        _n = lambda i, x: f"{prefix}_{i}_{x}"

    for i, p in enumerate(runs):
        if os.path.isdir(p):
            label = _n(i, basename(p))
            ret.append({"value": abspath(p), "label": label})
    return ret


if __name__ == "__main__":

    # _d = data
    # data_root, builder, sql = OppAnalysis.builder_from_output_folder(_d[-1]["value"])
    _d = []
    # _d = data_dict("/mnt/data1tb/results/updated_dist/*")
    # _d.extend(data_dict("/mnt/data1tb/results/transmit_wErr_dist/*"))
    # _d.extend(data_dict("/mnt/data1tb/results/transmit_dist/*"))
    _d.extend(
        data_dict(
            "/mnt/data1tb/results/ymfDistDbg/simulation_runs/outputs/*/final_out",
            prefix="step_err_ymf",
        )
    )
    _d.extend(
        data_dict(
            "/mnt/data1tb/results/ymfDistDbg2/simulation_runs/outputs/*/final_out",
            prefix="ymfDistStep2",
        )
    )
    _d.extend(
        data_dict(
            "/mnt/data1tb/results/ymfDistDbg3/simulation_runs/outputs/*/final_out",
            prefix="ymfDistStep3",
        )
    )
    # _d.extend(data_dict("/mnt/data1tb/results/updated_dist_long/*"))
    _d.sort(key=lambda x: x["label"])
    data_root, builder, sql = OppAnalysis.builder_from_output_folder(_d[1]["value"])
    # data_root, builder, sql = OppAnalysis.builder_from_output_folder(data_root)
    builder.only_selected_cells(False)
    # sel = OppAnalysis.find_selection_method(builder)

    print("build app")
    m = OppModel(data_root, builder, sql)
    # df = m.erroneous_cells()
    m.copy_common_pdf(_d, append_label=True)

    app = DashBoard.combined("Foo", "", m)
    app.add(DashBoard.data_selector, _d)
    app.add(DashBoard.map_view)
    app.add(DashBoard.cell_err_app)
    app.run_app()

    print("done")
# %%
