import os, shutil
import string
from pandas.core.indexes import base
from scipy import rand
from suqc.CommandBuilder.SumoCommand import SumoCommand
from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.environment import CrownetEnvironmentManager
from suqc.parameter.create import opp_creator, coupled_creator
from suqc.parameter.sampling import ParameterVariationBase
from suqc.request import CoupledDictVariation, CrownetRequest
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
from suqc.CommandBuilder.OmnetCommand import OmnetCommand
from omnetinireader.config_parser import ObjectValue, UnitValue, QString, BoolValue
import pprint
import random



def main(base_path):
    
    t = UnitValue.s(400.0)

    par_var = [
        {
            "omnet": {  
                "*.traci.launcher.sumoCfgBase": 'absFilePath("sumo/simpleCrossing/example_peds_cars.sumo.cfg")'
            },
        },
        # {
        #     "omnet": {  
        #         "*.traci.launcher.sumoCfgBase": 'absFilePath("sumo/simpleCrossing/example_peds_cars.sumo2.cfg")'
        #     },
        # },
    ]
    par_var = OmnetSeedManager(
        par_variations=par_var, 
        rep_count=1, 
        omnet_fixed=False, 
        vadere_fixed=None).get_new_seed_variation()

    model = SumoCommand()\
        .write_container_log()\
        .omnet_tag("latest")\
        .sumo_tag("latest")\
        .experiment_label("out")
    model.timeout = None 
    model.qoi(["all"])
    model.verbose()


    # Enviroment setup. 
    # 
    ini_file = os.path.abspath("../omnetpp.ini")
    base_dir = os.path.abspath("/mnt/data1tb/results/")
    os.makedirs(base_dir, exist_ok=True)
    
    env = CrownetEnvironmentManager(
        base_path=base_dir,
        env_name=__file__.replace(".py", ""),
        opp_config="simple_cam_den",
        opp_basename="omnetpp.ini",
        mobility_sim=("sumo", "latest"), # use omnet internal mobility models
        communication_sim=("omnet", "latest"),
        handle_existing="ask_user_replace"
    )


    # Todo add all necessary sumo files not present in the ini file. 
    env.copy_data(
        base_ini_file=ini_file,
        scenario_files=[]
        )
    
    _rnd = ''.join(random.choices(string.ascii_lowercase + string.digits, k=6))
    parameter_variation = ParameterVariationBase().add_data_points(par_var)
    setup = CrownetRequest(
        env_man = env,
        parameter_variation=parameter_variation,
        model=model,
        creator=opp_creator,
        rnd_hostname_suffix=f"_{_rnd}",
        runscript_out="runscript.out"
    )
    print("setup done")
    # par_var, data = setup.run(len(par_var))
    par_var, data = setup.run(1)



if __name__ == "__main__":
    main("./")