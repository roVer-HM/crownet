import sys, os

from flowcontrol.crownetcontrol.controller.dummy_controller import Controller, TikTokController
from flowcontrol.crownetcontrol.traci import constants_vadere as tc

from flowcontrol.crownetcontrol.traci.connection_manager import ControlTraciWrapper
from flowcontrol.crownetcontrol.traci.subsciption_listners import VaderePersonListener


if __name__ == "__main__":

	scenario_file = os.path.join(os.environ["VADERE_PATH"], "Scenarios/Demos/roVer/scenarios/scenario002.scenario")

	sub = VaderePersonListener.with_vars(
		"persons", {"pos": tc.VAR_POSITION, "target_list": tc.VAR_TARGET_LIST}
	)
	controller = TikTokController()

	if len(sys.argv) == 1:
		settings = [
			"--port",
			"9999",
			"--host-name",
			"vadere",
			"--client-mode",
			"--start-server",
			"--gui-mode",
			"--scenario",
			scenario_file
		]

		traci_manager = ControlTraciWrapper.get_controller_from_args(
			working_dir=os.getcwd(), args=settings, controller=controller)
	else:
		traci_manager = ControlTraciWrapper.get_controller_from_args(
			working_dir=os.path.dirname(os.path.abspath(__file__)),
			controller=controller)

	controller.initialize_connection(traci_manager)
	controller.start_controller()
