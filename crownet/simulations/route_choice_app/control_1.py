import os
import sys
import time

from typing import Union

import numpy as np
import json

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener

sys.path.append(os.path.abspath(".."))

from flowcontrol.strategy.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc
from flowcontrol.utils.misc import get_scenario_file

from shapely.geometry import Polygon, Point

import pandas as pd
import matplotlib.pyplot as plt

working_dir = dict()
PRECISION = 8

class MeasurementArea:

    def __init__(self, id, polygon: Polygon):
        self.id = id
        self.area = polygon
        self.cell_contribution = dict()

    def get_cell_contribution(self, cells):
        if len(self.cell_contribution) > 0:
            return self.cell_contribution

        print(f"Initialize cell contributions for measurement area with id = {self.id}.")

        for key, cell in cells.items():
            common_area = cell.polygon.intersection(self.area).area

            if common_area > 0:
                self.cell_contribution[key] = common_area/cell.polygon.area

        return self.cell_contribution



class Cell:
    def __init__(self, id, polygon : Polygon, count=0):
        self.id = id
        self.polygon = polygon
        self.number_of_agents_in_cell = count

    def get_density(self):
        return self.number_of_agents_in_cell/self.polygon.area

    def get_count(self):
        return self.number_of_agents_in_cell

    def set_count(self, count):
        self.number_of_agents_in_cell = count

class DensityMapper:

    def __init__(self, cell_dimensions, cell_size, measurement_areas : dict):
        self.cell_dimensions = cell_dimensions
        self.cell_size = cell_size
        self.cells = dict()
        self.measurement_areas = measurement_areas

    def get_cells(self):
        if len(self.cells) > 0:
            return self.cells

        print("Initialize cell grid.")

        delta_x = self.cell_size[0]
        delta_y = self.cell_size[1]

        index = 0

        for x_coor in np.arange(0, self.cell_dimensions[0])*delta_x:
            for y_coor in np.arange(0, self.cell_dimensions[1])*delta_y:
                lower_left_corner = [x_coor,y_coor]
                lower_right_corner = [x_coor+delta_x, y_coor]
                upper_right_corner = [x_coor+delta_x, y_coor+delta_y]
                upper_left_corner = [x_coor, y_coor+delta_y]
                polygon = Polygon(np.array([lower_left_corner, lower_right_corner, upper_right_corner, upper_left_corner]))
                key = self.get_cell_key(x_coor, y_coor)
                self.cells[key] = Cell(id=index, polygon=polygon)
                index += 1

        return self.cells

    def get_cell_key(self, x_coor, y_coor):
        return f"x={x_coor:.1f}_y={y_coor:.1f}"

    def get_density_in_area(self, distribution = "uniform"):

        if distribution=="uniform":
            densities = self.get_density_uniform_assumption()
        else:
            raise NotImplementedError("Not implemented yet. Use distribution type uniform.")
        return densities

    def get_density_uniform_assumption(self):

        densities = dict()

        for id, measurement_area in self.measurement_areas.items():

            area, counts = self.compute_counts_area(measurement_area)

            densities[id] = area/counts

    def compute_counts_area(self, measurement_area : MeasurementArea):
        cell_contributions = measurement_area.get_cell_contribution(self.get_cells())

        count = 0
        unit_area = 0

        for key, weight in cell_contributions.items():
            count_raw = self.get_cells()[key].get_count()
            count += count_raw*weight # weights the counts -> if 50% of the cell area overlaps with the measurement area, the weight would be 50%
            unit_area += weight

        if not np.isclose(unit_area*self.get_cell_area(), measurement_area.area.area):
            raise ValueError(f"Measurement area computed: {unit_area*self.get_cell_area()}. Should be {measurement_area.area.area}.")

        return count, unit_area*self.get_cell_area()


    def get_cell_area(self):
        return self.cell_size[0]*self.cell_size[1]

    def update_density(self, result):
        for entry in result:
            self.update_cell(x_coor=entry[0], y_coor = entry[1], count= entry[2])

    def update_cell(self, x_coor, y_coor, count):
        cell_key = self.get_cell_key(x_coor, y_coor)

        if cell_key not in self.get_cells().keys():
            raise ValueError(f"Key {cell_key} not found.")
        self.get_cells()[cell_key].set_count(count)


