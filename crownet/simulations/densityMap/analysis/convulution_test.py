import os
import numpy as np
from roveranalyzer.analysis import VadereAnalysis
from roveranalyzer.analysis.ts_analysis import adf_summary_test, adf_test
import scipy as sp
import pandas as pd
from scipy.sparse import coo_array, coo_matrix, csc_array, csr
import scipy.signal as sg
import scipy.ndimage as ndi
from matplotlib.backends.backend_pdf import PdfPages


def main():
    row = [2, 2, 2, 3, 3, 3, 4, 4, 4]
    col = [2, 3, 4, 2, 3, 4, 2, 3, 4]
    data = np.ones(9)
    sp = coo_array((data, (row, col)), shape=(7, 7))
    # spa = coo_matrix((data, (row, col)), shape=(7,7))
    # spm = spa.tocsr()
    # spm.A
    kernel = 1 / 9 * np.ones(shape=(3, 3))
    print(sp.toarray())
    print(kernel)

    # x = ndi.correlate(sp.toarray(), kernel)
    x = sg.convolve2d(sp, kernel, mode="same")
    print(x)


if __name__ == "__main__":

    p = "/home/vm-sts/repos/crownet/crownet/simulations/densityMap/vadere/output/"
    pp = [5, 10, 15, 20, 25]
    pdf = PdfPages(os.path.join(p, "out_750.pdf"))
    _adf = []
    for i in pp:
        path = os.path.join(p, f"mf_dyn_{i}/numAgents.csv")
        path_new = f"{path[0:-4]}.pdf"
        df = VadereAnalysis.read_time_step(
            path, cols=[f"iter_arrival_time_{i}s"], frame_consumer=lambda x: x.loc[:750]
        )
        VadereAnalysis.plot_number_agents_over_time(df, savefig=pdf)
        _adf.append(adf_summary_test(df, df.columns[0]))
    pd.concat(_adf, axis=1).to_csv(os.path.join(p, "out_adf_750.csv"), sep=";")
    pdf.close()
    print("")
