import sys
from flowcontrol.crownetcontrol.controller.dummy_controller import Controller, TikTokController
from flowcontrol.crownetcontrol.traci import constants_vadere as tc
import logging

from flowcontrol.crownetcontrol.traci.connection_manager import ClientModeConnection
from flowcontrol.crownetcontrol.traci.subsciption_listners import VaderePersonListener


# TODO parse host, name, port

# class ScenarioControl(Controller):
# TikTokController ->
#	pass


def server_test():
	sub = VaderePersonListener.with_vars("persons", {"pos": tc.VAR_POSITION, "target_list": tc.VAR_TARGET_LIST})
	controller = TikTokController()
	traci_manager = ClientModeConnection(control_handler=controller, port=9999)
	controller.initialize_connection(traci_manager)
	controller.start_controller()


if __name__ == "__main__":
	print(sys.argv)
	server_test()
	print()
