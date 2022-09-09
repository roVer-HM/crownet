import sys, os

from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.setup.vadere import get_scenario_content
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener

sys.path.append(os.path.abspath(".."))

from flowcontrol.strategy.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc
from flowcontrol.utils.misc import get_scenario_file

class PingPong(Controller):

    def __init__(self):
        super().__init__()
        self.control = [
            (0, ["2"]), # first call (in omnetpp simulation time), first target
            (5.0, ["3"]), # second call, second target
            (10, ["2"]),
            (15, ["3"]),
            (20, ["2"]),
            (25, ["3"]),
            (30, ["2"]),
        ]
        self.count = 0

    def initialize_connection(self, con_manager):
        self.con_manager = con_manager

    def handle_sim_step(self, sim_time, sim_state):
        if self.count < len(self.control):
            print(f"TikTokController: apply control action ")
            for ped_id in ["1", "2", "3", "4"]:
                self.con_manager.domains.v_person.set_target_list(
                    str(ped_id), self.control[self.count][1]
                )
        else:
            print(f"TikTokController: handle_sim_step evaluate control...")


    def handle_init(self, sim_time, sim_state):
        print("TikTokController: handle_init")
        print(sim_state)

    def set_next_step_time(self):
        self.count += 1
        if self.count < len(self.control):
            self.next_call = self.control[self.count][0]
            self.con_manager.next_call_at(self.next_call)
            print(f"Set next call to {self.next_call}.")
        else:
            self.con_manager.next_call_at(100)
            print(f"TikTokController will not be called any more (set next call to 100s := after simulation end).")


if __name__ == "__main__":

    if len(sys.argv) == 1:
        settings = ["--scenario-file",
                    "test001.scenario",
                    "--experiment-label",
                    f"tiktok",
                    "--port",
                    "9999",
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




