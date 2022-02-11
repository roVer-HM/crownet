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
c__ = {11: "Short",
       31: "Medium",
       51: "Long",
       'Corridor1': "Short",
       'Corridor2':  "Medium",
       'Corridor3':  "Long",
       }

args_boxplot = dict(flierprops=dict(marker='+', markerfacecolor='#AAAAAA', markersize=3,
                                linestyle='none', markeredgecolor='#AAAAAA'),
                                boxprops=dict(color ='#464646'),
                                whiskerprops=dict(linewidth=1),
                                medianprops=dict(linewidth=2.5, color='k'),
                                notch=True)

controller__ = {"OpenLoop": "Fixed order strategy",
                "ClosedLoop" : "Recommend emptiest corridor strategy" }
sim_time_steady_flow_start = 250
sim_time_steady_flow_end = 500
time_step_size = 0.4
var_corridor = "Corridor"
reaction_prob_key_short = "reactionProbability"

probs = np.linspace(0, 1.0, 11)
probs = np.linspace(0, 1.0, 41)

def get_fundamental_diagrams(controller_type, time_start=None):
    if time_start is None:
        time_start = sim_time_steady_flow_start

    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_fundamentalDiagramm.csv", index_col=[0, 1])
    densities_closed_loop = c1.join(c2)

    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= time_start]
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
        data[reaction_prob_key_short] = data[reaction_prob_key_short].round(3)
        data = data.pivot(columns=["reactionProbability"])
        data.columns = data.columns.droplevel()
        data.rename(columns=controller__, inplace=True)
        data.boxplot(**args_boxplot)
        plt.ylim()
        plt.xlabel("Parameter c")
        plt.ylim(0,400)
        plt.title(f"{controller__[c]}")
        plt.xticks(np.arange(1, 45, 4), [f"{x:.1f}" for x in np.linspace(0, 1, 11)])
        plt.ylabel("Travel time [s]")
        plt.xlabel("Compliance rate")
        plt.savefig(f"figs/travel_time_{controller__[c]}.png")
        plt.show()
        print()


    fig, ax = plt.subplots(nrows=1, ncols=3, figsize=(15, 5))

    time_25 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.25).unstack(level=0)
    time_25.columns = time_25.columns.droplevel(0)
    time_25.rename(columns=controller__, inplace=True)
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
    time_med.rename(columns=controller__, inplace=True)
    plt.sca(ax[1])
    time_med.plot(marker="o", ax=ax[1])
    ax[1].set_title("Median")
    ax[1].set_ylim(0, 150)
    ax[1].legend()
    ax[1].set_ylabel("Travel time [s]")
    ax[1].set_xlabel("Compliance rate")

    time_75 = travel_time.groupby(by=["Controller",reaction_prob_key_short]).quantile(0.75).unstack(level=0)
    time_75.columns = time_75.columns.droplevel(0)
    time_75.rename(columns=controller__, inplace=True)
    plt.sca(ax[2])
    time_75.plot(marker="o", ax=ax[2])
    ax[2].set_title("75% Quartile")
    ax[2].set_ylim(0, 150)
    ax[2].legend()
    ax[2].set_ylabel("Travel time [s]")
    ax[2].set_xlabel("Compliance rate")

    plt.savefig("figs/Travel_time")
    plt.show()

def plot_hists_corridor1():
    densities, velocities = get_fundamental_diagrams(controller_type="NoController", time_start=0)

    for c in ["Corridor1"]:
        velocities[c][densities[c] == 0] = np.nan

    velocities.dropna(inplace=True)

    densities[simulation_time] = densities[simulation_time].round(2)
    densities.reset_index(inplace=True)
    densities.set_index(["id", simulation_time], inplace=True)

    for ii, data in densities["Corridor1"].groupby(by=["id"]):
        data.index = data.index.droplevel(0)
        data.plot(label=f"Simulation run {ii+1}") # bins=20, range=(0, 0.5))

    plt.legend()
    plt.xlabel("Simulation time [s]")
    plt.ylabel("Density $ped/m^{2}$")
    plt.title("Short corridor (no rerouting)")
    plt.savefig("figs/DensityOverTime.png")
    plt.show()

    velocities[simulation_time] = velocities[simulation_time].round(2)
    velocities.reset_index(inplace=True)
    velocities.set_index(["id", simulation_time], inplace=True)

    for ii, data in velocities["Corridor1"].groupby(by=["id"]):
        data.index = data.index.droplevel(0)
        data.plot(label=f"Simulation run {ii+1}") # bins=20, range=(0, 0.5))

    plt.legend()
    plt.xlabel("Simulation time [s]")
    plt.ylabel("Velocity $m/s$")
    plt.title("Short corridor (no rerouting)")
    plt.savefig("figs/VelocityOverTime.png")
    plt.show()

    densities, velocities = get_fundamental_diagrams(controller_type="NoController")
    times = get_time_controller_wise(controller_type="NoController")

    df = pd.concat([densities["Corridor1"].describe(), velocities["Corridor1"].describe()], axis=1)

    df["t"] = times[times["travel_time"] != np.inf].describe()["travel_time"]

    df.columns = ["Density $ped/m**2$", "Velocity $m/s$", "Travel time [s]"]
    df = df.transpose()
    df.to_latex("tables/NoControlCorridor1.tex", float_format="%.2f")




