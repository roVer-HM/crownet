import os
import shutil
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats
from sklearn.linear_model import LinearRegression
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import PolynomialFeatures

reaction_prob_key = "('Parameter', 'dummy', 'Compliance')"
time_step_key = "timeStep"
seed_key_key = "('Parameter', 'vadere', 'fixedSeed')"
wall_clock_time_key = "('MetaInfo', 'required_wallclock_time')"
return_code_key = "('MetaInfo', 'return_code')"
simulation_time = 'Simulation time'
corridors = {'areaDensityCountingNormed-PID101': "Corridor1",
             'areaDensityCountingNormed-PID102': "Corridor2",
             'areaDensityCountingNormed-PID103': "Corridor3"}
velocities_dict = {'velocity-PID25': "Corridor1",
                   'velocity-PID27': "Corridor2",
                   'velocity-PID29': "Corridor3"}
densities_dict = {'density-PID25': "Corridor1",
                  'density-PID27': "Corridor2",
                  'density-PID29': "Corridor3"}
c__ = {11: "Short",
       31: "Medium",
       51: "Long",
       'Corridor1': "Short",
       'Corridor2': "Medium",
       'Corridor3': "Long",
       }

args_boxplot = dict(flierprops=dict(marker='+', markerfacecolor='#AAAAAA', markersize=3,
                                    linestyle='none', markeredgecolor='#AAAAAA'),
                    boxprops=dict(color='#464646'),
                    whiskerprops=dict(linewidth=1),
                    medianprops=dict(linewidth=2.5, color='k'),
                    notch=True)

controller__ = {"OpenLoop": "Fixed order strategy",
                "ClosedLoop": "Minimal density strategy",
                "AvoidShort": "Simple density strategy"}
sim_time_steady_flow_start = 250
sim_time_steady_flow_end = 1250
time_step_size = 0.4
var_corridor = "Corridor"
reaction_prob_key_short = "reactionProbability"
compliance_rate = "Compliance rate c"


def get_fundamental_diagrams(controller_type, time_start=None, simulation_dir=None):
    if time_start is None:
        time_start = sim_time_steady_flow_start

    c1 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_parameters.csv"), index_col=[0, 1])
    c2 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_fundamentalDiagramm.csv"), index_col=[0, 1])
    densities_closed_loop = c1.join(c2)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= time_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                               inplace=True)
    densities_closed_loop.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)

    velocities_ = densities_closed_loop.drop(columns=list(densities_dict.keys()))
    velocities_.rename(columns=velocities_dict, inplace=True)
    velocities_["Controller"] = controller_type
    velocities_.sort_index(axis=1, inplace=True)

    densities_ = densities_closed_loop.drop(columns=list(velocities_dict.keys()))
    densities_.rename(columns=densities_dict, inplace=True)
    densities_["Controller"] = controller_type
    densities_.sort_index(axis=1, inplace=True)

    densities_2 = get_densities(controller_type=controller_type, time_start=time_start, simulation_dir=simulation_dir)

    if densities_.equals(densities_2):
        print(f"{controller_type}: density check successful.")
    else:
        raise ValueError("Fundamental diagram densities differ from measured densities.")

    return densities_, velocities_


def get_densities(controller_type, time_start, simulation_dir):
    c1 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_parameters.csv"), index_col=[0, 1])
    c2 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_densities.csv"), index_col=[0, 1])
    densities_closed_loop = c1.join(c2)
    densities_closed_loop.rename(columns=corridors, inplace=True)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= time_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                               inplace=True)
    densities_closed_loop.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)
    densities_closed_loop["Controller"] = controller_type
    densities_closed_loop.sort_index(axis=1, inplace=True)
    return densities_closed_loop


def get_time_controller_wise(controller_type, simulation_dir):
    c1 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_startTime.csv"), index_col=[0, 1, 2])
    c2 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_targetReachTime.csv"), index_col=[0, 1, 2])
    times = c1.join(c2)
    travel_times = pd.DataFrame(times["reachTime-PID12"].sub(times["startTime-PID13"]),
                                columns=["travel_time"])
    travel_times["Controller"] = controller_type
    travel_times = travel_times[times["startTime-PID13"] >= sim_time_steady_flow_start]

    travel_times.reset_index(level="pedestrianId", inplace=True, drop=True)

    param = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_parameters.csv"), index_col=[0, 1])
    travel_times = travel_times.join(param[reaction_prob_key])

    travel_times.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)

    return travel_times


