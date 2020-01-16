#%%
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

import oppanalyzer.rover_analysis as ropp

# r = ropp.load_df('sim001/test-data/mf_D2D_1to2_Vadere_00/foo1.sca', override_existing_csv=False)
r2 = ropp.load_df(
    "test-data/mf_D2D_1to2_Vadere_00/foo1.vec", override_existing_csv=False
)
#%%
delay: pd.DataFrame = r2.opp.by_name("alertDelay:vector").iloc[4:]

fig, ax = plt.subplots()
for idx, (_, s) in enumerate(delay.iterrows()):
    r2.opp.set_labels(ax, s)
    lbl = ropp.create_label(s["module"], ["LTE_d2d_Vadere.", ".app[0]"])
    ax.plot(s["vectime"], s["vecvalue"], **ropp.plt_args(idx, label=lbl))

ax.set_xlim(left=10.0, right=11.0)
ax.set_yscale("log")
ax.legend()

fig.show()

#%%

rcvd_snir: pd.DataFrame = r2.opp.by_name_module(
    "rcvdSinr:vector", "LTE_d2d_Vadere.eNB.lteNic.channelModel"
).iloc[0]

fig, ax = plt.subplots()
ax.plot(rcvd_snir["vectime"], rcvd_snir["vecvalue"], **ropp.plt_args(label="rcvdSinr"))
r2.opp.set_labels(ax, rcvd_snir)
# ax.set_ylim(bottom=20, top=30)
# ax.set_xlim(left=10.0, right=11.0)
# ax.set_yscale('log')
ax.legend()

fig.show()
