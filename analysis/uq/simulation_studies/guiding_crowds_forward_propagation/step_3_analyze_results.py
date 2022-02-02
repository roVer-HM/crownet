import matplotlib.pyplot as plt
import pandas as pd
import statsmodels.api as sm
from scipy import stats
import numpy as np

from statsmodels.formula.api import ols
import os
from statsmodels.multivariate.manova import MANOVA

reaction_prob_key = "('Parameter', 'vadere', 'reactionProbabilities.[stimulusId==-400].reactionProbability')"
time_step_key = "timeStep"
seed_key_key = "('Parameter', 'vadere', 'fixedSeed')"
wall_clock_time_key = "('MetaInfo', 'required_wallclock_time')"
return_code_key = "('MetaInfo', 'return_code')"
simulation_time = 'Simulation time'
corridors = {'areaDensityCountingNormed-PID14': "Corridor1",
             'areaDensityCountingNormed-PID16': "Corridor2",
             'areaDensityCountingNormed-PID18': "Corridor3"}
velocities_dict = {'velocity-PID25': "Corridor1",
             'velocity-PID27': "Corridor2",
             'velocity-PID29': "Corridor3"}
densities_dict = {'density-PID25': "Corridor1",
             'density-PID27': "Corridor2",
             'density-PID29': "Corridor3"}
sim_time_steady_flow_start = 100
sim_time_steady_flow_end = 500
time_step_size = 0.4
var_corridor = "Corridor"
reaction_prob_key_short = "reactionProbability"

def get_fundamental_diagrams(controller_type):
    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_fundamentalDiagramm.csv", index_col=[0, 1])
    densities_closed_loop = c1.join(c2)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= sim_time_steady_flow_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                               inplace=True)
    densities_closed_loop.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)

    velocities_ = velocities_ = densities_closed_loop.drop(columns=list(densities_dict.keys()))
    velocities_.rename(columns=velocities_dict, inplace=True)
    velocities_["Controller"] = controller_type
    velocities_.sort_index(axis=1, inplace=True)

    densities_ = densities_closed_loop.drop(columns=list(velocities_dict.keys()))
    densities_.rename(columns=densities_dict, inplace=True)
    densities_["Controller"] = controller_type
    densities_.sort_index(axis=1, inplace=True)

    if densities_.equals(get_densities(controller_type=controller_type)):
        print(f"{controller_type}: density check successful.")
    else:
        pass
        #raise ValueError("Fundamental diagram densities differ from measured densities.")

    return densities_, velocities_


def get_densities(controller_type):
    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_densities.csv", index_col=[0, 1])
    densities_closed_loop = c1.join(c2)
    densities_closed_loop.rename(columns=corridors, inplace=True)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= sim_time_steady_flow_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                               inplace=True)
    densities_closed_loop.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)
    densities_closed_loop["Controller"] = controller_type
    densities_closed_loop.sort_index(axis=1, inplace=True)
    return densities_closed_loop


def plot_density_distribution(densities, controller):
    densities_closed_loop_mean = densities.groupby(by=[reaction_prob_key_short, simulation_time]).min()
    densities_closed_loop_std = densities.groupby(by=[reaction_prob_key_short, simulation_time]).std()


    for reaction_prob, data in densities_closed_loop_mean.groupby(by=reaction_prob_key_short):
        data.boxplot(showmeans=True, rot=45, fontsize=10)
        #data.plot()
        title = f"Controller type: {controller}. Reaction probability: {reaction_prob}"
        plt.title(title)
        plt.ylabel("Density [ped/m^2] (mean)")
        plt.savefig(os.path.join("figs", f"{title}.png"))
        plt.show()

    for reaction_prob, data in densities_closed_loop_std.groupby(by=reaction_prob_key_short):
        data.boxplot(showmeans=True, rot=45, fontsize=10)
        title = f"Controller type: {controller}. Reaction probability: {reaction_prob}"
        plt.title(title)
        plt.ylabel("Density [ped/m^2] (std)")
        plt.savefig(os.path.join("figs", f"{title}.png"))
        plt.show()

