import os
import subprocess
from pathlib import Path

_default_scavetool_cmd = "scavetool"
_default_use_docker_container = True

if "ROVER_MAIN" in os.environ:
    _default_rover_main = os.environ.get("ROVER_MAIN")
    _default_opp_container_path = os.path.join(
        os.environ.get("ROVER_MAIN"), "scripts/omnetpp"
    )
else:

    _default_rover_main = os.path.join(str(Path.home().absolute()), "rover-main")
    if not os.path.exists(_default_rover_main):
        print(
            f"ROVER_MAIN not set and not in default location. Deactivate docker support"
        )
        _default_use_docker_container = False
        _default_opp_container_path = ""
    else:
        print(f"ROVER_MAIN not set, use  default {_default_rover_main}")
        os.path.join(os.environ.get("ROVER_MAIN"), "scripts/omnetpp")


class ConfigException(Exception):
    pass


class Config:
    """
    Config object to determine the correct way to call the scavetool.
    """

    def __init__(self, **kwargs):

        self.scave_tool_cmd = kwargs.get("scave_tool_cmd", _default_scavetool_cmd)
        self.use_docker_container = kwargs.get(
            "use_docker_container", _default_use_docker_container
        )
        self.opp_container_path = kwargs.get(
            "opp_container_path", _default_opp_container_path
        )
        self.rover_main = kwargs.get("rover_main", _default_rover_main)

    def test_config(self):
        try:
            out = subprocess.check_output(
                [self.opp_container_path, "exec", self.scave_tool_cmd, "--help"],
                env=os.environ.copy(),
            )
            out_str = out.decode("utf-8").split("\n")[0:2]
            print("using: " + " ".join(out_str))
        except subprocess.CalledProcessError as e:
            raise ConfigException(
                f"Docker container found but scavetool created error: {e.cmd}"
            )
        except Exception as ee:
            raise ConfigException(
                f"container_path command not found. container_path was: {self.opp_container_path}"
            )

    @property
    def scave_cmd(self):
        if self.use_docker_container:
            return [self.opp_container_path, "exec", self.scave_tool_cmd]
        else:
            return [self.scave_tool_cmd]

    @property
    def uses_container(self):
        return self.use_docker_container

    def escape(self, val):
        """Escape characters if docker ist used."""
        if self.uses_container:
            val = val.replace(r" ", r"\ ")
            val = val.replace(r"(", r"\(")
            val = val.replace(r")", r"\)")
        return val