def get_travel_times(simulation_dir):
    c2 = get_time_controller_wise(controller_type="OpenLoop", simulation_dir=simulation_dir)
    c3 = get_time_controller_wise(controller_type="ClosedLoop", simulation_dir=simulation_dir)
    c4 = get_time_controller_wise(controller_type="AvoidShort", simulation_dir=simulation_dir)
    travel_times = pd.concat([c2, c3, c4])
    return travel_times


def plot_travel_time(travel_time, output_dir):
    travel_time = travel_time[travel_time["travel_time"] != np.inf]

    for c, data in travel_time.groupby(by=["Controller"]):
        controller_type = controller__[c[0]]
        data.drop(columns="Controller", inplace=True)
        data.reset_index(inplace=True, drop=True, )
        data[reaction_prob_key_short] = data[reaction_prob_key_short].round(3)
        data = data.pivot(columns=["reactionProbability"])
        data.columns = data.columns.droplevel()
        data.rename(columns=controller__, inplace=True)
        data.boxplot(**args_boxplot)
        plt.ylim()
        plt.xlabel("Parameter c")
        plt.ylim(0, 1000)
        plt.title(controller_type)
        plt.xticks(np.arange(1, 45, 4), [f"{x:.1f}" for x in np.linspace(0, 1, 11)])
        plt.ylabel("Travel time [s]")
        plt.xlabel(compliance_rate)
        plt.savefig(os.path.join(output_dir, f"travel_time_{controller_type}.pdf"))
        plt.show(block=False)
        plt.close()

    fig, ax = plt.subplots(nrows=1, ncols=3, figsize=(15, 5), sharey=True)
    fig.subplots_adjust(wspace=0.1, hspace=0)

    time_25 = travel_time.groupby(by=["Controller", reaction_prob_key_short]).quantile(0.25).unstack(level=0)
    time_25.columns = time_25.columns.droplevel(0)
    time_25.rename(columns=controller__, inplace=True)
    plt.sca(ax[0])
    ax[0].hlines(y=time_25.max().max(), xmin=0, xmax=1, colors=["k"])
    time_25.plot(marker="o", ax=ax[0])
    plt.legend()
    ax[0].set_title("25% Quartile")
    ax[0].set_ylim(25, 250)
    ax[0].legend()
    ax[0].set_ylabel("Travel time [s]")
    ax[0].set_xlabel(compliance_rate)

    time_med = travel_time.groupby(by=["Controller", reaction_prob_key_short]).median().unstack(level=0)
    time_med.columns = time_med.columns.droplevel(0)
    time_med.rename(columns=controller__, inplace=True)
    plt.sca(ax[1])
    ax[1].hlines(y=time_med.max().max(), xmin=0, xmax=1, colors=["k"])
    time_med.plot(marker="o", ax=ax[1])
    ax[1].set_title("Median")
    ax[1].legend()
    ax[1].set_xlabel(compliance_rate)

    time_75 = travel_time.groupby(by=["Controller", reaction_prob_key_short]).quantile(0.75).unstack(level=0)
    time_75.columns = time_75.columns.droplevel(0)
    time_75.rename(columns=controller__, inplace=True)
    plt.sca(ax[2])
    ax[2].hlines(y=time_75.max().max(), xmin=0, xmax=1, colors=["k"])
    time_75.plot(marker="o", ax=ax[2])
    ax[2].set_title("75% Quartile")
    ax[2].legend()
    ax[2].set_xlabel(compliance_rate)

    plt.savefig(os.path.join(output_dir, "Travel_time.pdf"))
    plt.show(block=False)
    plt.close()
    print("Plotted travel_time.pdf")


