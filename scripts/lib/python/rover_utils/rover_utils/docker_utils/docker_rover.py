import argparse
import os
import subprocess
import sys
from enum import Enum

import docker

from docker_utils.common import Cfg


class CmdWrapper(Enum):
    WAIT_FOR = 1
    BASH = 2


class RoverClient:
    def __init__(self, ns: argparse.Namespace, wrap_type: CmdWrapper):
        self.cli: docker.DockerClient = docker.from_env()
        self.cfg = Cfg.config()
        self.ns = ns
        self._wrap_type = wrap_type

        self.check_create_name()
        self.check_ptrace()

        if self._wrap_type == CmdWrapper.WAIT_FOR:
            self.wrap_command_wait_for()
        else:
            self.wrap_command_bash()

        self.run_args = Cfg.run_args()
        self.run_args["image"] = f"{self.ns.registry}{self.ns.image_name}:{self.ns.tag}"
        if self.ns.random_name:
            self.run_args.pop("name", "None")
        else:
            self.run_args["name"] = self.ns.container_name
        self.run_args["network"] = self.check_create_network()
        self.run_args["remove"] = False #self.ns.rm
        self.run_args["user"] = os.getuid()
        self.run_args["hostname"] = self.ns.container_name
        self.run_args["environment"].update(self.ns.env_dict)
        self.run_args["tty"] = sys.stdin.isatty()
        self.run_args["command"] = self.ns.cmd

    def check_create_network(self):
        """
        Check if network exists. If not create it.
        """
        networks = [n.name for n in self.cli.networks.list()]
        if self.cfg["general"]["INTNET"] not in networks:
            self.cli.networks.create(name=self.cfg["general"]["INTNET"])

        return self.cfg["general"]["INTNET"]

    def wrap_command_wait_for(self):
        """
        add waitfor.sh script to exec command. This is used in the OMNeT++ container
        """
        self.ns.cmd.insert(0, "/waitfor.sh")

    def wrap_command_bash(self):
        """
        wrap exec command in a shell call
        """
        self.ns.cmd.insert(0, "bash")
        self.ns.cmd.insert(1, "-c")
        self.ns.cmd.insert(2, "cd")
        self.ns.cmd.insert(3, os.curdir)

    def check_create_name(self):
        """
        Check if a container with the given name exist. If yes increment name based on
        given container_name
        :return: return selected name
        """
        # todo: this is not multiprocessors/thread safe.
        idx = 0
        name = self.ns.container_name
        while name in [c.name for c in self.cli.containers.list(all=True)]:
            name = f"{self.ns.container_name}{idx}"
            idx += 1

        self.ns.container_name = name
        return self.ns.container_name

    def run_container_x11(self):

        print(subprocess.call(["xhost", '+local:docker@']))

        ret = self.cli.containers.run(**self.run_args)

        print(ret)

    @staticmethod
    def check_ptrace():
        try:
            ret = subprocess.check_output(
                ["/sbin/sysctl", "kernel.yama.ptrace_scope"], timeout=1
            )
            key, val = ret.decode("utf-8").split(" = ")
            if int(val) == 1:
                print(
                    "Warning: It seems that your system does not allow ptrace - debugging will not work."
                )
                print(
                    "         Edit /etc/sysctl.d/10-ptrace.conf and set ptrace_scope = 0."
                )
                print(
                    "         See: https://linux-audit.com/protect-ptrace-processes-kernel-yama-ptrace_scope/"
                )
        except subprocess.TimeoutExpired:
            print(f"timeout error in /sbin/sysctl kernel.yama.ptrace_scope")
        except UnicodeDecodeError:
            print(f"cannot parse output of /sbin/sysctl kernel.yama.ptrace_scope")
        except ValueError:
            print(
                f"expected an integer value. Cannot parse  output of /sbin/sysctl kernel.yama.ptrace_scope"
            )
            print(f"output was: {key} = {val}")
