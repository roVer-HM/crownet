import os
import sys
import json

from shapely.geometry import Polygon

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.crownetcontrol.traci.connection_manager import ClientModeConnection, ServerModeConnection
from flowcontrol.strategy.sensor.density import MeasurementArea, DensityMapper
from flowcontrol.dataprocessor.dataprocessor import DensityMeasurements, Writer, Manager, CorridorRecommendation
from flowcontrol.strategy.controlaction import InformationStimulus, Rectangle, StimulusInfo, Location, TimeFrame

sys.path.append(os.path.abspath(".."))

from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.strategy.timestepping.timestepping import FixedTimeStepper
from flowcontrol.strategy.controller.control_algorithm import AlternateTargetAlgorithm, MinimalDensityAlgorithm, AvoidCongestion




class LocalController(Controller):

    def __init__(self, control_alg=None):
        super().__init__(
            control_algorithm=control_alg,
            time_stepper=FixedTimeStepper(time_step_size=2.0, start_time=10.0),
            processor_manager=Manager()
        )
        self.current_target = "none"
        self.densities_last = [0., 0., 0.]

    def handle_sim_step(self, sim_time, sim_state):
        self.measure_state(sim_time)
        self.compute_next_corridor_choice(sim_time)
        self.apply_redirection_measure(sim_time)
        self.write_dataprocessor_data(sim_time)
        self.time_stepper.forward_time()

    def measure_state(self, sim_time):
        densities = list()
        for a in [101, 102, 103]:
            __, __, density = self.con_manager.domains.v_sim.get_data_processor_value(str(a))
            densities.append(density)
        self.densities_last = densities

    def write_dataprocessor_data(self, sim_time):

        self.processor_manager.update_sim_time(sim_time)
        self.processor_manager.write("densities_received", *self.densities_last)
        self.processor_manager.write("path_choice", self.current_target)

    def handle_init(self, sim_time, sim_state):
        density_dp = DensityMeasurements(writer=Writer(os.path.join(self.output_dir, "densities_received.txt")),
                                         user_defined_header=["shortRoute", "mediumRoute", "longRoute"])

        self.processor_manager.registerProcessor("densities_received", density_dp)
        self.processor_manager.registerProcessor("path_choice", CorridorRecommendation(
            Writer(os.path.join(self.output_dir, "path_choice.txt"))))

    def write_data(self):
        self.processor_manager.finish()

    def compute_next_corridor_choice(self, sim_time):
        self.current_target = self.control_algorithm.get_next_target()

    def apply_redirection_measure(self, sim_time):
        recommendation = self._get_stimulus_info(sim_time)
        self.con_manager.domains.v_sim.send_control(message=recommendation.toJSON())

    def _get_stimulus_info(self, sim_time):
        location = Location(areas=Rectangle(x=0., y=0., width=25., height=25.))
        recommendation = InformationStimulus(f"use target {self.current_target}")
        start_time = sim_time
        end_time = start_time + 1.6 #TODO adjust the value here if you adjust the time stepping
        timeframe = TimeFrame(start_time=sim_time, end_time=end_time)
        s = StimulusInfo(location=location, stimuli=recommendation, timeframe=timeframe)
        return s


class NoController(LocalController, Controller):

    def __init__(self):
        super().__init__(control_alg=None)

    def compute_next_corridor_choice(self, sim_time):
        pass

    def apply_redirection_measure(self, sim_time):
        pass


class OpenLoop(LocalController, Controller):

    def __init__(self):
        control_alg = AlternateTargetAlgorithm(alternate_targets=[11, 21, 31])
        super().__init__(control_alg=control_alg)


class ClosedLoop(LocalController, Controller):

    def __init__(self):
        control_alg = MinimalDensityAlgorithm(alternate_targets=[11, 21, 31])
        super().__init__(control_alg=control_alg)

    def compute_next_corridor_choice(self, sim_time):
        self.current_target = self.control_algorithm.get_next_target(self.densities_last)

class AvoidShort(LocalController,Controller):

    def __init__(self):
        self.alternative_route_id = 31
        super().__init__(control_alg=AvoidCongestion(alternative_route_id=self.alternative_route_id))

    def compute_next_corridor_choice(self, sim_time):

        current_target = self.control_algorithm.get_next_target(density_congested_route = self.densities_last[0],
                                                                    density_alternate_route = self.densities_last[2])
        if current_target == None:
            self.current_target = "none"
        else:
            self.current_target = current_target

    def apply_redirection_measure(self, sim_time):

        if self.current_target == self.alternative_route_id:
            super().apply_redirection_measure(sim_time)

if __name__ == "__main__":

    if len(sys.argv) == 1:
        # default settings for control-vadere (no Omnetpp!)
        settings = ["--controller-type",
                    "AvoidShort",  #
                    "--scenario-file",
                    "three_corridors.scenario",
                    "--port",
                    "9999",
                    "--host-name",
                    "vadere",
                    "--client-mode",
                    "--output-dir",
                    "sim-output-task1",
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
