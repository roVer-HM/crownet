#!/usr/bin/env python3

from glob import glob
from itertools import chain, repeat
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.ticker import MaxNLocator

matplotlib.rcParams["pdf.fonttype"] = 42
matplotlib.rcParams["ps.fonttype"] = 42
matplotlib.rcParams["pgf.texsystem"] = "pdflatex"
matplotlib.rcParams.update(
    {
        "font.family": "serif",
        "font.size": 11,
        "axes.labelsize": 8,
        "axes.titlesize": 8,
        "legend.fontsize": 8,
        "figure.titlesize": 11,
        "xtick.labelsize": 8,
        "ytick.labelsize": 8,
    }
)
matplotlib.rcParams["text.usetex"] = True


def main():

    out_path = os.path.join(os.path.dirname(__file__), "rank.pdf")

    with PdfPages(out_path) as pdf:
        for alpha in ["0", "0.25", "0.5", "0.75", "1"]:
            fig, axes = plt.subplots(
                nrows=2,
                ncols=3,
                sharey="all",
                subplot_kw=dict(aspect="equal"),
                # gridspec_kw={
                #     "width_ratios": list(repeat(1/3, 3)),
                #     "height_ratios": list(repeat(.1, 2))
                # }
            )
            fig.suptitle(f"Rank value for $\\alpha={alpha}$")
            axes = list(chain(*axes))
            axes[-1].set_axis_off()
            for idx, stepDist in enumerate(["0", "50", "100", "150", "200"]):
                in_path = os.path.join(
                    os.path.dirname(__file__),
                    f"rank_alpha{alpha}_stepDist{stepDist}.csv",
                )
                ax = axes[idx]
                data = pd.read_csv(in_path, sep=";", skipinitialspace=True)[
                    ["age", "dist", "rank"]
                ]
                data = data.sort_values(["age", "dist"])
                X = data["age"].unique()
                X.sort()
                Y = data["dist"].unique()
                Y.sort()
                Z = data["rank"].to_numpy().reshape((len(X), len(Y)))
                x, y = np.meshgrid(Y, X)

                cp = ax.contour(x, y, Z, levels=10)
                ax.clabel(cp, inline=True, fontsize=8, fmt="%1.1e")
                ax.xaxis.set_major_locator(MaxNLocator(5))
                ax.set_title(f"step distance={stepDist}")
                ax.set_xlabel("distance [m]")
                ax.set_ylabel("age [s]")
                ax.set_ylim(0, 500)
                ax.set_xlim(0, 500)
                # ax.set_aspect("equal")
            fig.tight_layout()
            pdf.savefig(fig)
    print("done")


if __name__ == "__main__":
    main()
