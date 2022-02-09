import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

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


    travel_times = pd.concat([c2, c3])
    #travel_times = pd.concat([travel_times, c3])

    return travel_times

def plot_travel_time(travel_time):

    travel_time = travel_time[travel_time["travel_time"] != np.inf]

    for c, data in travel_time.groupby(by=["Controller"]):
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
        plt.ylabel("Travel time [s]")
        plt.xlabel("Compliance rate")
        plt.savefig(f"figs/travel_time_{c}.png")
        plt.show()
        print()


    fig, ax = plt.subplots(nrows=1, ncols=3, figsize=(15, 5))

    time_25 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.25).unstack(level=0)
    time_25.columns = time_25.columns.droplevel(0)
    plt.sca(ax[0])
    time_25.plot(marker="o", ax=ax[0])
    plt.legend()
    ax[0].set_title("25% Quartile")
    ax[0].set_ylim(0, 150)
    ax[0].legend()
    ax[0].set_ylabel("Travel time [s]")
    ax[0].set_xlabel("Compliance rate")


    time_med = travel_time.groupby(by=["Controller",reaction_prob_key_short]).median().unstack(level=0)
    time_med.columns = time_med.columns.droplevel(0)
    plt.sca(ax[1])
    time_med.plot(marker="o", ax=ax[1])
    ax[1].set_title("Median")
    ax[1].set_ylim(0, 150)
    ax[1].legend()
    ax[1].set_ylabel("Travel time [s]")
    ax[1].set_xlabel("Compliance rate")

    time_75 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.75).unstack(level=0)
    time_75.columns = time_75.columns.droplevel(0)
    plt.sca(ax[2])
    time_75.plot(marker="o", ax=ax[2])
    ax[2].set_title("75% Quartile")
    ax[2].set_ylim(0, 150)
    ax[2].legend()
    ax[2].set_ylabel("Travel time [s]")
    ax[2].set_xlabel("Compliance rate")

    plt.savefig("figs/Travel_time")
    plt.show()




def get_densities_velocities():
    densities_closed_loop, velocities_closed_loop = get_fundamental_diagrams(controller_type="ClosedLoop")
    densities_open_loop, velocities_open_loop = get_fundamental_diagrams(controller_type="OpenLoop")
    densities_default, velocities_default = get_fundamental_diagrams(controller_type="NoController")

    densities = pd.concat([densities_closed_loop, densities_open_loop])
    velocities = pd.concat([velocities_closed_loop, velocities_open_loop])

    for c in corridors.values():
        #nan velocities in case there is no agent in the corridor
        velocities[c][densities[c] == 0] = np.nan

    return densities, velocities

def get_densities_normed(densities, density_thres = 0.31):
    # 0.31 ped/m**2 refers to a level of service A
    # 0.141 ped/m**2 -> social distance
    dens_max = 5.4
    densities_normed = densities.copy()
    densities_normed[list(corridors.values())] = (densities[list(corridors.values())] - density_thres) / (dens_max - density_thres)

    for col in list(corridors.values()):
        densities_normed[col][densities_normed[col] < 0] = 0

    return densities_normed


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
    path_choice[reaction_prob_key_short] = path_choice[reaction_prob_key_short].round(1)
    return path_choice

def plot_quantity(densities, file_name, y_min=0, y_max=2.5, ylabel="Density [ped/m*2]"):
    fig, ax = plt.subplots(nrows=2, ncols=3, figsize=(15, 10))
    i = 0
    for c, data in densities.groupby(by=["Controller"]):  # oxplot(column="travel_time"):

        densities_ = data
        densities_[reaction_prob_key_short] = densities_[reaction_prob_key_short].round(1)
        densities_ = densities_.reset_index(drop=True).drop(columns=[simulation_time, "Controller"]).pivot(
            columns=[reaction_prob_key_short])

        ii = 0
        for corridor_ in densities_.columns.get_level_values(0).unique():
            densities__ = densities_.xs(corridor_, axis=1)
            plt.sca(ax[i, ii])
            densities__.boxplot()
            ax[i, ii].set_title(f"{c}, {corridor_}")
            ax[i, ii].set_ylim(y_min, y_max)
            ax[i, ii].set_xlabel("Compliance rate")
            ax[i, ii].set_ylabel(ylabel)
            ii += 1
        i += 1

    plt.savefig(f"figs/{file_name}.png")
    plt.show()

def plot_route_1_recommended(path_choice):
    pp = path_choice[path_choice["Controller"] == "OpenLoop"]
    pp = pp[pp["corridorRecommended"] == 11]
    ppp = pp.groupby(by=reaction_prob_key_short).count()

    pp = path_choice[path_choice["Controller"] == "ClosedLoop"]
    pp = pp[pp["corridorRecommended"] == 11]
    pp = pp.groupby(by=reaction_prob_key_short).count()

    pp["corridorRecommended"].plot(label="Minimal density strategy")

    ppp = pd.concat([ppp["corridorRecommended"], pp["corridorRecommended"]], axis=1).fillna(0)
    ppp.columns = ["OpenLoop", "ClosedLoop"]
    ppp.plot()
    plt.ylabel("Counts")
    plt.title("Corridor 1 recommended")
    plt.savefig("figs/route1recommded.png")
    plt.show()


if __name__ == "__main__":

    path_choice = pd.concat([get_path_choice("OpenLoop"), get_path_choice("ClosedLoop")], axis=0)
    plot_route_1_recommended(path_choice)

    travel_time = get_travel_times()
    plot_travel_time(travel_time)

    densities, velocities = get_densities_velocities()

    plot_quantity(densities, "densities", y_min=-0.1, y_max=2.5, ylabel="Density [ped/m*2]")
    plot_quantity(velocities, "velocities", y_min=-0.1, y_max=1.8, ylabel="Velocities [m/s]")

    densities_normed_A = get_densities_normed(densities, density_thres=0.31,)
    plot_quantity(densities_normed_A, "densities_LOS_A", y_min=-0.1, y_max=0.5, ylabel="Density value [1]")

    densities_normed_15 = get_densities_normed(densities, density_thres=0.141)
    plot_quantity(densities_normed_15, "densities_LOS_15", y_min=-0.1, y_max=0.5, ylabel="Density value [1]")

    # write table data

    velocities.drop(columns=simulation_time).\
        groupby(by=["Controller", reaction_prob_key_short]).\
        median().\
        to_latex("tables/velocities.tex")
    densities.drop(columns=simulation_time).\
        groupby(by=["Controller", reaction_prob_key_short]).\
        median().\
        to_latex("tables/densities.tex")

    velocities.drop(columns=simulation_time).\
        groupby(by=["Controller", reaction_prob_key_short]).\
        mean().\
        to_latex("tables/velocities_mean.tex")
    densities.drop(columns=simulation_time).\
        groupby(by=["Controller", reaction_prob_key_short]).\
        mean().\
        to_latex("tables/densities_mean.tex")

    print()