class NoController(Controller):
    def __init__(self):
        super().__init__()
        print("Dummy controller.")

        self.density_over_time = list()
        self.time_step = list()
        self.sensor_time_step_size = 0.4
        self.corridor_choice_over_time = list()
        self.next_call = 0
        self.time_step_size = 10

        self.densityMapper = None

    def handle_sim_step(self, sim_time, sim_state):

        self.measure_state(sim_time)

        # necessary, because time intervals for sensoring and applying a control action differ
        if (sim_time-self.sensor_time_step_size) % self.time_step_size == 0:
            self.compute_next_corridor_choice(sim_time)
            self.apply_redirection_measure()

        self.next_call += self.sensor_time_step_size
        self.con_manager.next_call_at(self.next_call)

    def measure_state(self, sim_time):

        cell_dim, cell_size, result = self.con_manager.domains.v_sim.get_density_map(sending_node="misc[0].app[1].app")
        self.update_density_map(cell_dim, cell_size, result)

        densities = self.densityMapper.get_density_in_area(distribution="uniform")

        time_step = (
            np.round(sim_time / self.sensor_time_step_size, 0) + 1
        )  # time = 0.0s := timestep 1, time step size: 0.4

        # print(f"Sim-time: {sim_time}, timeStep: {time_step}) \t Density measured: {density}")
        self.density_over_time.append(np.array(densities.values()))
        self.time_step.append(time_step)

    def update_density_map(self, cell_dim, cell_size, result):
        if self.densityMapper is None:
            self.densityMapper = DensityMapper(cell_dimensions=cell_dim, cell_size=cell_size, measurement_areas=self.measurement_areas)
        self.densityMapper.update_density(result)

    def apply_redirection_measure(self):
        pass

    def handle_init(self, sim_time, sim_state):
        self.counter = 0
        self.con_manager.next_call_at(0.01) #set to 0.4, 0.01 -> test case
        self.measurement_areas = self._get_measurement_areas([1, 2, 3, 4, 5])

    def check_density_measures(self):
        self.measure_state(self.next_call + self.sensor_time_step_size)
        index = pd.Index(data=self.time_step, name="timeStep", dtype=int)
        densities = pd.DataFrame(
            data=np.array(self.density_over_time),
            columns=[f"areaDensityCountingNormed-PID{x}" for x in [14, 15, 16, 17, 18]],
            index=index,
        ).round(PRECISION)

        density_file = os.path.join(working_dir["path"], "densities.txt")
        while os.path.isfile(density_file) is False:
            time.sleep(1)
        time.sleep(1)
        
        dens_check = (
            pd.read_csv(
                density_file,
                delimiter=" ",
                index_col=[0],
            )
            .sort_index(axis=1)
            .round(PRECISION)
        )
        if densities.equals(dens_check) is False:

            if densities[:-1].equals(dens_check[:-1]) is False:
                raise ValueError(
                    "Densities from data processors do not equal THESE densities."
                )
            else:
                print(
                    "INFO: Simulation time might differ for last time step. Skipped time step in comparison."
                )

    def _get_measurement_areas(self, measurement_area_ids):
        areas = dict()
        for measurement_id in measurement_area_ids:
            polygon = self.con_manager.domains.v_polygon.get_shape(str(measurement_id))
            areas[measurement_id] = MeasurementArea(polygon=Polygon(np.array(polygon)), id=measurement_id)

        return areas

    def collect_data(self):

        self.check_density_measures()
        self.plot_densities()

    def plot_densities(self):

        densities = pd.read_csv(
            os.path.join(working_dir["path"], "densities.txt"),
            delimiter=" ",
            index_col=[0],
        )

        dataprocessor_name_density = "areaDensityCountingNormed-PID"
        corridor_name = "Corridor"
        dataprocessormapping_density = {
            f"{dataprocessor_name_density}14": f"{corridor_name} 1",  # shortest path
            f"{dataprocessor_name_density}15": f"{corridor_name} 2",
            f"{dataprocessor_name_density}16": f"{corridor_name} 3",
            f"{dataprocessor_name_density}17": f"{corridor_name} 4",
            f"{dataprocessor_name_density}18": f"{corridor_name} 5",  # longest path
        }

        densities.index = densities.index * self.sensor_time_step_size
        cut = 250 # cut off simulation results before simulation time t=100s (250*0.4)
        interval = 1000 # cut off simulation results before  after t=100s+400s=500s (1000*0.4)
        # densities
        densities = densities.iloc[cut : cut + interval, :]
        densities.rename(columns=dataprocessormapping_density, inplace=True)
        densities.sort_index(axis=1, inplace=True)

        densities.plot()
        plt.legend(loc='upper left')
        plt.xlim([0, self.sensor_time_step_size*(cut+interval)])
        plt.xlabel("Simulation time [s]")
        plt.ylabel("Density [1/m^2]")
        plt.title(f"Density over time")
        plt.ylim([0, 2])
        plt.legend()
        plt.savefig(os.path.join(working_dir["path"], f"{os.path.basename(working_dir['path'])}_density_over_time.png"))
        plt.show(block=False)
        plt.close()

        densities.boxplot()
        plt.ylabel("Density [1/m^2]")
        plt.title(
            f"Number of pedestrians (sample size = {len(densities)})"
        )
        plt.ylim([0, 2])
        plt.savefig(os.path.join(working_dir["path"], f"{os.path.basename(working_dir['path'])}_density_distribution.png"))
        plt.show(block=False)
        plt.close()

    def compute_next_corridor_choice(self, sim_time):
        pass

    def set_reaction_model_parameters(self, reaction_probability):
        pass

    def get_mapped_counts_from_density_map(self, cell_dim, density, a):
        #approximate the density in a specified area by using the counts stored in the omnetpp/density map
        density_mapper = self.densityMapper.get_density_in_area()

        return density



