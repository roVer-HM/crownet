import os
import sys
import time

import numpy as np
import json

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.crownetcontrol.traci.connection_manager import ClientModeConnection, ServerModeConnection
from flowcontrol.strategy.sensor.density import MeasurementArea, DensityMapper

sys.path.append(os.path.abspath(".."))

from flowcontrol.strategy.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from shapely.geometry import Polygon, Point

import pandas as pd
import matplotlib.pyplot as plt

working_dir = dict()
PRECISION = 8


class NoController(Controller):
    def __init__(self):
        super().__init__()

        self.density_over_time = list()
        self.time_step = list()
        self.sensor_time_step_size = 0.4
        self.corridor_choice_over_time = list()
        self.next_call = 0.0
        self.time_step_size = 10.0

        self.densityMapper = None

    def handle_sim_step(self, sim_time, sim_state):
        print(f"handle_sim_step: simulation time = {sim_time}.")

        self.measure_state(sim_time)

        # necessary, because time intervals for sensoring and applying a control action differ
        if (sim_time - self.sensor_time_step_size) % self.time_step_size == 0:
            print(f"Current Simtime: {sim_time - self.sensor_time_step_size}, apply control measure at {sim_time}")
            self.compute_next_corridor_choice(sim_time)
            self.apply_redirection_measure(sim_time + 0.4)  # vadere time step size

        self.next_call += self.sensor_time_step_size
        self.con_manager.next_call_at(self.next_call)

    def measure_state(self, sim_time):
        if isinstance(self.con_manager, ServerModeConnection):
            cell_dim, cell_size, result = self.con_manager.domains.v_sim.get_density_map(
                sending_node="gloablDensityMap")  # "misc[0].app[1].app" OR "gloablDensityMap"

            self.update_density_map(cell_dim, cell_size, result)

            densities = self.densityMapper.get_density_in_area(distribution="uniform").values()
        elif isinstance(self.con_manager, ClientModeConnection):
            densities = list()
            peds1 = self.con_manager.sub_listener["default"].pedestrians
            for a in self.measurement_areas.values():
                counts = len([p["id"] for p in peds1 if a.area.contains(Point(p["pos"][0], p["pos"][1]))])
                densities.append(counts / a.area.area)
        else:
            raise ValueError("Cannot handle send_ctl command.")

        time_step = (
                np.round(sim_time / self.sensor_time_step_size, 0) + 1
        )

        print(f"Sim-time: {sim_time}, timeStep: {time_step}) \t Density measured: {densities}")
        self.density_over_time.append(np.array(list(densities)))
        self.time_step.append(time_step)

    def update_density_map(self, cell_dim, cell_size, result):
        if self.densityMapper is None:
            obs = self.get_obstacles_as_polygons()
            self.densityMapper = DensityMapper(cell_dimensions=cell_dim, cell_size=cell_size,
                                               measurement_areas=self.measurement_areas, obstacles=obs)
        if result is not None:
            self.densityMapper.update_density(result)

    def get_obstacles_as_polygons(self):
        obs = self.con_manager.domains.v_sim.get_obstacles()
        obstacles = list()
        for __, polygon in obs.items():
            polygon = Polygon(np.array(polygon))
            obstacles.append(polygon)
        return obstacles

    def apply_redirection_measure(self, executation_time):
        pass

    def handle_init(self, sim_time, sim_state):
        self.counter = 0
        working_dir["path"] = self.con_manager.domains.v_sim.get_output_directory()
        print(f"Output is stored in {working_dir['path']}.")
        self.con_manager.next_call_at(self.next_call)
        print(f"Set next call to: {self.next_call}")
        self.measurement_areas = self._get_measurement_areas([1, 2, 3, 4, 5])

    def _get_measurement_areas(self, measurement_area_ids):
        areas = dict()

        for measurement_id in measurement_area_ids:
            polygon = self.con_manager.domains.v_polygon.get_shape(str(measurement_id))
            areas[measurement_id] = MeasurementArea(polygon=Polygon(np.array(polygon)), id=measurement_id)

        return areas

    def postprocess_sim_results(self):
        self.write_data()
        self.check_data()
        self.plot_data()

    def write_data(self):
        self.write_densities()

    def write_densities(self):
        index = pd.Index(data=self.time_step, name="timeStep", dtype=int)
        densities = pd.DataFrame(
            data=np.array(self.density_over_time),
            columns=[f"areaDensityCountingNormed-PID{x}" for x in [14, 15, 16, 17, 18]],
            index=index,
        ).round(PRECISION)
        densities.to_csv(
            os.path.join(working_dir["path"], "densities_measurement_areas_flowcontrol.txt"), index=True, sep=" ",
        )

    def plot_data(self):
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
        cut = 250  # cut off simulation results before simulation time t=100s (250*0.4)
        interval = 1000  # cut off simulation results before  after t=100s+400s=500s (1000*0.4)
        # densities
        densities = densities.iloc[cut: cut + interval, :]
        densities.rename(columns=dataprocessormapping_density, inplace=True)
        densities.sort_index(axis=1, inplace=True)

        densities.plot()
        plt.legend(loc='upper left')
        plt.xlim([0, self.sensor_time_step_size * (cut + interval)])
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
        plt.savefig(
            os.path.join(working_dir["path"], f"{os.path.basename(working_dir['path'])}_density_distribution.png"))
        plt.show(block=False)
        plt.close()

    def compute_next_corridor_choice(self, sim_time):
        pass

    def set_reaction_model_parameters(self, reaction_probability):
        pass

    def check_data(self):
        pass


