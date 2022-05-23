import os
import sys
import json

from shapely.geometry import Polygon

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.crownetcontrol.traci.connection_manager import ClientModeConnection, ServerModeConnection
from flowcontrol.strategy.sensor.density import MeasurementArea, DensityMapper
from flowcontrol.dataprocessor.dataprocessor import *
from flowcontrol.strategy.controlaction import InformationStimulus, Rectangle, StimulusInfo, Location

sys.path.append(os.path.abspath(".."))

from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.strategy.timestepping.timestepping import FixedTimeStepper
from flowcontrol.strategy.controller.control_algorithm import AlternateTargetAlgorithm, MinimalDensityAlgorithm

class NoController(Controller):

    def __init__(self, control_alg=None):
        super().__init__(
            control_algorithm=control_alg,
            time_stepper=FixedTimeStepper(time_step_size=10.0, start_time=10.0),
            processor_manager=Manager()
        )
        self.density_over_time = list()

    def handle_sim_step(self, sim_time, sim_state):
        self.processor_manager.update_sim_time(sim_time)
        self.measure_state(sim_time)
        self.compute_next_corridor_choice()
        self.apply_redirection_measure()
        self.commandID += 1
        self.time_stepper.forward_time()

    def compute_next_corridor_choice(self):
        pass

    def measure_state(self, sim_time):
        if isinstance(self.con_manager, ServerModeConnection):
            cell_dim, cell_size, result = self.con_manager.domains.v_sim.get_density_map(
                sending_node="gloablDensityMap")  # "misc[0].app[1].app" OR "gloablDensityMap"
            self.update_density_map(cell_dim, cell_size, result)
            densities = self.densityMapper.get_density_in_area(distribution="uniform").values()
        elif isinstance(self.con_manager, ClientModeConnection):
            densities = list()
            for a in [25, 24, 23]:
                __, __, density = self.con_manager.domains.v_sim.get_data_processor_value(str(a))
                # density = -11.0 #TODO replace
                densities.append(density)
        else:
            raise ValueError("Cannot handle send_ctl command.")
        self.density_over_time.append(densities)

    def update_density_map(self, cell_dim, cell_size, result):
        if self.densityMapper is None:
            obs = self._get_obstacles_as_polygons()
            measurement_areas = self._get_measurement_areas([1, 3, 5])
            self.densityMapper = DensityMapper(cell_dimensions=cell_dim, cell_size=cell_size,
                                               measurement_areas=measurement_areas, obstacles=obs)
        if result is not None:
            self.densityMapper.update_density(result)

    def _get_measurement_areas(self, measurement_area_ids):
        areas = dict()
        for measurement_id in measurement_area_ids:
            polygon = self.con_manager.domains.v_polygon.get_shape(str(measurement_id))
            areas[measurement_id] = MeasurementArea(polygon=Polygon(np.array(polygon)), id=measurement_id)
        return areas

    def _get_obstacles_as_polygons(self):
        obs = self.con_manager.domains.v_sim.get_obstacles()
        obstacles = list()
        for __, polygon in obs.items():
            polygon = Polygon(np.array(polygon))
            obstacles.append(polygon)
        return obstacles

    def apply_redirection_measure(self):
        pass

    def handle_init(self, sim_time, sim_state):
        self.processor_manager.registerProcessor("commandId", ControlActionCmdId(
            writer=Writer(os.path.join(self.output_dir, "commandIds.txt"))))

    def write_data(self):
        self.processor_manager.finish()


class OpenLoop(NoController, Controller):

    def __init__(self, control_alg = None):
        self.target_ids = [31, 21, 11]
        self.redirected_agents = list()
        self.commandID = 111
        self.redirection_area = Rectangle(x=180., y=190., width=20., height=15.)

        if control_alg == None:
            control_alg= AlternateTargetAlgorithm(alternate_targets=self.target_ids)
        super().__init__(control_alg=control_alg)


    def get_stimulus_info(self):

        location = Location(areas=self.redirection_area)
        recommendation = InformationStimulus(f"use target [{self.current_target}]")
        s = StimulusInfo(location=location, stimuli=recommendation)
        return s


    def measure_state(self, sim_time):
        commandIds = self.con_manager.domains.v_sim.get_received_command_ids(22)
        self.processor_manager.write("sending_times", commandIds)
        super().measure_state(sim_time)

    def write_data(self):
        commandIdsSent = self.processor_manager.get_processor_values("commandId")
        self.processor_manager.post_loop("sending_times", commandIdsSent)
        super().write_data()


    def apply_redirection_measure(self):
        self.compute_next_corridor_choice()
        recommendation = self.get_stimulus_info()
        self.processor_manager.write("commandId", self.commandID)

        if isinstance(self.con_manager, ServerModeConnection):
            self.con_manager.domains.v_sim.send_control(message=recommendation.toJSON(), sending_node_id="misc[0].app[0]")
        else:
            self.con_manager.domains.v_sim.send_control(message=recommendation.toJSON())

    def compute_next_corridor_choice(self):
        self.current_target = self.control_algorithm.get_next_target()


    def handle_init(self, sim_time, sim_state):
        super().handle_init(sim_time, sim_state)
        self.con_manager.domains.v_sim.init_control()

        self.processor_manager.registerProcessor("sending_times", SendingTimes(
            Writer(os.path.join(self.output_dir, "sending_times.txt"))))
        self.processor_manager.registerProcessor("path_choice", CorridorRecommendation(
            Writer(os.path.join(self.output_dir, "path_choice.txt"))))


class ClosedLoop(OpenLoop, Controller):
    def __init__(self):
        self.target_ids = [31, 21, 11]
        control_alg = MinimalDensityAlgorithm(alternate_targets=self.target_ids)
        super().__init__(control_alg=control_alg)


    def compute_next_corridor_choice(self):
        self.current_target = self.control_algorithm.get_next_target(self.density_over_time[-1])
        self.processor_manager.write("path_choice", self.current_target)



if __name__ == "__main__":

    if len(sys.argv) == 1:
        # default settings for control-vadere (no Omnetpp!)
        settings = ["--controller-type",
                    "OpenLoop",  #
                    "--scenario-file",
                    "route_choice_real_world.scenario",
                    "--port",
                    "9999",
                    "--host-name",
                    "vadere",
                    "--client-mode",
                    # "--start-server",
                    # "--gui-mode",
                    "--output-dir",
                    "sim-output-task1",
                    # "-j",
                    # "/home/christina/repos/crownet/vadere/VadereManager/target/vadere-server.jar"
                    ]
    else:
        settings = sys.argv[1:]

    sub = VadereDefaultStateListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION}, init_sub=True,
    )
    controller = get_controller_from_args(working_dir=os.getcwd(), args=settings)
    controller.register_state_listener("default", sub, set_default=True)
    controller.start_controller()
