

import suqc,os

vadere_filename = "vadere1_4.jar"
path2tutorial= "/home/christina/repos/suq-controller/tutorial"
post_changes=suqc.PostScenarioChangesBase(apply_default=True)

samples = [ [ {'speedDistributionMean': 0.7, 'speedDistributionStandardDeviation': 5.2 } ], [ {'speedDistributionMean': 1.3, 'speedDistributionStandardDeviation': 5.4 } ] ]

for x in range(len(samples)):
    current_folder = "simulation_000" + str(x)
    env_man = suqc.EnvironmentManager.create_variation_env(basis_scenario="/home/christina/repos/rover-main/simulation-campaigns/testfolder/id_0001/input/example.scenario",
                                                       base_path= os.path.abspath(".") , env_name=current_folder, handle_existing="force_replace", vadere_folder = "input")
    parameter_dict_list = samples[x]

    parameter_variation = suqc.UserDefinedSampling(parameter_dict_list)
    parameter_variation = parameter_variation.multiply_scenario_runs(1)

    scenario_creation = suqc.VadereScenarioCreation(env_man, parameter_variation, post_changes)
    request_item_list = scenario_creation.generate_vadere_scenarios(1)

print("finisehd")
