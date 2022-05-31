import os, sys

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener
from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc


from flowcontrol.crownetcontrol.controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc


from flowcontrol.strategy.controller.control_algorithm import AlternateTargetAlgorithm
from flowcontrol.strategy.timestepping.timestepping import FixedTimeStepper

class PingPong(Controller):
    def __init__(self):
        super().__init__(control_algorithm=AlternateTargetAlgorithm(alternate_targets=[2, 3]),
            time_stepper=FixedTimeStepper(time_step_size=4.0, start_time=4.0, end_time=30.0))

    def handle_sim_step(self, sim_time, sim_state):

        print(f"TikTokController: {sim_time} apply control action ")
        target_id = self.control_algorithm.get_next_target()
        pedestrians = self.con_manager.domains.v_person.get_id_list()
        for ped_id in pedestrians:
            self.con_manager.domains.v_person.set_target_list(
                str(ped_id), [str(target_id)]
            )
        self.time_stepper.forward_time()


if __name__ == "__main__":

    if len(sys.argv) == 1:
        settings = ["--scenario-file",
                    "test001.scenario",
                    "--experiment-label",
                    f"tiktok",
                    "--port",
                    "9997",
                    "--host-name",
                    "vadere",
                    "--client-mode",
                    ]
    else:
        settings = sys.argv[1:]

    sub = VadereDefaultStateListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION}, init_sub=True,
    )
    controller = get_controller_from_args(working_dir=os.getcwd(), args=settings, controller=PingPong())
    controller.register_state_listener("default", sub, set_default=True)
    controller.start_controller()




