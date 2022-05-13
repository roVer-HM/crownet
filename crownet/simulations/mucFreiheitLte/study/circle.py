import os, shutil
from pandas.core.indexes import base
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import CoupledScenarioCreation, CrownetCreation
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
import pprint

def main(base_path):
    reps = 1    # seed-set
    t = 400     # seconds
    # t = 3     # seconds
    # Parameter Variations 
    par_var = [
        {"omnet": {"*.misc[*].nTable.maxAge": "5s",
                   "sim-time-limit": f"{str(t)}s",
                   "**.numRbDl" : "25" ,
                   "**.numRbUl" : "25",
                   "**.numBands" : "25",
                   "**.vadereScenarioPath": '"vadere/scenarios/mf_circle.scenario"'
                   }
        }
    ]
    par_var = OmnetSeedManager(par_variations=par_var, rep_count=reps, omnet_fixed=False, vadere_fixed=None).get_new_seed_variation()


    # Model := Call to run_script.py 
    # This will define how the simulation is run. 
    # These settings here will override defaults set during CrownetRequest setup.
    # model = OmnetCommand() \
    #     .write_container_log() \
    #         .omnet_tag("latest") \
    #             .experiment_label("out")
    model = VadereOppCommand()\
         .write_container_log() \
         .omnet_tag("latest") \
         .experiment_label("out")
    model.timeout = None 
    model.qoi(["build_hdf"])
    model.verbose()


    # Enviroment setup. 
    # 
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("../suqc")
    os.makedirs(base_dir, exist_ok=True)

    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name="circle",
        opp_config="subwayDynamic_multiEnb_compact_density",
        opp_basename="omnetpp.ini",
        # mobility_sim=("omnet", ""), # use omnet internal mobility models
        mobility_sim=("vadere", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        # handle_existing="force_replace"
        # handle_existing="write_in"
        handle_existing="ask_user_replace"
    )
    env.copy_data(base_ini_file=ini_file)

    
    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
        creator=coupled_creator
    )

    # par_var, data = setup.run(2*3)
    par_var, data = setup.run(1)


def opp_creator(env_man, parameter_variation, njobs):
        scenario_creation = CrownetCreation(env_man, parameter_variation)
        return  scenario_creation.generate_scenarios(njobs)

def coupled_creator(env_man, parameter_variation, njobs):
        scenario_creation = CoupledScenarioCreation(env_man, parameter_variation)
        return  scenario_creation.generate_scenarios(njobs)

if __name__ == "__main__":
    main("./")