def get_time_controller_wise(controller_type):
    c1 = pd.read_csv(f"{controller_type}_startTime.csv", index_col=[0, 1, 2])
    c2 = pd.read_csv(f"{controller_type}_targetReachTime.csv", index_col=[0, 1, 2])
    times = c1.join(c2)
    travel_times = pd.DataFrame(times["reachTime-PID12"].sub(times["startTime-PID13"]),
                                columns=["travel_time"])
    travel_times["Controller"] = controller_type
    travel_times = travel_times[times["startTime-PID13"] >= sim_time_steady_flow_start]

    travel_times.reset_index(level="pedestrianId", inplace=True, drop=True)

    param = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    travel_times = travel_times.join(param[reaction_prob_key])

    travel_times.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)

    return travel_times

def get_travel_times():

    c1 = get_time_controller_wise(controller_type="NoController")
    c2 = get_time_controller_wise(controller_type="OpenLoop")
    c3 = get_time_controller_wise(controller_type="ClosedLoop")


    travel_times = pd.concat([c1, c2])
    travel_times = pd.concat([travel_times, c3])

    return travel_times

def plot_travel_time(travel_time):

    fig, ax = plt.subplots(nrows=5, ncols=1, figsize=(6, 15))
    counter = 0

    for c in ["OpenLoop","ClosedLoop","NoController"]:
        t_ = travel_time[travel_time["Controller"] == c]

        for reactionProb in [1.0, 0.5]:
            data = t_[t_[reaction_prob_key_short] == reactionProb]

            times = data[data["travel_time"] != np.inf]
            stucked = data[data["travel_time"] == np.inf]
            if len(data) != 172000:
                raise ValueError(c)
            ax[counter].hist(times["travel_time"], range=(0, 400), bins=20)

            finished = len(times)

            title = f"{c}, {reactionProb}. Counts: {finished}."

            m = times["travel_time"].median()
            mad = times["travel_time"].mad()

            ax[counter].axvline(m, color='k', linestyle='dashed', linewidth=1)
            min_ylim, max_ylim = ax[counter].get_ylim()
            ax[counter].text(m * 1.1, max_ylim * 0.9, f'Median: {m:.2f}. Mad: {mad:.2f}')
            ax[counter].set_xlabel("Travel times [s]")

            ax[counter].set_title(title)
            counter +=1
            if counter >= 5:
                break

    fig.tight_layout()
    plt.savefig(f"figs/travel_times.png")
    plt.show()

def get_densities_velocities():
    densities_closed_loop, velocities_closed_loop = get_fundamental_diagrams(controller_type="ClosedLoop")
    densities_open_loop, velocities_open_loop = get_fundamental_diagrams(controller_type="OpenLoop")
    densities_default, velocities_default = get_fundamental_diagrams(controller_type="NoController")

    densities = pd.concat([densities_default, densities_open_loop])
    densities = pd.concat([densities, densities_closed_loop])

    velocities = pd.concat([velocities_default, velocities_open_loop])
    velocities = pd.concat([velocities, velocities_closed_loop])

    return densities, velocities

def get_densities_normed(densities):
    densities_normed = densities.copy()
    densities_normed[list(corridors.values())] = (densities[list(corridors.values())] - 0.31) / (5.4 - 0.31)

    for col in list(corridors.values()):
        densities_normed[col][densities_normed[col] < 0] = 0

    return densities_normed


def get_safety_values(densities_normed):

    medians_ = densities_normed.groupby(by=["Controller", reaction_prob_key_short]).median()
    medians_.drop(columns=[simulation_time], inplace=True)
    variances_ = densities_normed.groupby(by=["Controller", reaction_prob_key_short]).mad()**2
    variances_.drop(columns=[simulation_time], inplace=True)

    gammas = np.arange(0,10)
    df2 = pd.DataFrame()
    df = pd.DataFrame()
    for gamma in gammas:
        x = (-medians_ - gamma*variances_)*100
        x = x.transpose()

        x_mean = x.idxmin()
        x_mean.name = gamma
        x_mean =x_mean.to_frame()

        x_min = pd.DataFrame(x.min(axis=0), columns=[gamma])
        df = pd.concat([df, x_mean], axis=1)
        df2 = pd.concat([df2, x_min], axis=1)
    df2 = df2.transpose()
    df2.index.name = "gamma"

    df = df.transpose()
    df.index.name = "gamma"

    return df, df2

