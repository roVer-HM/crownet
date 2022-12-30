import subprocess
import os
import signal
import time
import pandas as pd

def execute(command):
    prc = subprocess.Popen(["bash", "-c", f"{command}"])
    return prc
    

def cronwet_command(run, config):
    return f"docker exec -w \"{os.getcwd()}\" omnetpp sh -c \"../../src/CROWNET -r {run} -m -u Cmdenv "\
            f"-c {config} -n "\
            "..:../../src:../../../artery/scenarios/artery:../../../artery/scenarios/car2car-grid:../../../artery/scenarios/gemv2:../../../artery/scenarios/highway-police:../../../artery/scenarios/lte-blackice:../../../artery/scenarios/mt-its2017:../../../artery/scenarios/ots-demo:../../../artery/scenarios/rsu_grid:../../../artery/scenarios/simulte:../../../artery/scenarios/storyboard:../../../artery/scenarios/testbed:../../../artery/scenarios/transfusion:../../../artery/src/artery:../../../artery/src/ots:../../../artery/src/traci:../../../inet4/examples:../../../inet4/showcases:../../../inet4/src:../../../inet4/tests/validation:../../../inet4/tests/networks:../../../inet4/tutorials:../../../simu5g/emulation:../../../simu5g/simulations:../../../simu5g/src:../../../veins/subprojects/veins_inet/examples/veins_inet:../../../veins/subprojects/veins_inet/examples/veins_inet_pedestrians:../../../veins/subprojects/veins_inet/examples/veins_inet_vadere:../../../veins/subprojects/veins_inet/src/veins_inet:../../../veins/examples/veins:../../../veins/src/veins -x inet.applications.voipstream;inet.common.selfdoc;inet.emulation;inet.examples.emulation;inet.examples.voipstream;inet.linklayer.configurator.gatescheduling.z3;inet.showcases.emulation;inet.showcases.visualizer.osg;inet.transportlayer.tcp_lwip;inet.visualizer.osg --image-path=../../images:../../../inet4/images:../../../simu5g/images:../../../veins/subprojects/veins_inet/images:../../../veins/images -l ../../../inet4/src/INET -l ../../../simu5g/src/simu5g -l ../../../veins/subprojects/veins_inet/src/veins_inet -l ../../../veins/src/veins "\
            "omnetpp.ini\";exit 0"
            
    
def single_run(run, config):
    vadere = execute("../../../scripts/vadere")
    sumo = execute("../../../scripts/sumo")
    
    # Give vadere and sumo time to intialize
    time.sleep(10)
    
    crownet = execute(cronwet_command(run, config))
    
    frame = pd.DataFrame()
    
    with open(f"load_study_results/{config}_{run}.txt", "a") as f:
        
        while crownet.poll() is None:
            f.write(subprocess.check_output(["docker", "stats", "--format", "{{.Name}}_{{.CPUPerc}}_{{.MemUsage}}", "--no-stream", "omnetpp"]).decode("utf-8"))
            f.write(subprocess.check_output(["docker", "stats", "--format", "{{.Name}}_{{.CPUPerc}}_{{.MemUsage}}", "--no-stream", "vadere"]).decode("utf-8"))
            f.write(subprocess.check_output(["docker", "stats", "--format", "{{.Name}}_{{.CPUPerc}}_{{.MemUsage}}", "--no-stream", "sumo"]).decode("utf-8"))
            
    
    execute("docker kill vadere")
    execute("docker kill sumo")
    
    time.sleep(10)
    
if __name__ == "__main__":
    for rep in range(5, 10):
        single_run(rep, "LoadStudySumo")
        single_run(rep, "LoadStudyCombined")
        
        