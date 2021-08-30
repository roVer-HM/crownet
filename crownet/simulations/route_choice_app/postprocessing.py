from flowcontrol.strategy.sensor.density import DensityMapCheck

PRECISION = 8
import os

if __name__ == "__main__":
    working_dir = dict()
    working_dir["path"] = "/home/christina/repos/crownet/crownet/simulations/route_choice_app/results/final_20210830-11:35:16"

    density_file_2: str = os.path.join(working_dir["path"], "countsCellwise.txt")
    global_csv = os.path.join(os.path.dirname(working_dir["path"]), "global.csv")

    vadere_counts = DensityMapCheck.get_cell_density_vadere_truth(file_path = density_file_2)
    omnet_counts = DensityMapCheck.get_cell_density_omnett_truth(file_path = global_csv)