def plot_hists_corridor1(output_dir, simulation_dir):
    densities, velocities = get_fundamental_diagrams(controller_type="NoController", time_start=0,
                                                     simulation_dir=simulation_dir)

    for c in ["Corridor1"]:
        velocities[c][densities[c] == 0] = np.nan

    velocities.dropna(inplace=True)

    densities[simulation_time] = densities[simulation_time].round(2)
    densities.reset_index(inplace=True)
    densities.set_index(["id", simulation_time], inplace=True)

    for ii, data in densities["Corridor1"].groupby(by=["id"]):
        data.index = data.index.droplevel(0)
        data.plot(label=f"Simulation run {ii[0] + 1}")  # bins=20, range=(0, 0.5))

    plt.legend()
    plt.xlabel("Simulation time [s]")
    plt.ylabel("Density [$ped/m^{2}$]")
    plt.title("Short corridor (no rerouting)")
    plt.savefig(os.path.join(output_dir, "DensityOverTime.pdf"))
    plt.show(block=False)
    plt.close()

    velocities[simulation_time] = velocities[simulation_time].round(2)
    velocities.reset_index(inplace=True)
    velocities.set_index(["id", simulation_time], inplace=True)

    for ii, data in velocities["Corridor1"].groupby(by=["id"]):
        data.index = data.index.droplevel(0)
        data.plot(label=f"Simulation run {ii[0] + 1}")  # bins=20, range=(0, 0.5))

    plt.legend()
    plt.xlabel("Simulation time [s]")
    plt.ylabel("Velocity [$m/s$]")
    plt.title("Short corridor (no rerouting)")
    plt.savefig(os.path.join(output_dir, "VelocityOverTime.pdf"))
    plt.show(block=False)
    plt.close()

    densities, velocities = get_fundamental_diagrams(controller_type="NoController", simulation_dir=simulation_dir)
    times = get_time_controller_wise(controller_type="NoController", simulation_dir=simulation_dir)

    df = pd.concat([densities["Corridor1"].describe(), velocities["Corridor1"].describe()], axis=1)

    df["t"] = times[times["travel_time"] != np.inf].describe()["travel_time"]

    df.columns = ["Density $ped/m**2$", "Velocity $m/s$", "Travel time [s]"]
    df = df.transpose()
    df.to_latex(os.path.join(output_dir, "NoControlCorridor1.tex"), float_format="%.2f")


def plot_fundamental_diagram_corridor1(output_dir, simulation_dir):
    densities, velocities = get_fundamental_diagrams(controller_type="NoController", time_start=0.0,
                                                     simulation_dir=simulation_dir)

    for c in ["Corridor1"]:
        velocities[c][densities[c] == 0] = np.nan
        densities[c][densities[c] == 0] = np.nan

    densities.dropna(inplace=True)
    velocities.dropna(inplace=True)

    flux = densities["Corridor1"] * velocities["Corridor1"]

    degree = 2
    polyreg = make_pipeline(PolynomialFeatures(degree), LinearRegression())
    polyreg.fit(densities["Corridor1"].values.reshape(-1, 1), flux.values.reshape(-1, 1))

    plt.scatter(densities["Corridor1"], flux, label="Simulation data")
    d__ = np.linspace(0, 2.1)
    flow_reg = polyreg.predict(d__.reshape(-1, 1))
    plt.plot(d__, flow_reg.ravel(), color="black", label="Regression (poly.)")

    d___ = np.linspace(2.1, 3.2)
    flow_reg_ = polyreg.predict(d___.reshape(-1, 1))
    plt.plot(d___, flow_reg_.ravel(), color="black", linestyle="--", label=None)

    plt.scatter(2.1, flow_reg.max(), marker="D", c="black", label="$J_{1,max}$ (Regression)")
    plt.text(2.1 + 0.1, flow_reg.max() + 0.05, f'{flow_reg.max():.2f} (d = 2.1 $ped/m^{2}$)')

    densitiy_max_flux = densities.iloc[np.where(flux == flux.max())[0][0], :]["Corridor1"]
    plt.scatter(densitiy_max_flux, flux.max(), marker="s", c="black", label="$J_{1,max}$ (Data)")
    plt.text(densitiy_max_flux + 0.1, flux.max() + 0.0, f'{flux.max():.2f} (d = {densitiy_max_flux:.2f} $ped/m^{2}$)')

    plt.xlabel("Density [ped/m²]")
    plt.ylabel("Specific flow [1/(ms)]")
    plt.xlim(-0.1, 3.5)
    plt.title(f"Fundamental diagram: short corridor")
    plt.legend()
    plt.savefig(os.path.join(output_dir, f"fundamentalDiagramShortCorridor1.pdf"))
    plt.show(block=False)
    plt.close()

    plt.scatter(densities["Corridor1"], velocities["Corridor1"], label="Simulation data")
    plt.xlabel("Density [$ped/m^{2}$]")
    plt.ylabel("Velocity [m/s]")
    plt.title(f"Fundamental diagram: short corridor")
    plt.xlim(-0.1, 3.5)
    plt.legend()
    plt.savefig(os.path.join(output_dir, f"fundamentalDiagramShortCorridor2.pdf"))
    plt.show(block=False)
    plt.close()


