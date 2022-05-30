import os
import sys

sys.path.append(os.path.abspath(".."))

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.strategy.controlaction import InformationStimulus, Circle, StimulusInfo, Location

from flowcontrol.strategy.timestepping.timestepping import FixedTimeStepper


class ChangeTarget(Controller):
    """
    Send RouteChoice only once at the start of the simulation.
    With [Config test_control001_*]:
      * message will be propagated once
      * only first wave will get the control command and change there
        target accordingly
      * second wave will use there default target
    With [Config test_control002_*]:
      * message will be propagated 30 times with 1.0s interval
      * both waves will get the message
    """

    def __init__(self):
        self.redirection_area = Circle(radius=100) #
        self.current_target = 3
        super().__init__(time_stepper=FixedTimeStepper(start_time=4.0, end_time=4.0))


    def get_stimulus_info(self, target):

        location = Location(areas=self.redirection_area)
        recommendation = InformationStimulus(f"use target [{target}]")
        s = StimulusInfo(location=location, stimuli=recommendation)
        return s


    def handle_sim_step(self, sim_time, sim_state):
        s = self.get_stimulus_info(target=3)
        print(s.toJSON())
        self.con_manager.domains.v_sim.send_control(message=s.toJSON(), sending_node_id="misc[0].app[0]")
        self.time_stepper.forward_time()

    def handle_init(self, sim_time, sim_state):
        super().handle_init(sim_time, sim_state)
        self.con_manager.domains.v_sim.init_control()




if __name__ == "__main__":

    controller = ChangeTarget()

    if len(sys.argv) == 1:

        settings = [
            "--port",
            "9997",
            "--host-name",
            "0.0.0.0",
        ]
        print(settings)
        controller = get_controller_from_args(working_dir=os.getcwd(), args=settings, controller=controller)
    else:
        controller = get_controller_from_args(
            working_dir=os.path.dirname(os.path.abspath(__file__)),
            controller=controller, )


    sub = VadereDefaultStateListener.with_vars(
        "persons",
        {"pos": tc.VAR_POSITION, "speed": tc.VAR_SPEED, "angle": tc.VAR_ANGLE},
        init_sub=True,
    )

    controller.register_state_listener("default", sub, set_default=True)
    controller.start_controller()
