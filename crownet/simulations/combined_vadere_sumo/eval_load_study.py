import glob
import humanfriendly
import numpy as np

def main(glo):
    ram_sumo = []
    ram_opp = []
    ram_vadere = []
    
    cpu_sumo = []
    cpu_opp = []
    cpu_vadere = []
        
    for file in glob.glob(f"load_study_results/{glo}"):
        
        with open(file, "r") as f:
            for line in f.readlines():
                simulator = line.split("_")[0]
                load = float(line.split("_")[1].split("%")[0])
                ram = humanfriendly.parse_size(line.split("_")[2].split(" ")[0])
                
                if simulator == "omnetpp":
                    ram_opp.append(ram - humanfriendly.parse_size("1.3 GiB"))
                    cpu_opp.append(load)
                elif simulator == "vadere":
                    ram_vadere.append(ram)
                    cpu_vadere.append(load)
                elif simulator == "sumo":
                    ram_sumo.append(ram)
                    cpu_sumo.append(load)
                    
                    
    print(f"RAM OPP: min: {humanfriendly.format_size(min(ram_opp))}, max: {humanfriendly.format_size(max(ram_opp))}, mean: {humanfriendly.format_size(np.mean(ram_opp))}, stdev: {humanfriendly.format_size(np.std(ram_opp))}")
    print(f"RAM Vadere: min: {humanfriendly.format_size(min(ram_vadere))}, max: {humanfriendly.format_size(max(ram_vadere))}, mean: {humanfriendly.format_size(np.mean(ram_vadere))}, stdev: {humanfriendly.format_size(np.std(ram_vadere))}")
    print(f"RAM Sumo: min: {humanfriendly.format_size(min(ram_sumo))}, max: {humanfriendly.format_size(max(ram_sumo))}, mean: {humanfriendly.format_size(np.mean(ram_sumo))}, stdev: {humanfriendly.format_size(np.std(ram_sumo))}")   
    
    print(f"Load OPP: min: {min(cpu_opp)}%, max: {max(cpu_opp)}%, mean: {np.mean(cpu_opp)}%, stdev: {np.std(cpu_opp)}")
    print(f"Load Vadere: min: {min(cpu_vadere)}%, max: {max(cpu_vadere)}%, mean: {np.mean(cpu_vadere)}%, stdev: {np.std(cpu_vadere)}")
    print(f"Load Sumo: min: {min(cpu_sumo)}%, max: {max(cpu_sumo)}%, mean: {np.mean(cpu_sumo)}%, stdev: {np.std(cpu_sumo)}")
          
if __name__ == "__main__":
    print("======== Combined ========")
    main("*Combined*.txt")
    print("======== Sumo ========")
    main("*Sumo*.txt")
     
