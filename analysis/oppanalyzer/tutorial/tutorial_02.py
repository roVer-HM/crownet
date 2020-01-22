import sys

import matplotlib.pyplot as plt

from tutorial.imports import *

# Structure based on suq-controller (Daniel Lehmberg)
# This is just to make sure that the systems path is set up correctly, to have correct imports.
# (It can be ignored)
sys.path.append(
    os.path.abspath(".")
)  # in case tutorial is called from the root directory
sys.path.append(os.path.abspath(".."))  # in tutorial directly

# OppAccessor as extension to a Pandas Data Frame
# Access the object by using df.opp
#   opp.attr     : dictionary of simulation paramters and metadata for scalars and vector data
#   opp.plot     : simple plot methods for single vector values
#   opp.filter() : data frame filter with method chaining
#   opp.tex      : misc methods for latex ouput (e.g. attribute tables, ...)


def simple_plots():
    scavetool = ScaveTool()  # use default Config. See tutorial_01 on setup.
    df = scavetool.load_df_from_scave(
        input_paths=[
            os.path.join(path2data, "tutorial_01*.vec")
        ],  # ! ONLY vectors this time !
        scave_filter='module("LTE_d2d_Vadere.eNB.lteNic.channelModel")',
        recursive=True,
    )
    eNB_vec1 = df.iloc[0]

    fig, axes = plt.subplots(nrows=2)
    ax_time = axes[0]
    df.opp.plot.create_histogram(ax_time, eNB_vec1)
    ax_time.legend()

    ax_hist = axes[1]
    df.opp.plot.create_time_series(ax_hist, eNB_vec1)
    fig.show()


def dataframe_filter_usage():
    """
    filter Data Frame. All chained filters are additive. Example below will select the same
    data as simple_plots()
    """
    scavetool = ScaveTool()
    df = scavetool.load_csv(os.path.join(path2data, "tutorial_01_result.csv"))
    df = df.opp.filter().vector().module_regex(".*adere\.eNB\.lteNic\.channe.*").apply()
    print(df.head())


def tex_exports():
    scavetool = ScaveTool()
    df = scavetool.load_csv(os.path.join(path2data, "tutorial_01_result.csv"))
    # just use the first row (df.iloc[0]) to get a run identifier
    print(df.opp.tex.create_attribute_tabular(df.iloc[0]["run"]))


if __name__ == "__main__":
    simple_plots()
# dataframe_filter_usage()
# tex_exports()