def get_flux(velocities, densities):
    flux = velocities.copy()
    flux[list(corridors.values())] = velocities[list(corridors.values())].multiply(densities[list(corridors.values())])
    return flux

if __name__ == "__main__":

    travel_time = get_travel_times()
    plot_travel_time(travel_time)

    densities, velocities = get_densities_velocities()
    flux = get_flux(velocities, densities)
    densities_normed = get_densities_normed(densities)
    safety_values_min_corridors, safety_values_min = get_safety_values(densities_normed)


    # write data
    output_dir = os.path.abspath("tables")
    file_name = os.path.join((output_dir), "safety_value_gamma_1.0.csv")
    safety_values_min.iloc[1,:].to_csv(file_name)

    file_name = os.path.join((output_dir), "safety_value_min_corridor.csv")
    safety_values_min_corridors.to_csv(file_name)
    #densities_mean = densities.groupby(["Controller", reaction_prob_key_short]).median()
    #densities_mean.drop(columns=[simulation_time], inplace=True)
    #densities_std = densities.groupby(["Controller", reaction_prob_key_short]).mad()
    #densities_std.drop(columns=[simulation_time], inplace=True)

    #velocities_mean = velocities.groupby(["Controller", reaction_prob_key_short]).mean()
    #velocities_mean.drop(columns=[simulation_time], inplace=True)
    #velocities_std = velocities.groupby(["Controller", reaction_prob_key_short]).std()
    #velocities_std.drop(columns=[simulation_time], inplace=True)


    # generate plots

    safety_values_min.plot()
    plt.ylim(-50, 0)
    plt.xlabel("Weighting parameter gamma xx")
    title = "Safety value (minimum)"
    plt.ylabel(title)
    plt.savefig(f"figs/{title}.png")
    plt.show()

    fig, ax = plt.subplots(nrows=5, ncols=len(corridors.values()), figsize=(15, 15))
    ii = 0
    for controller in ["OpenLoop","ClosedLoop","NoController"]:

        v_ = velocities[velocities["Controller"] == controller]
        d_ = densities_normed[densities_normed["Controller"] == controller]

        for reactionProb in [1.0, 0.5]:
            v__ = v_[v_[reaction_prob_key_short] == reactionProb]
            d__ = d_[d_[reaction_prob_key_short] == reactionProb]

            title_ = f'{controller}, r={reactionProb}'

            iii = 0
            for c in list(corridors.values()):
                ax[ii, iii].hist(d__[c], label=c, bins=20, range = (0, 0.5))

                m = d__[c].median()
                mad = d__[c].mad()
                ax[ii, iii].axvline(m, color='k', linestyle='dashed', linewidth=1)
                min_ylim, max_ylim = ax[ii, iii].get_ylim()
                ax[ii, iii].text(m * 1.1, max_ylim * 0.9, f'Median: {m:.2f}. Mad: {mad:.2f}')

                ax[ii, iii].set_xlabel("Densities normed")
                ax[ii, iii].set_title(title_)
                ax[ii, iii].legend()
                iii += 1
            if ii%2==0:
                ii += 1
            if ii >= 5:
                break
        ii+=1
        if ii >= 5:
            break

    fig.tight_layout()
    plt.savefig(f"figs/densitiy_measure.png")
    plt.show()


    fig, ax = plt.subplots(nrows=5, ncols=len(corridors.values()), figsize=(15, 15))
    ii = 0
    for controller in ["OpenLoop","ClosedLoop","NoController"]:

        v_ = velocities[velocities["Controller"] == controller]
        d_ = densities[densities["Controller"] == controller]

        for reactionProb in [1.0, 0.5]:
            v__ = v_[v_[reaction_prob_key_short] == reactionProb]
            d__ = d_[d_[reaction_prob_key_short] == reactionProb]

            title_ = f'{controller}, r={reactionProb}'

            iii = 0
            for c in list(corridors.values()):

                ax[ii,iii].hist(v__[c], range=(0,2), label=c, bins=20)

                m = v__[c].median()
                mad = v__[c].mad()
                ax[ii, iii].axvline(m, color='k', linestyle='dashed', linewidth=1)
                min_ylim, max_ylim = ax[ii, iii].get_ylim()
                ax[ii, iii].text(m * 1.1, max_ylim * 0.9, f'Median: {m:.2f}. Mad: {mad:.2f}')

                ax[ii,iii].set_xlabel("Velocity [m/s]")
                ax[ii,iii].set_title(title_)
                ax[ii, iii].legend()
                iii+=1
            if ii%2==0:
                ii += 1
            if ii >= 5:
                break
        ii+=1
        if ii >= 5:
            break

    fig.tight_layout()
    plt.savefig(f"figs/velocities.png")
    plt.show()

    fig, ax = plt.subplots(nrows=5, ncols=len(corridors.values()), figsize=(15, 15))
    ii = 0
    for controller in ["OpenLoop","ClosedLoop","NoController"]:

        v_ = velocities[velocities["Controller"] == controller]
        d_ = densities[densities["Controller"] == controller]

        for reactionProb in [1.0, 0.5]:
            v__ = v_[v_[reaction_prob_key_short] == reactionProb]
            d__ = d_[d_[reaction_prob_key_short] == reactionProb]

            title_ = f'{controller}, r={reactionProb}'

            iii = 0
            for c in list(corridors.values()):
                ax[ii,iii].hist(d__[c], range=(0,2.5), label=c, bins=20)

                m = d__[c].median()
                mad = d__[c].mad()
                ax[ii, iii].axvline(m, color='k', linestyle='dashed', linewidth=1)
                min_ylim, max_ylim = ax[ii, iii].get_ylim()
                ax[ii, iii].text(m * 1.1, max_ylim * 0.9, f'Median: {m:.2f}. Mad: {mad:.2f}')

                ax[ii,iii].set_xlabel("Density [ped/m**2]")
                ax[ii,iii].set_title(title_)
                ax[ii, iii].legend()
                iii+=1
            if ii%2==0:
                ii += 1
            if ii >= 5:
                break
        ii+=1
        if ii >= 5:
            break

    fig.tight_layout()
    plt.savefig(f"figs/densities.png")
    plt.show()

    fig, ax = plt.subplots(nrows=5, ncols=len(corridors.values()), figsize=(15, 15))
    ii = 0
    for controller in ["OpenLoop", "ClosedLoop", "NoController"]:

        v_ = velocities[velocities["Controller"] == controller]
        d_ = densities[densities["Controller"] == controller]

        for reactionProb in [1.0, 0.5]:
            v__ = v_[v_[reaction_prob_key_short] == reactionProb]
            d__ = d_[d_[reaction_prob_key_short] == reactionProb]

            title_ = f'{controller}, r={reactionProb}'

            iii = 0
            for c in list(corridors.values()):

                ax[ii, iii].scatter(x=d__[c], y=v__[c], label=c)
                ax[ii, iii].set_xlabel("Density [ped/m**2]")
                ax[ii, iii].set_ylabel("Velocity [m/s]")
                ax[ii, iii].set_xlim(0,2.3)
                ax[ii, iii].set_ylim(0,2.2)

                ax[ii, iii].set_title(title_)
                ax[ii, iii].legend()
                iii += 1
            if ii % 2 == 0:
                ii += 1
            if ii >= 5:
                break
        ii += 1
        if ii >= 5:
            break

    fig.tight_layout()
    plt.savefig(f"figs/fundamentalDiagram.png")
    plt.show()
    print("Finished plotting.")




