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
sim_time_steady_flow_start = 100
sim_time_steady_flow_end = 500
time_step_size = 0.4
var_corridor = "Corridor"
reaction_prob_key_short = "reactionProbability"


def get_densities(controller_type):
    c1 = pd.read_csv(f"{controller_type}_parameters.csv", index_col=[0, 1])
    c2 = pd.read_csv(f"{controller_type}_densities.csv", index_col=[0, 1])
    densities_closed_loop = c1.join(c2)
    densities_closed_loop.rename(columns=corridors, inplace=True)
    densities_closed_loop.sort_index(axis=1, inplace=True)
    densities_closed_loop[simulation_time] = densities_closed_loop[time_step_key] * time_step_size
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] >= sim_time_steady_flow_start]
    densities_closed_loop = densities_closed_loop[densities_closed_loop[simulation_time] < sim_time_steady_flow_end]
    densities_closed_loop.drop([seed_key_key, wall_clock_time_key, return_code_key, time_step_key], axis=1,
                               inplace=True)
    densities_closed_loop.rename(columns={reaction_prob_key: reaction_prob_key_short}, inplace=True)
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
    densities_closed_loop = get_densities(controller_type="ClosedLoop")
    #plot_density_distribution(densities_closed_loop, controller="ClosedLoop")

    densities_open_loop = get_densities(controller_type="OpenLoop")
    #plot_density_distribution(densities_open_loop,controller="OpenLoop")

    densities_default = get_densities(controller_type="NoController")
    #plot_density_distribution(densities_default,controller="NoController")

    densities_default.groupby(by=[reaction_prob_key_short, simulation_time]).mean().plot()
    plt.show()


    # compare strategies
    densities_default["Controller"] = "NoControl"
    densities_open_loop["Controller"] = "OpenLoop"
    densities_closed_loop["Controller"] = "ClosedLoop"

    densities = pd.concat([densities_default, densities_open_loop])
    densities = pd.concat([densities, densities_closed_loop])

    # perform two-way ANOVA -https://www.statology.org/two-way-anova-python/
    model = ols('Corridor1 ~ C(reactionProbability) + C(Controller) + C(reactionProbability):C(Controller)', data=densities).fit()
    print(sm.stats.anova_lm(model, typ=2))