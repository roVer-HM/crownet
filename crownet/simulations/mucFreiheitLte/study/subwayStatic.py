import os, shutil
from pandas.core.indexes import base
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
import pprint

def main(base_path):
    reps = 3    # seed-set

    # Parameter Variations 
    par_var = [
        {"omnet": {"*.misc[*].nTable.maxAge": "3s"}},
        # {"omnet": {"*.misc[*].nTable.maxAge": "5s"}},
        {"omnet": {"*.misc[*].nTable.maxAge": "8s"}},
        {"omnet": {"*.misc[*].nTable.maxAge": "10s"}},
    ]
    par_var = OmnetSeedManager(par_variations=par_var, rep_count=reps, omnet_fixed=False, vadere_fixed=None).get_new_seed_variation()


    # Model := Call to run_script.py 
    # This will define how the simulation is run. 
    # These settings here will override defaults set during CrownetRequest setup.
    model = OmnetCommand() \
        .write_container_log() \
            .omnet_tag("latest") \
                .experiment_label("out")
    model.timeout = None 
    model.qoi(["build_hdf", "create_figures"])


    # Enviroment setup. 
    # 
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("../suqc")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name="test_003",
        opp_config="subwayStationary_multiEnb",
        opp_basename="omnetpp.ini",
        mobility_sim=("omnet", ""), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace"
    )
    env.copy_data(base_ini_file=ini_file)

    
    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    f = foo(env, parameter_variation)
    print(f)
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
    )

    par_var, data = setup.run(3*3)

def foo(env, var):
    runs = {}
    for (par_id, run_id), par in var._points.iterrows():
        base_dir = os.path.join(
            env.get_env_outputfolder_path(),
            env.get_simulation_directory(par_id, run_id),
        )
        run_name = os.path.basename(base_dir)
        run = {}
        run["name"] = run_name
        run["base_dir"] = base_dir
        run["par_id"] = par_id
        run["run_id"] = run_id
        par = par.droplevel(0).to_frame().reset_index(1).groupby(level=0)
        for group, df in par:
            sim = {}
            for index, row in df.iterrows():
                row = row.to_list()
                sim[row[0]] = row[1]
            run[group] = sim
        runs[run_name] = run  


    return runs


if __name__ == "__main__":
    main("./")