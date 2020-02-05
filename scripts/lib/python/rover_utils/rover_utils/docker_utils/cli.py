import argparse

from docker_utils.docker_rover import RoverClient, CmdWrapper
from docker_utils.common import Cfg
import os


def main_omnetpp(ns):
    if ns.exec is None:
        setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))
    else:
        setattr(ns, "cmd", ns.exec)

    client = RoverClient(ns, CmdWrapper.WAIT_FOR)
    client.run_container_x11()


def main_vadere(ns):
    if ns.exec is None:
        setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))
    else:
        setattr(ns, "cmd", ns.exec)

    client = RoverClient(ns, CmdWrapper.BASH)
    client.run_container_x11()


def main_vadere_gui(ns):
    setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))
    if ns.project != "":
        ns.cmd.extend(["--project", ns.project])

    client = RoverClient(ns, CmdWrapper.BASH)
    client.run_container_x11()
    pass


def main_vadere_postvis(ns):
    setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))

    client = RoverClient(ns, CmdWrapper.BASH)
    client.run_container_x11()
    pass


def main_sumo(ns):
    setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))

    client = RoverClient(ns, CmdWrapper.BASH)
    client.run_container_x11()
    pass


def main_sumo_gui(ns):
    setattr(ns, "cmd", Cfg.get_attr(ns.command_name, "cmd"))

    client = RoverClient(ns, CmdWrapper.BASH)
    pass


class EnvAction(argparse.Action):
    """
    In addition to save the value to the specified destination create am environment pair to
    use later on. The key of the entry is the upper cased value of the option_string.

    Version 1: key name derived from option_string (longform only. e.g. --traci-port 9998 --> TRACI_PORT=9998
        a) save value of option_string to designated destination
        b) save enviroment key/value pair in env_dict. => --traci-port 9998 --> env_dict={TRACI_PORT=9998}

    Version 2: option_string is --env (e.g. --env TRACI_GUI=True ) ! No spaces !
        a) save value of option_string to designated destination (never used)
        b) save value  of  option_string key/value pair in env_dict. => --env TRACI_GUI=TRUE -> env_dict{TRACI_GUI=TRUE}
    """

    def __call__(self, parser, namespace, values, option_string=None):
        if not hasattr(namespace, "env_dict"):
            # create env attribute if missing.
            setattr(namespace, "env_dict", {})

        if option_string == "--env":
            key, val = values[0].split("=")
        elif option_string.startswith("--"):
            key = option_string[2:].replace("-", "_").upper()
            val = values
        else:
            raise argparse.ArgumentParser(
                f"EnvAction expected --env or --*** option_strings but received: {option_string}"
            )

        env: dict = getattr(namespace, "env_dict")
        env.setdefault(key, val)

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        if self.option_strings[0] == "--env":
            self.env_dict_key = None
        elif self.option_strings[0].startswith("--"):
            self.env_dict_key = self.option_strings[0][2:].replace("-", "_").upper()
        else:
            self.env_dict_key = None


def add_default_args(container_name, parser):
    parser.add_argument(
        "-n",
        "--container-name",
        required=False,
        dest="container_name",
        default=container_name,
        help=f"Container name. (Default: {container_name})",
    )
    parser.add_argument(
        "-i",
        "--image-name",
        required=False,
        dest="image_name",
        default=container_name,
        help=f"(short) Image name. (Default: {container_name})",
    )
    parser.add_argument(
        "-r",
        "--registry",
        required=False,
        dest="registry",
        default="sam-dev.cs.hm.edu:5023/rover/rover-main/",
        help="Docker Registry path",
    )
    parser.add_argument(
        "-t",
        "--tag",
        required=False,
        dest="tag",
        default="latest",
        help="Container tag to use",
    )
    parser.add_argument(
        "--rm",
        required=False,
        dest="rm",
        action="store_true",
        default=True,
        help="delete container after container stops",
    )
    parser.add_argument(
        "--random-name",
        required=False,
        dest="random_name",
        action="store_true",
        default=False,
        help="Let Docker choose a name for the container.",
    )
    parser.add_argument(
        "--env",
        required=False,
        action=EnvAction,
        dest="env",
        nargs="+",
        help="Define environment variables to send to the docker container",
    )


def add_traci_args(parser, default_port, default_gui):
    parser.add_argument(
        "--traci-port",
        required=False,
        dest="traci_port",
        action=EnvAction,
        default=default_port,
        help="Set traci port to listen to",
    )
    parser.add_argument(
        "--traci-gui",
        required=False,
        dest="traci_gui",
        action=EnvAction,
        default=default_gui,
        help="Set to activate gui or not.",
    )
    parser.add_argument(
        "--traci-debug",
        required=False,
        dest="traci_debug",
        action=EnvAction,
        default=False,
        help="Set debug in conatiner",
    )


