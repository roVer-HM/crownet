import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats

condition_names = {"A1": "Congestion info\n + arrow\n",
                   "A2": "Congestion info\n + arrow\n+ top down view\n",
                   "A3": "Congestion info\n + arrow\n+ team spirit\n",
                   "A4": "Congestion info\n + arrow\n+ top down view\n+ team spirit\n",
                   "B1": "Arrow \n",
                   "B2": "Arrow \n+ top down view\n",
                   "B3": "Arrow \n+ team spirit\n",
                   "B4": "Arrow \n+ top down view\n+ team spirit\n",
                   "Uninformed": "No congestion\ninformation\n"
                   }


reaction_prob_key = "('Parameter', 'dummy_var', 'compliance_rate')"
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
       21: "Medium",
       31: "Long",
       'Corridor1': "Short",
       'Corridor2': "Medium",
       'Corridor3': "Long",
       }

condition = "('Parameter', 'dummy_var', 'condition')"
stat = "('Parameter', 'dummy_var', 'statistic')"
condition_short = 'condition'
stat_short = 'stat'

args_boxplot = dict(flierprops=dict(marker='+', markerfacecolor='#AAAAAA', markersize=3,
                                    linestyle='none', markeredgecolor='#AAAAAA'),
                    boxprops=dict(color='#464646'),
                    whiskerprops=dict(linewidth=1),
                    medianprops=dict(linewidth=2.5, color='k'),
                    notch=True)

controller__ = {"OpenLoop": "Fixed order strategy",
                "ClosedLoop": "Minimal density strategy"}
sim_time_steady_flow_start = 250
sim_time_steady_flow_end = 1250
time_step_size = 0.4
var_corridor = "Corridor"
reaction_prob_key_short = "reactionProbability"
compliance_rate = "Compliance rate c"
probs = np.linspace(0, 1.0, 10)

def move_no_info_to_first_col(df):
    col = df.pop("No congestion\ninformation\n")
    df.insert(0, col.name, col)
    return df

def get_fundamental_diagrams(controller_type, time_start=None):

    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_fundamentalDiagramm.csv", index_col=[0, 1])
    densities_closed_loop = c1.join(c2)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= time_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.rename(columns={condition: condition_short, stat: stat_short}, inplace=True)

    velocities_ = densities_closed_loop.drop(columns=list(densities_dict.keys()))
    velocities_.rename(columns=velocities_dict, inplace=True)
    velocities_["Controller"] = controller_type
    velocities_.sort_index(axis=1, inplace=True)
    velocities_ = velocities_[['Controller', 'Corridor1', 'Corridor2', 'Corridor3', 'Simulation time','condition', 'stat']]

    densities_ = densities_closed_loop.drop(columns=list(velocities_dict.keys()))
    densities_.rename(columns=densities_dict, inplace=True)
    densities_["Controller"] = controller_type
    densities_.sort_index(axis=1, inplace=True)
    densities_ = densities_[['Controller', 'Corridor1', 'Corridor2', 'Corridor3', 'Simulation time','condition', 'stat']]

    densities_["condition"].replace(condition_names, inplace=True)
    velocities_["condition"].replace(condition_names, inplace=True)

    #densities_ = move_no_info_to_first_col(densities_)
    #velocities_ = move_no_info_to_first_col(velocities_)

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


def get_travel_time(controller_type):
    c1 = pd.read_csv(f"{controller_type}_startTime.csv", index_col=[0, 1, 2])
    c2 = pd.read_csv(f"{controller_type}_targetReachTime.csv", index_col=[0, 1, 2])
    times = c1.join(c2)
    travel_times = pd.DataFrame(times["reachTime-PID12"].sub(times["startTime-PID13"]),
                                columns=["travel_time"])
    travel_times = travel_times.assign(Controller=controller_type)
    travel_times = travel_times[times["startTime-PID13"] >= sim_time_steady_flow_start]

    travel_times.reset_index(level="pedestrianId", inplace=True, drop=True)

    param = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    travel_times = travel_times.join(param[[condition, stat]])

    travel_times.rename(columns={condition: condition_short, stat: stat_short}, inplace=True)

    travel_times = travel_times[travel_times["travel_time"] != np.inf]

    travel_times["condition"].replace(condition_names, inplace=True)

    return travel_times




