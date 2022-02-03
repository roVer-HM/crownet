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

probs= np.linspace(0, 1.0, 11)

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

    travel_time = travel_time[travel_time["travel_time"] != np.inf]


    for c, data in travel_time.groupby(by=["Controller"]): #oxplot(column="travel_time"):
        data.drop(columns="Controller", inplace=True)
        data.reset_index(inplace=True, drop=True,)
        data[reaction_prob_key_short] = data[reaction_prob_key_short].round(1)
        data = data.pivot(columns=["reactionProbability"])
        data.columns = data.columns.droplevel()
        data.boxplot()
        plt.ylim()
        plt.xlabel("Parameter c")
        plt.ylim(0,400)
        plt.title(f"Controller {c}")
        plt.show()
        print()



    fig, ax = plt.subplots(nrows=1, ncols=3, figsize=(15, 5))

    time_25 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.25).unstack(level=0)
    ax[0].plot(time_25, marker="o")
    ax[0].set_title("25% Quantile")
    ax[0].set_ylim(0, 140)

    time_med = travel_time.groupby(by=["Controller",reaction_prob_key_short]).median().unstack(level=0)
    ax[1].plot(time_med, marker= "o")
    ax[1].set_title("Median")
    ax[1].set_ylim(0, 140)

    time_75 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.75).unstack(level=0)
    ax[2].plot(time_75, marker="o")
    ax[2].set_title("75% Quantile")
    ax[2].set_ylim(0,140)

    plt.show()


    fig, ax = plt.subplots(nrows=len(probs), ncols=2, figsize=(6, int(rows*1.5)))
    counter = 0
    for c in ["OpenLoop","ClosedLoop","NoController"]:
        t_ = travel_time[travel_time["Controller"] == c]

        for reactionProb in probs:
            data = t_[t_[reaction_prob_key_short] == reactionProb]

            times = data[data["travel_time"] != np.inf]
            stucked = data[data["travel_time"] == np.inf]
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
            if counter >= len(probs):
                counter=0

    fig.tight_layout()
    plt.savefig(f"figs/travel_times.png")
    plt.show()
    print()

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
    variances_ = densities_normed.groupby(by=["Controller", reaction_prob_key_short]).mad()
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


def get_path_choice(controller_type):
    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_path_choice.csv", index_col=[0, 1])
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
    return path_choice

if __name__ == "__main__":

    path_choice = pd.concat([get_path_choice("OpenLoop"), get_path_choice("ClosedLoop")], axis=0)




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

    fig, ax = plt.subplots(nrows=1, ncols=4, figsize=(15, 5))
    ii = 0
    for (c,r), data in path_choice.groupby(by=["Controller", reaction_prob_key_short]):
        sss = pd.Series({11: 0, 31: 0, 51: 0})
        d = data["corridorRecommended"].value_counts()
        d = sss.combine(d, max)
        ax[ii].bar(x=d.index,height=d.values, width=15)
        title = f"{c}, {r}"
        ax[ii].set_title(title)
        ax[ii].set_xticks(d.index)
        ax[ii].set_ylim(0,2500)
        ax[ii].set_xlabel("Recommended corridor")
        ax[ii].set_ylabel("Counts")
        ii+=1
    plt.savefig(f"figs/CorridorRecommendation.png")
    plt.show()
    print()


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