class OpenLoop(NoController, Controller):
    def __init__(self):
        super().__init__()
        self.target_ids = [11, 21, 31, 41, 51]
        self.redirected_agents = list()
        self.controlModelType = "RouteChoice"
        self.controlModelName = "distributePeds"
        self.commandID = 111

        self.reaction_model = {
            "isBernoulliParameterCertain": True,
            "BernoulliParameter": 1.0,
        }


    def collect_data(self):

        self.check_density_measures()
        self.path_choice()
        self.plot_densities()


    def apply_redirection_measure(self):

        probabilities = [0, 0, 0, 0, 0]
        probabilities[self.counter] = 1.0 # all of the agents should use one specific corridor

        command = {"targetIds": self.target_ids, "probability": probabilities}
        action = {
            "commandId": self.commandID,
            "command": command,
            "space": {"x": 0.5, "y": 0.5, "width": 5, "height": 15}, # get information direclty after spawning process
        }
        action = json.dumps(action)
        self.commandID += 1
        self.con_manager.domains.v_sim.send_control(message=action, model=self.controlModelName, sending_node_id="misc[0].app[0].app")


    def _increase_counter(self):
        self.counter += 1
        self._reset_counter()

    def _reset_counter(self):
        if self.counter >= len(self.target_ids):
            self.counter = 0

    def compute_next_corridor_choice(self, sim_time):

        old_counter = self.counter

        self.choose_corridor()

        print(f"Use corridor {self.target_ids[self.counter]}.")

        self.corridor_choice_over_time.append([sim_time, old_counter, self.counter])

    def choose_corridor(self):
        self._increase_counter()

    def path_choice(self):

        corridor_corrections = pd.DataFrame(
            data=np.array(self.corridor_choice_over_time),
            columns=["timeStep", "OldCorridor", "NewCorridor"],
        )
        corridor_corrections.set_index("timeStep", inplace=True)
        corridor_corrections.to_csv(
            os.path.join(working_dir["path"], "path_choice.txt")
        )

        plt.scatter(
            corridor_corrections.index.to_numpy(),
            corridor_corrections["NewCorridor"] + 1,
        )
        plt.ylim([0.5, 5.5])
        plt.xlim([0, 500])
        plt.yticks([1, 2, 3, 4, 5])
        plt.xlabel("Simulation time [s]")
        plt.ylabel("Corridor recommendation")
        plt.savefig(os.path.join(working_dir["path"], f"{os.path.basename(working_dir['path'])}_path_choice.png"))
        plt.show(block=False)
        plt.close()

    def get_reaction_model_parameters(self):
        return json.dumps(
          self.reaction_model
        )

    def set_reaction_model_parameters(self, reaction_probability):
        self.reaction_model["BernoulliParameter"] = reaction_probability



    def handle_init(self, sim_time, sim_state):
        self.con_manager.domains.v_sim.init_control(
            self.controlModelName, self.controlModelType, self.get_reaction_model_parameters()
        )
        super().handle_init(sim_time, sim_state)