def get_densities_velocities(simulation_dir):
    densities_closed_loop, velocities_closed_loop = get_fundamental_diagrams(controller_type="ClosedLoop",
                                                                             simulation_dir=simulation_dir)
    densities_open_loop, velocities_open_loop = get_fundamental_diagrams(controller_type="OpenLoop",
                                                                         simulation_dir=simulation_dir)
    densities_avoid_short, velocities_avoid_short = get_fundamental_diagrams(controller_type="AvoidShort",
                                                                             simulation_dir=simulation_dir)

    densities = pd.concat([densities_closed_loop, densities_open_loop, densities_avoid_short])
    velocities = pd.concat([velocities_closed_loop, velocities_open_loop, velocities_avoid_short])

    for c in corridors.values():
        velocities[c][densities[c] == 0] = np.nan

    return densities, velocities


def get_path_choice(controller_type, simulation_dir):
    c1 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_parameters.csv"), index_col=[0, 1])
    c2 = pd.read_csv(os.path.join(simulation_dir, f"{controller_type}_path_choice.csv"), index_col=[0, 1])
    path_choice = c1.join(c2)

    path_choice[simulation_time] = path_choice[time_step_key] * time_step_size
    path_choice = path_choice[path_choice[simulation_time] >= sim_time_steady_flow_start]
    path_choice = path_choice[path_choice[simulation_time] < sim_time_steady_flow_end]
    path_choice.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                     inplace=True)
    path_choice.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)
    path_choice["Controller"] = controller_type
    path_choice.sort_index(axis=1, inplace=True)

    path_choice.reset_index(inplace=True)
    path_choice.drop(columns=["run_id", "id"], inplace=True)
    path_choice.set_index(simulation_time, inplace=True)
    path_choice[reaction_prob_key_short] = path_choice[reaction_prob_key_short].round(3)
    return path_choice


def plot_quantity(densities, file_name, y_min=0.0, y_max=2.5, ylabel="Density [ped/m*2]", output_dir=None):
    fig, ax = plt.subplots(nrows=3, ncols=3, figsize=(11, 11))
    fig.subplots_adjust(wspace=0.05, hspace=0.3)

    i = 0
    c_order = list()
    for c, data in densities.groupby(by=["Controller"]):  # oxplot(column="travel_time"):
        c = c[0]
        c_order.append(controller__[c])

        densities_ = data
        densities_[reaction_prob_key_short] = densities_[reaction_prob_key_short].round(3)
        densities_ = densities_.reset_index(drop=True).drop(columns=[simulation_time, "Controller"]).pivot(
            columns=[reaction_prob_key_short])

        ii = 0
        for corridor_ in ["Corridor1", "Corridor2", "Corridor3"]:
            densities__ = densities_.xs(corridor_, axis=1)
            plt.sca(ax[ii, i])
            densities__.boxplot(**args_boxplot)
            ax[ii, i].set_title(f"{c__[corridor_]} corridor")
            ax[ii, i].set_ylim(y_min, y_max)
            ax[ii, i].set_xlabel(compliance_rate)
            if i == 0:
                ax[ii, i].set_ylabel(ylabel)
            else:
                ax[ii, i].set_yticklabels([])
            ax[ii, i].set_xticks(np.arange(1, 45, 4))
            ax[ii, i].set_xticklabels([f"{x:.1f}" for x in np.linspace(0, 1, 11)])

            ii += 1
        i += 1

    fig.suptitle("                  ".join(c_order), fontsize=16, x=0.47)
    plt.savefig(os.path.join(output_dir, f"{file_name}.pdf"))
    plt.show(block=False)
    plt.close()