def plot_travel_time(travel_time):


    data = travel_time
    data.drop(columns="Controller", inplace=True)
    data.reset_index(inplace=True, drop=True, )
    data.set_index(["condition", "stat"], inplace=True)

    for s_, data_s in data.groupby(level="stat"):

        tit = f"TravelTime{s_}"
        data_s.index = data_s.index.droplevel(1)
        data_s = pd.DataFrame(data_s.unstack() , columns= ["travel_time"])
        data_s.reset_index(inplace=True)
        data_s = data_s.drop(columns=["level_0"])
        data_s = data_s.pivot(columns=[condition_short])
        data_s.columns = data_s.columns.droplevel(0)

        data_s = move_no_info_to_first_col(data_s)

        x = [data_s[column].dropna().values for column in data_s]
        res = stats.kruskal(*x)

        data_s.boxplot()
        plt.title(f"{s_}")
        plt.ylabel("Travel time [s]")
        plt.xlabel("")
        plt.savefig(f"figs/{tit}.pdf")
        plt.xticks(rotation=90, ha="right", rotation_mode="anchor")
        plt.show()


    statistics_ = data.groupby(level=["stat", "condition"]).describe()
    statistics_.to_latex("tables/TravelTimeStatistics.tex", escape=False, float_format="%.1f")
    print()



def get_densities_velocities(start_time = None):
    densities, velocities = get_fundamental_diagrams(controller_type="ClosedLoop", time_start= start_time)

    for c in corridors.values():
        velocities[c][densities[c] == 0] = np.nan

    return densities, velocities


def get_densities_normed(densities, density_thres=0.31):
    # 0.31 ped/m**2 refers to a level of service A
    # 0.141 ped/m**2 -> social distance
    dens_max = 5.4
    densities_normed = densities.copy()
    densities_normed[list(corridors.values())] = (densities[list(corridors.values())] - density_thres) / (
                dens_max - density_thres)

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
    path_choice = path_choice[[condition, stat, "Simulation time", "corridorRecommended"]]

    path_choice.rename(columns={condition: condition_short, stat: stat_short}, inplace=True)
    path_choice = path_choice.assign(Controller=controller_type)
    path_choice.sort_index(axis=1, inplace=True)

    path_choice.reset_index(inplace=True)
    path_choice.drop(columns=["run_id", "id"], inplace=True)
    path_choice.set_index(simulation_time, inplace=True)

    path_choice = path_choice[path_choice["condition"]!="Uninformed"]

    #path_choice.rename(index=condition_names, inplace=True)
    path_choice["condition"].replace(condition_names, inplace=True)

    return path_choice


def plot_quantity(densities, file_name, y_min=0, y_max=2.5, ylabel="Density [ped/m*2]"):
    fig, ax = plt.subplots(nrows=3, ncols=2, figsize=(10, 12.5))
    fig.subplots_adjust(wspace=0.05, hspace=0.3)

    i = 0
    c_order = list()
    for c, data in densities.groupby(by=["Controller"]):  # oxplot(column="travel_time"):

        c_order.append(controller__[c])

        densities_ = data
        densities_[reaction_prob_key_short] = densities_[reaction_prob_key_short].round(3)
        densities_ = densities_.reset_index(drop=True).drop(columns=[simulation_time, "Controller"]).pivot(
            columns=[reaction_prob_key_short])

        ii = 0
        for corridor_ in densities_.columns.get_level_values(0).unique():
            densities__ = densities_.xs(corridor_, axis=1)
            plt.sca(ax[ii, i])
            densities__.boxplot(**args_boxplot)
            ax[ii, i].set_title(f"{c__[corridor_]} corridor")
            ax[ii, i].set_ylim(y_min, y_max)
            ax[ii, i].set_xlabel(compliance_rate)
            if i%2 ==0:
                ax[ii, i].set_ylabel(ylabel)
            else:
                ax[ii, i].set_yticklabels([])
            ax[ii, i].set_xticks(np.arange(1, 45, 4))
            ax[ii, i].set_xticklabels([f"{x:.1f}" for x in np.linspace(0, 1, 11)])

            ii += 1
        i += 1

    fig.suptitle("                  ".join(c_order), fontsize=16, x=0.47)
    plt.savefig(f"figs/{file_name}.png")
    plt.show()
    print()


def plot_number_of_recommendations_long_route(path_choice):
    corridor_ = 31 # long corridor

    path_choice = path_choice[path_choice["corridorRecommended"] == corridor_]
    ppp = path_choice.groupby(by=[stat_short, condition_short]).count()["corridorRecommended"].unstack().transpose()
    ppp.columns.name = "Group"


    ppp.sort_index(inplace=True)

    ppp.plot.bar()
    plt.xticks(ha="right", rotation_mode="anchor", rotation=90)
    plt.ylabel("Number of recommendations \n for the long route")
    plt.xlabel("")
    plt.savefig(f"figs/NumberOfRouteRecommendationsLongRoute.pdf")
    plt.show()
    print()






