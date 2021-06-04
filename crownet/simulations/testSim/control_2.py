import sys, os

import json

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.setup.vadere import get_scenario_content
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener

sys.path.append(os.path.abspath(".."))

from flowcontrol.strategy.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

class ChangeTarget(Controller):

    def __init__(self):
        super().__init__()
        self.count = 0
        self.next_time = [25.0, 500.0] # cal again at time t = 100.0 


    def initialize_connection(self, con_manager):
        self.con_manager = con_manager

    def handle_sim_step(self, sim_time, sim_state):
        self.count += 1
        
        sending_node = "node[0].app[0]"
        model = "RouteChoice"
        command = {"targetIds" : [3] , "probability" : [1.0]}
        action = { "time" : sim_time+0.4, "space" : {"x" : 0.0, "y" : 0.0, "radius": 100}, "command" : command}
        action = json.dumps(action)
        
        self.con_manager.domains.v_sim.send_control(message=action, model=model, sending_node_id=sending_node)
        
        if len(self.next_time) < self.count:
            self.con_manager.next_call_at(self.next_time[self.count])

    def handle_init(self, sim_time, sim_state):
        # call first at time 1.0 s
        self.con_manager.next_call_at(1.0)


if __name__ == "__main__":

    sub = VadereDefaultStateListener.with_vars(
        "persons",
        {"pos": tc.VAR_POSITION, "speed": tc.VAR_SPEED, "angle": tc.VAR_ANGLE},
        init_sub=True,
    )

    controller = ChangeTarget()

    if len(sys.argv) == 1:

        settings = [
            "--port",
            "9997",
            "--host-name",
            "0.0.0.0",
        ]


        controller = get_controller_from_args(working_dir=os.getcwd(), args=settings, controller=controller)
    else:
        controller = get_controller_from_args(
            working_dir=os.path.dirname(os.path.abspath(__file__)),
            controller=controller,)

    controller.register_state_listener("default", sub, set_default=True) 
    print("start ...")
    controller.start_controller(**kwargs)

