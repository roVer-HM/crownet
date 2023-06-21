#!/usr/bin/env python3
import os
import json
import matplotlib.pyplot as plt
import numpy as np

FILES = [
    "RunStudyVeloDifference_temp_vamsps.json",
    "RunStudyVeloDifferenceVadereGroups_temp_vamsps.json"
]
LABELS = [
    "Individual Mobility",
    "Group Mobility"
]
COLORS = [
    "black",
    "blue"
]
YLABEL = "VAMs / second"
PARAMETER_NAME = "maxClusterVelocityDifference"

def visualize():
    fig = plt.figure(figsize=(10, 4))
    ax = plt.gca()

    ax.grid(True, axis="both", linestyle="--")

    for i in range(len(FILES)):
        with open(FILES[i], "r") as f:
            data = json.load(f)

            parameters = sorted([k for k in data.keys() if len(data[k])])

            ax.errorbar(
                parameters,
                [np.mean(data[p]) for p in parameters],
                [np.std(data[p]) for p in parameters],
                color=COLORS[i],
                ecolor="gray",
                # linestyle='None',
                marker='o',
                capsize=5,
                label=LABELS[i]
            )

    ax.legend()
    
    for tick in ax.get_xticklabels():
        tick.set_rotation(45)
    
    plt.ylabel(YLABEL, size=18)
    plt.xlabel(f"{PARAMETER_NAME} parameter", size=18)
    fig.tight_layout()
    
    fig.savefig(f"fig/multi.png")    
    fig.savefig(f"fig/multi.svg")
    fig.savefig(f"fig/multi.pdf")
    plt.show()


if __name__ == '__main__':
    visualize()
    