def parser_omnetpp(sub):
    command_name = "omnetpp"

    omnetpp = sub.add_parser(
        name=command_name,
        description="Command: 'omnetpp' *** Start OMNeT++ Container ***",
        help="Start OMNeT++ Container (Default: OMNeT++ IDE)",
    )
    omnetpp.add_argument(
        "--exec",
        required=False,
        nargs="+",
        dest='exec',
        help="execute following stuff in the container (do not run ini.sh)",
    )
    omnetpp.set_defaults(main_func=main_omnetpp)
    omnetpp.set_defaults(sub_parser=omnetpp)
    omnetpp.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), omnetpp)


def parser_vadere(sub):
    command_name = "vadere"

    vadere = sub.add_parser(
        name=command_name,
        description="Start Vadere Container",
        help="Start Vadere Container (Default: vader-launchd)",
    )
    vadere.add_argument(
        "--exec",
        required=False,
        nargs="+",
        dest='exec',
        help="execute following stuff in the container (do not run ini.sh)",
    )
    vadere.set_defaults(main_func=main_vadere)
    vadere.set_defaults(sub_parser=vadere)
    vadere.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), vadere)
    add_traci_args(vadere, 9998, False)


def parser_vadere_gui(sub):
    command_name = "vadere-gui"

    vadere_gui = sub.add_parser(
        name=command_name,
        description="Start Vadere-Gui in Container",
        help="Start Vadere-Gui in Container",
    )
    vadere_gui.add_argument(
        "--project",
        required=False,
        dest="project",
        default="",
        help="Path to project to open. Last project of non given."
    )
    vadere_gui.set_defaults(main_func=main_vadere_gui)
    vadere_gui.set_defaults(sub_parser=vadere_gui)
    vadere_gui.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), vadere_gui)
    add_traci_args(vadere_gui, 9998, True)


def parser_vadere_postvis(sub):
    command_name = "vadere-postvis"

    vadere_postvis = sub.add_parser(
        name=command_name,
        description="Start Vadere Postvis Gui Container",
        help="Start Vadere Postvis Gui Container",
    )
    vadere_postvis.set_defaults(main_func=main_vadere_postvis)
    vadere_postvis.set_defaults(sub_parser=vadere_postvis)
    vadere_postvis.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), vadere_postvis)
    add_traci_args(vadere_postvis, 9998, True)


def parser_sumo(sub):
    command_name = "sumo"

    sumo = sub.add_parser(
        name=command_name,
        description="Start sumo Container",
        help="Start sumo Container (Default: sumo-lauchd",
    )
    sumo.add_argument(
        "--exec",
        required=False,
        nargs="+",
        dest='exec',
        help="execute following stuff in the container (do not run ini.sh)",
    )
    sumo.set_defaults(main_func=main_sumo)
    sumo.set_defaults(sub_parser=sumo)
    sumo.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), sumo)
    add_traci_args(sumo, 9999, False)


def parser_sumo_gui(sub):
    command_name = "sumo-gui"

    sumo_gui = sub.add_parser(
        name=command_name,
        description="Start sumo-gui Container",
        help="Start sumo-gui Container",
    )
    sumo_gui.set_defaults(main_func=main_sumo_gui)
    sumo_gui.set_defaults(sub_parser=sumo_gui)
    sumo_gui.set_defaults(command_name=command_name)
    add_default_args(Cfg.get_attr(command_name, "default_image_name"), sumo_gui)
    add_traci_args(sumo_gui, 9999, True)


def parse_args(arguments=None):
    main = argparse.ArgumentParser(prog="Docker Wrapper for rover Containers")

    sub = main.add_subparsers(title="Commands")

    # CMD 1 omnetpp
    parser_omnetpp(sub)

    # CMD 2 vadere
    parser_vadere(sub)

    # CMD 3 vadere-gui
    parser_vadere_gui(sub)

    # CMD 4 vadere-postvis
    parser_vadere_postvis(sub)

    # CMD 5 sumo
    parser_sumo(sub)

    # CMD 6   sumo-gui
    parser_sumo_gui(sub)

    if arguments is None:
        _ns = main.parse_args()
    else:
        _ns = main.parse_args(arguments)

    # search for arguments with EnvAction not specified as arguments (thus default values are used)
    # on these arguments the EnvAction was not called. Do this here so the default values are added
    # to the env_dict in the namespace
    if not hasattr(_ns, "env_dict"):
        # create env attribute if missing.
        setattr(_ns, "env_dict", {})

    for action in _ns.sub_parser._actions:
        if (
            type(action) == EnvAction
            and action.env_dict_key is not None
            and action.env_dict_key not in _ns.env_dict
        ):
            action(main, _ns, action.default, action.option_strings[0])

    return _ns


if __name__ == "__main__":
    ns = parse_args(["vadere-gui", "-h"])
    # ns = parse_args(['-h'])
    # ns = parse_args()
    ns.main_func(ns)