class OpenLoop(NoController, Controller):
    def __init__(self):
        super().__init__()
        self.target_ids = [11, 21, 31, 41, 51]
        self.redirected_agents = list()
        self.controlModelType = "RouteChoice"
        self.controlModelName = "distributePeds"
        self.commandID = 111
        self.commandIDwriter = list()

        self.reaction_model = {
            "isBernoulliParameterCertain": True,
            "BernoulliParameter": 1.0,
        }

    def apply_redirection_measure(self, executionTime):
        probabilities = [0, 0, 0, 0, 0]
        probabilities[self.counter] = 1.0  # all of the agents should use one specific corridor

        command = {"targetIds": self.target_ids,
                   "probability": probabilities,
                   "reactionProbability": list(
                       np.ones(len(self.target_ids)) * self.reaction_model['BernoulliParameter'])
                   }
        action = {
            "commandId": self.commandID,
            "command": command,
            "space": {"x": 0.5, "y": 0.5, "width": 5, "height": 15},  # get information direclty after spawning process
        }
        action = json.dumps(action)
        self.commandIDwriter.append([np.round(executionTime/0.4), self.commandID])

        self.commandID += 1
        self.con_manager.domains.v_sim.send_control(message=action, model=self.controlModelName)

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

    def plot_data(self):
        super().plot_data()
        self.plot_path_choice()

    def plot_path_choice(self):
        corridor_corrections = pd.read_csv(os.path.join(working_dir["path"], "path_choice.txt"))
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

    def write_data(self):
        super().write_data()
        self.write_path_choice()
        self.write_commandIds()
        self.write_sendingTime()

    def write_path_choice(self):
        corridor_corrections = pd.DataFrame(
            data=np.array(self.corridor_choice_over_time),
            columns=["timeStep", "OldCorridor", "NewCorridor"],
        )
        corridor_corrections.set_index("timeStep", inplace=True)
        corridor_corrections.to_csv(
            os.path.join(working_dir["path"], "path_choice.txt")
        )

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

    def write_commandIds(self):
        key = "timeStep"
        commandIds = pd.DataFrame(
            data=np.array(self.commandIDwriter),
            columns=[key, "commandId"],
            dtype=int
        )
        commandIds.set_index(key, inplace=True)
        commandIds.to_csv(
            os.path.join(working_dir["path"], "commandIds.txt")
        )

    def write_sendingTime(self):
        sent = pd.read_csv(os.path.join(working_dir["path"], "commandIds.txt"), index_col=[1])
        received = pd.read_csv(os.path.join(working_dir["path"], "commandIdsReceived.txt"), index_col=[0], sep=" ")

        # TODO iterate through sent, not received
        for commandId, receivedTimeStep in received.groupby("commandId-PID22").groups.items():
            if commandId > 0:
                sentTimeStep = sent.loc[commandId, :].values[0]
                sending_times = pd.DataFrame(data = (receivedTimeStep-sentTimeStep-1)*self.sensor_time_step_size,
                                             )
                print(sending_times)

            print()

        print()




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


if __name__ == "__main__":

    isUseOmnet = False
    isRunInDockerContainer = True
    scenario_file = "simplified_default_sequential.scenario"
    settings = ["--controller-type", "OpenLoop",
                "--scenario-file", scenario_file ,
                "--experiment-label", f"no_disturbance_openControl_{time.time()}"] #TODO change this

    _settings = list()
    if len(sys.argv) == 1:
        if isUseOmnet:
            _settings = [
                "--port",
                "9997",
                "--host-name",
                "0.0.0.0",
            ]
        else:
            if isRunInDockerContainer:
                _settings = [
                    "--port",
                    "9999",
                    "--host-name",
                    "vadere",
                    "--client-mode",
                ]
            else:
                _settings = [
                    "--port",
                    "9999",  # 9999
                    "--host-name",
                    "localhost",
                    "--client-mode",
                    "--start-server",
                    "--gui-mode",
                    "--path-to-vadere-repo",
                    os.path.abspath("../../../vadere"),
                    "--suppress-prompts",
                ]
            _settings.extend(["--output-dir", os.path.join(os.path.abspath(os.path.dirname(__file__)), "results")])

    else:
        settings_ = sys.argv[1:]

    settings.extend(_settings)

    sub = VadereDefaultStateListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION}, init_sub=True,
    )
    controller = get_controller_from_args(working_dir=os.getcwd(), args=settings)
    controller.register_state_listener("default", sub, set_default=True)
    controller.set_reaction_model_parameters(reaction_probability=1.0)
    controller.start_controller()