class ClosedLoop(OpenLoop, Controller):
    def __init__(self):
        super().__init__()

    def choose_corridor(self):
        # measure densities in corridors
        # average densities between two controller calls
        number_of_time_steps_for_measurement = int(
            np.round(self.time_step_size / self.sensor_time_step_size, 0)
        )
        densities = np.array(
            self.density_over_time[-number_of_time_steps_for_measurement:]
        ).mean(axis=0)
        # set old corridor to inf to avoid congestion:
        densities[self.counter] = np.inf
        self.counter = np.argwhere(densities == densities.min()).ravel()[-1]

        print(
            f"The densities are: {densities.round(3)}. Minimum: index = {self.counter}."
        )


def main(
    settings, controller_type="OpenLoop", scenario="simplified_default_sequential", reaction_probability=1.0
):

    sub = VadereDefaultStateListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION}, init_sub=True,
    )

    scenario_file = get_scenario_file(f"vadere/scenarios/{scenario}.scenario")
    kwargs = {
        "file_name": scenario_file,
    }

    working_dir["path"] = os.path.join(os.getcwd(), f"{scenario}_{controller_type}_prob_{int(reaction_probability)}")

    settings_ = settings
    settings_.extend(["--output-dir", working_dir["path"]])
    settings_.extend(["--controller-type", controller_type])

    controller = get_controller_from_args(working_dir=os.getcwd(), args=settings_)
    controller.register_state_listener("default", sub, set_default=True)
    controller.set_reaction_model_parameters(reaction_probability=reaction_probability)
    controller.start_controller(**kwargs)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        # settings = [
        #     "--port",
        #     "9999",
        #     "--host-name",
        #     "localhost",
        #     "--client-mode",
        #     "--start-server",
        #     "--gui-mode",
        #     "--path-to-vadere-repo",
        #     os.path.abspath("../../../vadere"),
        #     "--suppress-prompts",
        # ]

        settings = [
            "--port",
            "9997",
            "--host-name",
            "0.0.0.0",
        ]

        main(
            settings,
            controller_type="NoController",
            scenario="simplified_default_sequential",
        )
        # main(
        #     settings,
        #     controller_type="OpenLoop",
        #     scenario="simplified_default_sequential",
        # )
        # main(
        #     settings,
        #     controller_type="ClosedLoop",
        #     scenario="simplified_default_sequential",
        # )
        # main(
        #     settings,
        #     controller_type="OpenLoop",
        #     scenario="simplified_default_sequential_disturbance",
        # )
        # main(
        #     settings,
        #     controller_type="ClosedLoop",
        #     scenario="simplified_default_sequential_disturbance",
        # )

    else:
        settings = sys.argv[1:]
