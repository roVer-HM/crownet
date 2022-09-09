
import os
import sys

from os.path import join, abspath
import roveranalyzer.simulators.crownet.dcd as DensityMap
from roveranalyzer.utils.general import Project
import roveranalyzer.simulators.opp as OMNeT
"""
Combined figures to compate ymf and umfPlus.
"""
def create_figures(root):
    p1  = f"{root}/crownet/simulations/mucFreiheitLte/suqc/opp_stationary2/simulation_runs/outputs/Sample_0_0/subwayStationary_multiEnb_out"
    p2  = f"{root}/crownet/simulations/mucFreiheitLte/suqc/opp_stationary3_ymfPlus/simulation_runs/outputs/Sample_0_0/subwayStationary_multiEnb_out"
    vec_name = "vars_rep_0.vec"
    sca_name = "vars_rep_0.sca"
    sql1 = OMNeT.CrownetSql(p1, join(p1, vec_name), join( p1, sca_name))
    sql2 = OMNeT.CrownetSql(p2, join(p2, vec_name), join( p2, sca_name))

    def b(r, sel): 
        builder = DensityMap.DcdHdfBuilder.get(
            hdf_path="data.h5",
            source_path=r
        ).epsg(Project.UTM_32N)
        return builder.build_dcdMap(selection=sel)


    dcd1 = b(p1, "ymf")
    dcd2 = b(p2, "ymfPlusDist")
    l_hdl = []
    l_lbl = []
    fig, ax = dcd1.plot_count_diff()
    for l in ax.get_lines():
        if l._label == "Actual count":
            l_hdl.insert(0, l)
            l_lbl.insert(0, "Tatsächliche Anzahl")
        elif l._label == "Mean count":
            l_hdl.append(l)
            l_lbl.append("Mittlere Anzahl (Ymf)")

    fig, ax = dcd2.plot_count_diff(ax=ax)
    l_hdl.append(ax.get_lines()[-1])
    l_lbl.append("Mittlere Anzahl (YmfPlus)")
    fig.legends.clear()
    ax.collections.clear()
    del ax.lines[3]
    ax.set_ylim([20, 110])
    ax.set_xlim([0, 35])
    # ax.get_legend().remove()
    ax.legend(l_hdl, l_lbl, loc="lower right", fontsize="x-large")
    ax.get_legend().legendHandles[-1].set_color(ax.lines[-1]._color)
    fig.show()
    ax.set_title("Personenanzahl über Zeit - Statische Szenario", fontsize=24)
    ax.set_ylabel("Personenanzahl")
    ax.set_xlabel("Zeit [s]")
    f_path = abspath(f"./count_over_time.png")
    print(f"save file to {f_path}")
    fig.savefig(f_path)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        create_figures(sys.argv[1])
    else:
        create_figures(os.environ["CROWNET_HOME"])