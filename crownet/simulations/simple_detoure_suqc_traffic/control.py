import sys, os

sys.path.append(os.path.abspath(".."))
from import_PYTHON_PATHS import *

from flowcontrol.crownetcontrol.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.crownetcontrol.traci.connection_manager import ControlTraciWrapper
from flowcontrol.crownetcontrol.traci.subsciption_listners import VaderePersonListener


class TikTokController(Controller):
    def __init__(self):
        super().__init__()
        self.sub = VaderePersonListener.with_vars(
            "persons",
            {"pos": tc.VAR_POSITION, "target_list": tc.VAR_TARGET_LIST},
            init_sub=True,
        )
        self.control = [
            (1.0, ["2"]),
            (5.0, ["3"]),
            (10, ["2"]),
            (15, ["3"]),
            (20, ["2"]),
            (25, ["3"]),
            (30, ["2"]),
        ]
        self.count = 0

    def initialize_connection(self, con_manager):
        self.con_manager = con_manager
        self.con_manager.register_subscription_listener(
            "vadere1", self.sub, set_default=True
        )

    def handle_sim_step(self, sim_time, sim_state, traci_client):
        if self.count >= len(self.control):
            return
        print(f"TikTokController: {sim_time} handle_sim_step evaluate control...")

        print(f"TikTokController: {sim_time} apply control action ")
        for ped_id in self.sub.pedestrian_id_list:
            self.con_manager.domains.v_person.set_target_list(
                str(ped_id), self.control[self.count][1]
            )

        self.con_manager.next_call_at(self.control[self.count][0])
        self.count += 1

    def handle_init(self, sim_time, sim_state, traci_client):
        print("TikTokController: handle_init")


if __name__ == "__main__":

    sub = VaderePersonListener.with_vars(
        "persons", {"pos": tc.VAR_POSITION, "target_list": tc.VAR_TARGET_LIST}
    )
    controller = TikTokController()

    scenario_file = os.path.join(os.getcwd(), "scenario002.scenario")

    if len(sys.argv) == 1:
        settings = [
            "--port",
            "9999",
            "--host-name",
            "vadere",
            "--client-mode",
            # "--start-server", start server manually if not set
            "--scenario",
            scenario_file,
        ]
        traci_manager = ControlTraciWrapper.get_controller_from_args(
            working_dir=os.getcwd(), args=settings, controller=controller
        )
    else:
        print(sys.argv)
        traci_manager = ControlTraciWrapper.get_controller_from_args(
            working_dir=os.path.dirname(os.path.abspath(__file__)),
            controller=controller,
        )

    controller.initialize_connection(traci_manager)
    controller.start_controller()

    print()
