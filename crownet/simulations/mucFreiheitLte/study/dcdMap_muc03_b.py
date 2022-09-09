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
    reps = 1    # seed-set
    mapCfgYmfDist = ObjectValue.from_args(
        "crownet::MapCfgYmfPlusDistStep", 
        "writeDensityLog", BoolValue.TRUE, 
        "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "alpha", 0.75,
        "idStreamType", QString("insertionOrder"),
        "stepDist", 50.0,
        "zeroStep", BoolValue.TRUE)
    mapCfgYmf = ObjectValue.from_args(
        "crownet::MapCfgYmf", 
        "writeDensityLog", BoolValue.TRUE, 
        "mapTypeLog", QString("all"),
        "cellAgeTTL", UnitValue.s(-1.0),
        "idStreamType", QString("insertionOrder"),
    )
    scenario_3source = QString("vadere/scenarios/mf_circle2.scenario")
    scenario_muc = QString("vadere/scenarios/mf_006_template.scenario")
    t = UnitValue.s(400.0)
    # seed = "195"
    seed = "113"

    par_var = [
        {
            "omnet": {  
                "sim-time-limit": t,
                "**.vadereScenarioPath" : scenario_muc,
                "*.pNode[*].app[1].foo": UnitValue.s(10.0) 
                },
            "vadere": {
                "pedestrian.freeFlowSpeed": 1.34
            }
        },{
            "omnet": {  
                "sim-time-limit": t,
                "**.vadereScenarioPath" : scenario_muc,
                "*.pNode[*].app[1].foo": UnitValue.s(10.0) 
                },
            "vadere": {
                "pedestrian.freeFlowSpeed": 1.34
            }
        }
    ]
    # add repetitions
    par_var = OmnetSeedManager(par_var, rep_count=3)\
        .get_new_seed_variation()

    # Model setup (Vadere/Omnet vs Sumo/Omnet)
    model = VadereOppCommand().write_container_log().model.qoi(["all"])

    # Request uses environment, parameter variation and model to run simulations
    setup = CrownetRequest(
        env_man = CrownetEnvironmentManager(...),
        parameter_variation=par_var,
        model=model,
    )
    setup.run(10) # run in parallel



if __name__ == "__main__":
    main("./")