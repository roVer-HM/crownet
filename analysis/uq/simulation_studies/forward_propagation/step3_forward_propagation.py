import matplotlib.pyplot as plt
import seaborn as sns

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PostProcessing.UQMethods.ForwardPropagation import ForwardPropagationAnalysis
from uq.PreProcessing.sampling.utils import read_json

if __name__ == "__main__":

    # Read sample values
    samplingMC = read_json("pMCHalton.json")

    # Forward propagation analysis

    # First static quantity of interest (scalar value, does not change over time)
    qoi = QuantityOfInterest.from_suqc_output(file_path="result_df_2.csv",
                                              name="DisseminationTime")
    fw = ForwardPropagationAnalysis(sample_base=samplingMC,
                                    quantity_of_interest_values=qoi
                                    )
    results = fw.get_results()
    #fw.plot_results()
    #fw.hist()

    # First dynamic quantity of interest (time-dependent)
    qoi_time_dep = QuantityOfInterest.from_suqc_output(file_path="result_df_1.csv",
                                                       col_name="percentageInformed-PID12",
                                                       name="PercentageInformed")

    fw2 = ForwardPropagationAnalysis(sample_base=samplingMC,
                                    quantity_of_interest_values=qoi_time_dep
                                    )
    results2 = fw2.get_results()
    #fw2.plot_results()
    #fw2.hist(time_key=1.6)