def plot_write_path_choice(path_choice, output_dir):
    counts_ = path_choice.groupby(by=["Controller", "corridorRecommended"]).count()
    counts = pd.DataFrame(counts_.iloc[:, 0])
    counts.columns = ['Counts']
    counts.reset_index(inplace=True)
    counts['Controller'] = counts['Controller'].replace(controller__)

    counts = counts[counts["corridorRecommended"] != "none"]
    counts["corridorRecommended"] = counts["corridorRecommended"].astype(int)

    counts = pd.pivot_table(counts, index='Controller', columns='corridorRecommended', values='Counts', fill_value=0)
    counts = counts.transpose()

    counts.to_latex(os.path.join(output_dir, "NumberOfRouteRecommensations.tex"), float_format="%.2f")

    plt.bar(counts.columns.values, counts.loc[11].values, color='r')
    plt.bar(counts.columns.values, counts.loc[21].values, bottom=counts.loc[11].values, color='b')
    plt.bar(counts.columns.values, counts.loc[31].values, bottom=counts.loc[11].values + counts.loc[21].values,
            color='g')

    plt.xlabel("Guiding strategy")
    plt.ylabel("Number of recommendations")
    plt.legend(labels=["Short route",  ##11
                       "Medium route",  ##21
                       "Long route"])  ##31

    plt.savefig(os.path.join(output_dir, "NumberOfRouteRecommendations.pdf"))
    plt.show(block=False)
    plt.close()


def compare_densities_statistics(densities, output_dir):
    densities = densities.reset_index()
    densities = densities[["id", "Controller", "Corridor1", "Corridor2", "Corridor3"]]

    densities['Controller'] = densities['Controller'].replace(controller__)
    densities = densities.groupby(by=["Controller", "id"])

    densities_max = densities.max().reset_index().set_index("id")
    densities_max = densities_max[["Controller", "Corridor1"]]
    densities_max = densities_max.pivot(columns="Controller")

    densities_max = densities_max.droplevel(0, axis=1)

    densities_max.boxplot()
    plt.ylabel("Density [ped/m*2]")
    plt.savefig(os.path.join(output_dir, "maxDensitities.pdf"))
    plt.show(block=False)
    plt.close()

    results_normality = ""
    for col in densities_max.columns:
        dens = densities_max[[col]]
        # not normally distributed, if p < 0.05
        s_1 = stats.kstest(dens.values, "norm")
        results_normality += f"normality check: Controller {col}: {s_1}. \n"

    # As the p-value is not less than 0.05, we cannot reject the null hypothesis that the median density is
    # the same for all three groups. Hence, We don’t have sufficient proof to claim that different controllers lead to statistically significant densities.
    kruskal = stats.kruskal(densities_max.iloc[:, 0], densities_max.iloc[:, 1], densities_max.iloc[:, 2])
    print(kruskal)

    with open(os.path.join(output_dir, 'statistical_test.txt'), 'w+') as f:
        f.write(f"Effect of controller: max densitiy (41 samples, 41 compliance rates)")
        f.write(f" not normally distributed, if p < 0.05 \n")
        f.write(results_normality)
        f.write(f"Kruskal Wallis test: densities. no difference between groups, if  p > 0.05.\n")
        f.write(f"{kruskal}")


def post_processing(simulation_dir):
    if os.path.isdir(simulation_dir) == False:
        raise NotADirectoryError(f"Could not find simulation dir {simulation_dir}. "
                                 f"Please check if you use the same name as defined in run_simulations.py")

    output_dir = os.path.join(simulation_dir, "analysis_guiding_strategies")

    # post processing of results
    if os.path.isdir(output_dir):
        print(f"Remove {output_dir}")
        shutil.rmtree(output_dir, ignore_errors=True)
    print(f"Write results to {output_dir}")
    os.makedirs(output_dir)

    travel_time = get_travel_times(simulation_dir=simulation_dir)
    plot_travel_time(travel_time, output_dir=output_dir)
    plot_hists_corridor1(output_dir, simulation_dir=simulation_dir)

    plot_fundamental_diagram_corridor1(output_dir, simulation_dir=simulation_dir)

    path_choice = pd.concat([get_path_choice("OpenLoop", simulation_dir),
                             get_path_choice("ClosedLoop", simulation_dir),
                             get_path_choice("AvoidShort", simulation_dir)], axis=0)
    plot_write_path_choice(path_choice=path_choice, output_dir=output_dir)

    densities, velocities = get_densities_velocities(simulation_dir=simulation_dir)
    compare_densities_statistics(densities, output_dir)
    plot_quantity(densities, "densities", y_min=-0.1, y_max=2.2, ylabel="Density [$ped/m^2$]", output_dir=output_dir)
    plot_quantity(velocities, "velocities", y_min=-0.1, y_max=2.2, ylabel="Velocity [$m/s$]", output_dir=output_dir)


if __name__ == "__main__":
    simulation_dir = "/sim_dir"  # TODO specify
    post_processing(simulation_dir)
    sys.exit(0)