def compare_random_densitiies(dd, travel_time, value="Median", ):
    time = travel_time

    # median or mean does not make difference

    dd = d_med.drop(columns=["Corridor2", "Corridor3"])
    dd.reset_index(0, inplace=True)
    dd = dd.pivot(columns="Controller")
    dd = dd.drop(0.0)
    dd.columns = dd.columns.droplevel(0)
    dd.rename(columns=controller__, inplace=True)
    a = dd.hist(figsize=(10,4),grid=False)
    a[0][0].set_ylim(0, 20)
    a[0][0].set_xlim(0.22, 2.1)
    a[0][0].set_xlabel("Median density [$ped/m^2$]")
    a[0][1].set_xlabel("Median density [$ped/m^2$]")
    a[0][0].set_ylabel("Counts")
    a[0][1].set_ylim(0, 20)
    a[0][1].set_yticklabels([])
    a[0][1].set_xlim(0.22, 2.1)

    for xxx in [0,1]:
        left, right = a[0][xxx].get_ylim()
        los = [0.2, 0.31, 0.43, 0.71, 1.08] #,, 2.17
        a[0][xxx].vlines(los[1:], ymin=left, ymax=right, color='b', linestyles='--')
        for los_, letter in zip(los, ["A", "B", "C", "D", "E", "F"]):
            a[0][xxx].text(los_ + 0.05, 10, f"LOS {letter}", rotation=90)


    plt.subplots_adjust(wspace=0.2, hspace=0)
    plt.savefig(f"figs/{value}DensityDist2.png")
    plt.show()

    dd.boxplot(grid=False)
    left, right = plt.xlim()
    los = [0.31, 0.43, 0.71, 1.08, 2.17]
    plt.hlines(los, xmin=left, xmax=right, color='b', linestyles='--')
    for los_, letter in zip(los, ["A", "B", "C", "D", "E", "F"]):
        plt.text(1.5, los_ - 0.075, f"LOS {letter}")

    plt.ylabel(f"{value} density [$ped/m^2$]")
    plt.title("Short corridor")
    plt.savefig(f"figs/{value}DensityDist.png")
    plt.show()
    # not normally distributed, if p < 0.05
    s_1 = stats.kstest(dd.iloc[:, 0], "norm")
    s_2 = stats.kstest(dd.iloc[:, 1], "norm")

    # difference between medians, if p < 0.05
    kruskal = stats.kruskal(dd.iloc[:, 0], dd.iloc[:, 1])
    mannwhitneyU = stats.mannwhitneyu(dd.iloc[:, 0], dd.iloc[:, 1])

    times = travel_time.groupby(by=["Controller", reaction_prob_key_short]).median()
    times.reset_index(0, inplace=True)
    times = times.pivot(columns="Controller")
    times = times.drop(0.0)
    times.columns = times.columns.droplevel(0)
    times.rename(columns=controller__, inplace=True)

    a = times.hist(figsize=(10,4), grid=False)
    a[0][0].set_ylim(0, 16)
    a[0][0].set_xlim(60, 200)
    a[0][0].set_xlabel("Median travel time [$s$]")
    a[0][0].set_ylabel("Counts")

    a[0][1].set_xlabel("Median travel time [$s$]")
    #a[0][1].set_ylabel("Counts")
    a[0][1].set_ylim(0, 16)
    a[0][1].set_xlim(60, 200)
    a[0][1].set_yticklabels([])
    plt.subplots_adjust(wspace=0.2, hspace=0)
    plt.savefig(f"figs/{value}TravelDist2.png")
    plt.show()

    times.boxplot()
    plt.ylabel(f"{value} travel time [s]")
    plt.savefig(f"figs/{value}TravelDist.png")
    plt.show()

    s_1_ = stats.kstest(times.iloc[:, 0], "norm")
    s_2_ = stats.kstest(times.iloc[:, 1], "norm")
    kruskal_ = stats.kruskal(times.iloc[:, 0], times.iloc[:, 1])
    mannwhitneyU_ = stats.mannwhitneyu(times.iloc[:, 0], times.iloc[:, 1])

    with open('tables/statistical_test.txt', 'w+') as f:
        f.write(f"{dd.columns[0]}: densities not normally distributed, since p = {s_1.pvalue} < 0.05.\n")
        f.write(f"{dd.columns[1]}: densities not normally distributed, since p = {s_2.pvalue} < 0.05.\n")
        f.write(f"Kruskal Wallis test: densities. no difference between groups, since  p = {kruskal.pvalue} > 0.05.\n")
        f.write(f"mannwhitneyu test: densities. difference between groups , p = {mannwhitneyU.pvalue} < 0.05.\n")
        f.write(f"{times.columns[0]}: densities not normally distributed, since p = {s_1_.pvalue} < 0.05.\n")
        f.write(f"{times.columns[1]}: densities not normally distributed, since p = {s_2_.pvalue} < 0.05.\n")
        f.write(f"Kruskal Wallis test: times. no difference between groups, since  p = {kruskal_.pvalue} > 0.05.\n")
        f.write(f"mannwhitneyu test: times. no difference between groups , p = {mannwhitneyU_.pvalue} > 0.05.\n")



