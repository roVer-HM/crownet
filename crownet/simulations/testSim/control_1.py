import json
import os
import sys

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener

sys.path.append(os.path.abspath(".."))

from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc


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
        super().__init__()
        self.time_step = 0
        self.time_step_interval = 0.4
        self.controlModelName = "RouteChoice1"
        self.controlModelType = "RouteChoice"
        self.sending_node = "misc[0].app[0]"


    def handle_sim_step(self, sim_time, sim_state):

        if sim_time == 4.0:
            p1 = [0.0, 1.0]
            print("Use target [2]")

            command = {"targetIds": [2, 3], "probability": p1}
            action = {
                "space": {"x": 0.0, "y": 0.0, "radius": 100},
                "commandId": self.commandID,
                "stimulusId": -400,
                "command": command,
            }
            action = json.dumps(action)

            print(f"TikTokController: {sim_time} apply control action ")
            self.con_manager.domains.v_sim.send_control(message=action,
                                                        model=self.controlModelName,
                                                        sending_node_id=self.sending_node)
            self.commandID += 1

    def handle_init(self, sim_time, sim_state):
        super().handle_init(sim_time, sim_state)
        #TODO remove reaction model from python code and vadere code!
        self.con_manager.domains.v_sim.init_control(
            self.controlModelName, self.controlModelType, json.dumps({})
        )

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
