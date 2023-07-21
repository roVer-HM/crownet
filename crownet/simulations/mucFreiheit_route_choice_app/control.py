import os
import sys

from shapely.geometry import Polygon

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.dataprocessor.dataprocessor import *
from flowcontrol.strategy.controlaction import InformationStimulus, Rectangle, StimulusInfo, Location, TimeFrame
from flowcontrol.strategy.sensor.density import MeasurementArea, DensityMapper

sys.path.append(os.path.abspath(".."))

from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.strategy.timestepping.timestepping import FixedTimeStepper
from flowcontrol.strategy.controller.control_algorithm import AvoidCongestion


class AvoidShort(Controller):

    def __init__(self):

        self.density_over_time = list()

        super().__init__(
            control_algorithm=AvoidCongestion(alternative_route_id=31),
            time_stepper=FixedTimeStepper(time_step_size=2.0, start_time=10.0),
            processor_manager=Manager()
        )

    def handle_init(self, sim_time, sim_state):
        self.densityMapper = None
        self.detour_app = "World.misc[0].app[0]"
        self.density_app = "World.misc[0].app[1].app"

        self.con_manager.domains.v_sim.init_control()
        self.processor_manager.registerProcessor("path_choice", CorridorRecommendation(
            Writer(os.path.join(self.output_dir, "path_choice.txt"))))

        self.processor_manager.registerProcessor("densities_mapped",
                                                 DensityMeasurements(Writer(os.path.join(self.output_dir, "densities_mapped.txt")),
                                                                     user_defined_header=["meas_area_1", "meas_area_2", "meas_area_3"]))

    def handle_sim_step(self, sim_time, sim_state):
        self.processor_manager.update_sim_time(sim_time)
        self.measure_state(sim_time)
        self.send_recommendation_if_necessary(sim_time)
        self.commandID += 1
        self.time_stepper.forward_time()

    def measure_state(self, sim_time):
        cell_dim, cell_size, result = self.con_manager.domains.v_sim.get_density_map(
            sending_node=self.density_app)
        self.update_density_map(cell_dim, cell_size, result)
        densities = list(self.densityMapper.get_density_in_area(distribution="uniform").values())
        self.processor_manager.write("densities_mapped", *densities)
        self.density_over_time.append(densities)

    def update_density_map(self, cell_dim, cell_size, result):
        if self.densityMapper is None:
            obs = self._get_obstacles_as_polygons()
            measurement_areas = self._get_measurement_areas([1, 2, 3])
            self.densityMapper = DensityMapper(cell_dimensions=cell_dim, cell_size=cell_size,
                                               measurement_areas=measurement_areas, obstacles=obs)
        if result is not None:
            self.densityMapper.update_density(result)

    def _get_measurement_areas(self, measurement_area_ids):
        areas = dict()
        for measurement_id in measurement_area_ids:
            polygon = self.con_manager.domains.v_polygon.get_measurement_area_shape(str(measurement_id))
            areas[measurement_id] = MeasurementArea(polygon=Polygon(np.array(polygon)), id=measurement_id)
        return areas

    def _get_obstacles_as_polygons(self):
        obs = self.con_manager.domains.v_sim.get_obstacles()
        obstacles = list()
        for __, polygon in obs.items():
            polygon = Polygon(np.array(polygon))
            obstacles.append(polygon)
        return obstacles

    def write_data(self):
        self.processor_manager.finish()

    def _get_stimulus_info(self, sim_time):
        location = Location(areas=Rectangle(x=58.2, y=71.1, width=20., height=15.))
        recommendation = InformationStimulus(f"use target {self.current_target}")
        start_time = sim_time
        end_time = start_time + 10000  # TODO adjust the value here if you adjust the time stepping
        timeframe = TimeFrame(start_time=sim_time, end_time=end_time)
        s = StimulusInfo(location=location, stimuli=recommendation, timeframe=timeframe)
        return s

    def send_recommendation_if_necessary(self, sim_time):
        densities = self.density_over_time[-1]
        density_short_route = densities[0]
        density_long_route = densities[2]
        self.current_target = self.control_algorithm.get_next_target( density_congested_route = density_short_route,
                                                                      density_alternate_route = density_long_route)
        print(f"Densities measured in routes:  short , medium , long : {densities}.")

        if self.current_target is None:
            print(f"Sim time {sim_time}:\t route recommendation opportunity skipped. Do not send information.")
            self.processor_manager.write("path_choice", "skipped")
        else:
            print(f"Sim time {sim_time}:\t Recommend route with intermediate target id = {self.current_target}")
            self.processor_manager.write("path_choice", self.current_target)

            recommendation = self._get_stimulus_info(sim_time)
            self.con_manager.domains.v_sim.send_control(message=recommendation.toJSON(),
                                                    sending_node_id=self.detour_app)


if __name__ == "__main__":

    if len(sys.argv) == 1:
        # default settings for control-vadere (no Omnetpp!)
        settings = ["--controller-type",
                    "AvoidShort",  #
                    "--port",
                    "9997",
                    "--host-name",
                    "0.0.0.0"  #
                    ]
    else:
        settings = sys.argv[1:]

    print(settings)

    sub = VadereDefaultStateListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION}, init_sub=True,
    )

    controller = get_controller_from_args(working_dir=os.getcwd(), args=settings)
    controller.register_state_listener("default", sub, set_default=True)
    controller.start_controller()