def get_fundamental_diagram_corridor1():
    densities, velocities = get_fundamental_diagrams(controller_type="NoController", time_start=0.0)

    for c in ["Corridor1"]:
        velocities[c][densities[c] == 0] = np.nan
        densities[c][densities[c] == 0] = np.nan

    densities.dropna(inplace=True)
    velocities.dropna(inplace=True)



    flux = densities["Corridor1"]*velocities["Corridor1"]

    from sklearn.preprocessing import PolynomialFeatures
    from sklearn.pipeline import make_pipeline
    from sklearn.linear_model import LinearRegression
    degree=2
    polyreg=make_pipeline(PolynomialFeatures(degree),LinearRegression())
    polyreg.fit(densities["Corridor1"].values.reshape(-1, 1), flux.values.reshape(-1, 1))


    plt.scatter(densities["Corridor1"], flux, label= "Simulation data" )
    d__ = np.linspace(0,2.1)
    flow_reg = polyreg.predict(d__.reshape(-1, 1))
    plt.plot(d__, flow_reg.ravel(), color="black", label= "Regression (poly.)")

    d___ = np.linspace(2.1, 3.2)
    flow_reg_ = polyreg.predict(d___.reshape(-1, 1))
    plt.plot(d___, flow_reg_.ravel(), color="black", linestyle= "--", label=None)

    plt.scatter(2.1, flow_reg.max(), marker="D", c="black", label="$J_{1,max}$ (Regression)")
    plt.text(2.1+0.1, flow_reg.max()+0.05, f'{flow_reg.max():.2f} (d = 2.1 $ped/m^{2}$)')

    densitiy_max_flux = densities.iloc[np.where(flux==flux.max())[0][0], :]["Corridor1"]
    plt.scatter(densitiy_max_flux, flux.max(), marker="s", c="black", label="$J_{1,max}$ (Data)")
    plt.text(densitiy_max_flux+0.1, flux.max() + 0.0, f'{flux.max():.2f} (d = {densitiy_max_flux:.2f} $ped/m^{2}$)')



    plt.xlabel("Density [ped/m²]")
    plt.ylabel("Specific flow [1/(ms)]")
    plt.xlim(-0.1,3.5)
    plt.title(f"Fundamental diagram: short corridor")
    plt.legend()
    plt.savefig(f"figs/fundamentalDiagramShortCorridor1.png")
    plt.show()

    plt.scatter(densities["Corridor1"], velocities["Corridor1"], label="Simulation data")
    plt.xlabel("Density [$ped/m^{2}$]")
    plt.ylabel("Velocity [m/s]")
    plt.title(f"Fundamental diagram: short corridor")
    plt.xlim(-0.1, 3.5)
    plt.legend()
    plt.savefig(f"figs/fundamentalDiagramShortCorridor2.png")
    plt.show()



def get_densities_velocities():
    densities_closed_loop, velocities_closed_loop = get_fundamental_diagrams(controller_type="ClosedLoop")
    densities_open_loop, velocities_open_loop = get_fundamental_diagrams(controller_type="OpenLoop")

    densities = pd.concat([densities_closed_loop, densities_open_loop])
    velocities = pd.concat([velocities_closed_loop, velocities_open_loop])

    for c in corridors.values():
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
    path_choice[reaction_prob_key_short] = path_choice[reaction_prob_key_short].round(3)
    return path_choice

def plot_quantity(densities, file_name, y_min=0, y_max=2.5, ylabel="Density [ped/m*2]"):
    fig, ax = plt.subplots(nrows=3, ncols=2, figsize=(10, 15))

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
            ax[ii, i].set_title(f"{c__[corridor_]} corridor.")
            ax[ii, i].set_ylim(y_min, y_max)
            ax[ii, i].set_xlabel("Compliance rate r [1]")
            ax[ii, i].set_ylabel(ylabel)
            ax[ii, i].set_xticks(np.arange(1,45,4))
            ax[ii, i].set_xticklabels([f"{x:.1f}" for x in np.linspace(0,1,11)])

            ii += 1
        i += 1

    fig.suptitle("                  ".join(c_order), fontsize=16, x= 0.47)
    plt.savefig(f"figs/{file_name}.png")
    plt.show()
    print()

def plot_route_1_recommended(path_choice, corridor_= 11):


    corridor = c__[corridor_]

    pp = path_choice[path_choice["Controller"] == "OpenLoop"]
    pp = pp[pp["corridorRecommended"] == corridor_]
    ppp = pp.groupby(by=reaction_prob_key_short).count()

    pp = path_choice[path_choice["Controller"] == "ClosedLoop"]
    pp = pp[pp["corridorRecommended"] == corridor_]
    pp = pp.groupby(by=reaction_prob_key_short).count()

    ppp = pd.concat([ppp["corridorRecommended"], pp["corridorRecommended"]], axis=1).fillna(0)
    ppp.columns = list(controller__.values())
    ppp.plot(marker="o")
    plt.ylim(-10,300)
    plt.ylabel("Counts")
    plt.xlabel("Compliance rate r [1]")
    plt.title(f"{corridor} route recommended")
    plt.savefig(f"figs/route{corridor}recommded.png")
    plt.show()
    print()


if __name__ == "__main__":
    plot_hists_corridor1()

    get_fundamental_diagram_corridor1()

    path_choice = pd.concat([get_path_choice("OpenLoop"), get_path_choice("ClosedLoop")], axis=0)
    plot_route_1_recommended(path_choice, 11)
    plot_route_1_recommended(path_choice, 31)
    plot_route_1_recommended(path_choice, 51)

    travel_time = get_travel_times()
    plot_travel_time(travel_time)

    densities, velocities = get_densities_velocities()

    plot_quantity(densities, "densities", y_min=-0.1, y_max=2.2, ylabel="Density [ped/m*2]")
    plot_quantity(velocities, "velocities", y_min=-0.1, y_max=2.2, ylabel="Velocities [m/s]")

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

