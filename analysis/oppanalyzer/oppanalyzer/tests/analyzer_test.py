import matplotlib.pyplot as plt
from oppanalyzer import Config, ScaveTool

cnf = Config()
scave = ScaveTool()
csv = scave.create_or_get_csv_file(
    csv_path=f"{cnf.rover_main}/analysis/oppanalyzer/oppanalyzer/tests/test-data-0.csv",
    input_paths=[
        f"{cnf.rover_main}/analysis/oppanalyzer/oppanalyzer/tests/*.sca",
        f"{cnf.rover_main}/analysis/oppanalyzer/oppanalyzer/tests/*.vec",
    ],
    scave_filter=None,
    override=False,
    recursive=True,
)
df = scave.load_csv(csv)

#%% Create time series and histogram of receive SINR at the eNB

sinr_enb = df.opp.filter().vector().name_regex("rcvdSinr.*").apply().iloc[0]

fig_snir, axes = plt.subplots(nrows=2)
ax_time = axes[0]
df.opp.plot.create_time_series(ax_time, sinr_enb)
ax_time.legend()

ax_hist = axes[1]
df.opp.plot.create_histogram(ax_hist, sinr_enb)
fig_snir.show()
