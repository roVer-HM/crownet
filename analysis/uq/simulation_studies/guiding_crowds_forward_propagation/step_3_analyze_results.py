import matplotlib.pyplot as plt
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols
import os

reaction_prob_key = "('Parameter', 'vadere', 'reactionProbabilities.[stimulusId==-400].reactionProbability')"
time_step_key = "timeStep"
seed_key_key = "('Parameter', 'vadere', 'fixedSeed')"
wall_clock_time_key = "('MetaInfo', 'required_wallclock_time')"
return_code_key = "('MetaInfo', 'return_code')"
simulation_time = 'Simulation time'
corridors = {'areaDensityCountingNormed-PID14': "Corridor1",
             'areaDensityCountingNormed-PID15': "Corridor2",
             'areaDensityCountingNormed-PID16': "Corridor3",
             'areaDensityCountingNormed-PID17': "Corridor4",
             'areaDensityCountingNormed-PID18': "Corridor5"}
velocities_dict = {'velocity-PID25': "Corridor1",
             'velocity-PID26': "Corridor2",
             'velocity-PID27': "Corridor3",
             'velocity-PID28': "Corridor4",
             'velocity-PID29': "Corridor5"}
densities_dict = {'density-PID25': "Corridor1",
             'density-PID26': "Corridor2",
             'density-PID27': "Corridor3",
             'density-PID28': "Corridor4",
             'density-PID29': "Corridor5"}
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
        raise ValueError("Fundamental diagram densities differ from measured densities.")

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
    densities_closed_loop_mean = densities.groupby(by=[reaction_prob_key_short, simulation_time]).mean()
    densities_closed_loop_std = densities.groupby(by=[reaction_prob_key_short, simulation_time]).std()


    for reaction_prob, data in densities_closed_loop_mean.groupby(by=reaction_prob_key_short):
        data.boxplot(showmeans=True, rot=45, fontsize=10)
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



if __name__ == "__main__":
    densities_closed_loop, velocities_closed_loop = get_fundamental_diagrams(controller_type="ClosedLoop")
    #plot_density_distribution(densities_closed_loop, controller="ClosedLoop")

    densities_open_loop, velocities_open_loop = get_fundamental_diagrams(controller_type="OpenLoop")
    #plot_density_distribution(densities_open_loop,controller="OpenLoop")

    densities_default, velocities_default = get_fundamental_diagrams(controller_type="NoController")
    #plot_density_distribution(densities_default,controller="NoController")

    #densities_default.groupby(by=[reaction_prob_key_short, simulation_time]).mean().plot()
    #plt.show()

    densities = pd.concat([densities_default, densities_open_loop])
    densities = pd.concat([densities, densities_closed_loop])

    velocities = pd.concat([velocities_default, velocities_open_loop])
    velocities = pd.concat([velocities, velocities_closed_loop])

    vvv = velocities[velocities[simulation_time] >= 117.5 ]
    vvv = vvv[vvv[simulation_time] <=117.7 ]
    vvv = vvv[vvv["Controller"] == "OpenLoop"]
    vvv = vvv[vvv[reaction_prob_key_short] == 1.0]
    vvv["Corridor1"].plot()

    vvv = densities[densities[simulation_time] >= 117.5]
    vvv = vvv[vvv[simulation_time] <= 117.7]
    vvv = vvv[vvv["Controller"] == "OpenLoop"]
    vvv = vvv[vvv[reaction_prob_key_short] == 1.0]
    vvv["Corridor1"].plot()
    plt.show()




    densities_mean = densities.groupby(by=["Controller", reaction_prob_key_short, simulation_time]).mean().reset_index()
    velocities_mean = velocities.groupby(by=["Controller", reaction_prob_key_short, simulation_time]) .mean().reset_index()
    densities_mean = densities
    velocities_mean = velocities


    for controller in ["OpenLoop","ClosedLoop","NoController"]:
        v_ = velocities_mean[velocities_mean["Controller"] == controller]
        d_ = densities_mean[densities_mean["Controller"] == controller]

        for reactionProb in [1.0, 0.5]:
            v__ = v_[v_[reaction_prob_key_short] == reactionProb]
            d__ = d_[d_[reaction_prob_key_short] == reactionProb]

            for c in list(corridors.values()):
                d___ = d__[c]
                v___ = v__[c]
                plt.scatter(x=d___, y=v___, label=c)


                #plt.xlim(0,1)
                #plt.ylim(0,1)
                plt.xlabel("Density [ped/m^2]")
                plt.ylabel("Velocity [m/s]")
                plt.title(f"Controller = {controller}. Reaction prob = {reactionProb}.")
                plt.legend()
                plt.show()

                df = pd.concat([d___, v___], axis=1)
                #df.plot()
                #plt.show()

                if controller == "NoController":
                    break

            print()

            if controller == "NoController":
                break






    # perform two-way ANOVA -https://www.statology.org/two-way-anova-python/
    model = ols('Corridor1 ~ C(reactionProbability) + C(Controller) + C(reactionProbability):C(Controller)', data=densities).fit()
    print(sm.stats.anova_lm(model, typ=2))