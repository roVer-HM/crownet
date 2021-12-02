# %%
import os
from typing import Tuple
from roveranalyzer.simulators.crownet.dcd.dcd_map import savefigure

from roveranalyzer.utils import Project
import roveranalyzer.simulators.crownet.dcd as Dmap
import roveranalyzer.simulators.opp as OMNeT

def run_data(run, sim, hdf_file="data.h5") -> Tuple[str, Dmap.DcdHdfBuilder, OMNeT.CrownetSql]:
    data_root = f"{os.environ['HOME']}/repos/crownet/crownet/simulations/{sim}/results/{run}"
    builder = Dmap.DcdHdfBuilder.get(hdf_file, data_root).epsg(Project.UTM_32N)
    
    sql = OMNeT.CrownetSql(
            vec_path=f"{data_root}/vars_rep_0.vec", 
            sca_path=f"{data_root}/vars_rep_0.sca", 
        network="World")
    return data_root, builder, sql


# %% 
#
#  'plot_error_histogram' creates histogram plots for density map cell errors, aggregated over the given time slice and 
#   all nodes (pedestrians). Use 'value' to specify which error measure to use 'err' for absolute error and 'sqerr' for 
#   squared error. Use 'agg_func' to specify how aggregate the error values over all pedestrians and time steps. 
#   
root_path1, b1, sql1 = run_data("vadere_XX_20211129-11:44:47", "mucFreiheitLte")
p1 = b1.build(override_hdf=False)
dcd = b1.build_dcdMap()

# %%
# plots single 
fig1, ax1 = dcd.plot_error_histogram(stat="percent", savefig=f"{root_path1}/out/error_hist.pdf")
# %%
# plots multiple histograms  of different times (and all times)
fig2, ax2 = dcd.plot_error_quantil_histogram(stat="percent", savefig=f"{root_path1}/out/error_hist_time.pdf")

# %%