def plot_stationary_behavior(quantity, name="Density", stationary="stationary"):

    for controller, data in quantity.groupby(by="Controller"):
        data.set_index(["Simulation time"], inplace=True)
        data.plot(linestyle = 'None', marker=".")
        plt.ylabel(name)
        plt.title(f"{controller}, {name}, {stationary}")
        print("Start saving....")
        plt.savefig(f"figs/Stationary_{controller}_{name}_{stationary}.png")
        plt.show()
        print("finished saving....")


def plot_distributions(quantity, name="density"):

    corridors = ["Corridor1", "Corridor2", "Corridor3"]
    for controller, data in quantity.groupby(by="Controller"):

        data.set_index(["stat", "condition"], inplace=True)
        data = data[corridors]
        for stats_, data_ in data.groupby(level="stat"):
            data_.groupby(level="condition").boxplot(layout=(3,4))
            plt.suptitle(f"{controller}, {name}, {stats_}")
            plt.savefig(f"figs/{name}_{controller}_{stats_}.png")
            plt.show()

def plot_velocities_densities_short_corridor(densities, velocities):


    densities = densities[["condition", "Corridor1", "stat"]]
    densities = densities.set_index("stat")

    for stats_, data_ in densities.groupby(level="stat"):
        data_ = data_.reset_index().pivot(values="Corridor1", columns="condition")
        #x = [data_[column].dropna().values for column in data_]
        #res = stats.kruskal(*x)
        data_ = move_no_info_to_first_col(data_)
        data_.boxplot()
        plt.xticks(rotation=90, ha="right", rotation_mode = "anchor")
        plt.title(f"{stats_}")
        plt.savefig(f"figs/DensityShortCorridor{stats_}.pdf")
        plt.ylabel("Density [$ped/m^2$] \n in the short corridor")
        plt.ylim(bottom= -0.05, top=1.7)
        plt.show()

    velocities = velocities[["condition", "Corridor1", "stat"]]
    velocities = velocities.set_index("stat")

    for stats_, data_ in velocities.groupby(level="stat"):
        data_ = data_.reset_index().pivot(values="Corridor1", columns="condition")
        #x = [data_[column].dropna().values for column in data_]
        #res = stats.kruskal(*x)
        data_ = move_no_info_to_first_col(data_)
        data_.boxplot()
        plt.xticks(rotation=90, ha="right", rotation_mode = "anchor")
        plt.title(f"{stats_}")
        plt.savefig(f"figs/VelocityShortCorridor{stats_}.pdf")
        plt.ylabel("Velocity [$m/s^2$] \n in the short corridor")
        plt.ylim(bottom= -0.05, top=2.2)
        plt.show()



if __name__ == "__main__":


    #path_choice =  get_path_choice("ClosedLoop")
    #plot_number_of_recommendations_long_route(path_choice)

    #travel_time = get_travel_time(controller_type="ClosedLoop")
    #plot_travel_time(travel_time)


    densities, velocities = get_densities_velocities(start_time=0.0)
    densities_stat, velocities_stat = get_densities_velocities(start_time=sim_time_steady_flow_start)

    plot_velocities_densities_short_corridor(densities_stat, velocities_stat)


    plot_distributions(densities_stat, name="Density")
    plot_distributions(velocities_stat, name="Velocity")



    plot_stationary_behavior(densities_stat, name="Density")
    plot_stationary_behavior(velocities_stat, name="Velocity")
    plot_stationary_behavior(densities, name="Density", stationary="inflow")
    plot_stationary_behavior(velocities, name="Velocity", stationary="inflow")

