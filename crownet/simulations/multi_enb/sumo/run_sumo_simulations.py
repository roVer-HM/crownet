from dataclasses import dataclass
import os
from os.path import join
import glob
import subprocess
import sys
import datetime
import random
from crownetutils.dockerrunner.dockerrunner import (
    ContainerLogWriter,
    DockerCleanup,
    DockerRunner,
)
from crownetutils.dockerrunner.simulators.sumorunner import SumoRunner
from crownetutils.entrypoint.parser import ArgList
from crownetutils.utils.parallel import run_kwargs_map, run_args_map
from crownetutils.sumo import SimDir

if "SUMO_HOME" in os.environ:
    sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))
import sumolib  # noqa
import traceExporter as tExp


class ContainerList:
    def __init__(self) -> None:
        self.suffix = SumoRunner.random_suffix()
        self.containers = []

    def create_and_run(self, cmd_list, out=sys.stdout):
        i = len(self.containers)
        sumo = SumoRunner(
            name=f"sumo_{i:03}_{self.suffix}",
            detach=True,
            cleanup_policy=DockerCleanup.REMOVE,
        )
        sumo.working_dir = root
        if isinstance(out, str):
            sumo.set_log_callback(
                ContainerLogWriter(os.path.join(out, f"_log/{sumo.name}.log"))
            )
        else:
            sumo.set_log_callback(ContainerLogWriter(out))
        print(f"start container: {sumo.name}")
        sumo.run(cmd_list)  # execute now
        self.containers.append(sumo)

    def wait(self):
        for c in self.containers:
            c: SumoRunner
            print(f"wait for {c.name}")
            ret = c.wait()
            print(f"cleanup container {c.name} with wait ret {ret}:")
            c.container_cleanup()


def generate_routes(sim_dir: SimDir, net_file, scenario_prefix="", N=50):
    runners = ContainerList()
    rnd = random.Random(x=42)
    for i in range(N):
        args = ArgList()
        args.add("--net-file", net_file)
        args.add("--random")
        args.add("-s", rnd.randint(0, 100_000))
        args.add("--fringe-factor", 1)
        args.add("--insertion-density", 6)
        args.add(
            "--output-trip-file",
            sim_dir.route(f"{i:03}_{scenario_prefix}_ped_trip.xml"),
        )
        args.add(
            "--route-file", sim_dir.route(f"{i:03}_{scenario_prefix}_ped_route.xml")
        )
        args.add("--begin", 0)
        args.add("--end", 1800)  # 30 min
        # args.add("--end", 3600) # 60 min
        args.add("--verbose")
        args.add("--additional-files", sim_dir.root("custom_vtype.xml"))
        args.add("--trip-attributes", "'type=\\\"pedType1\\\"'")
        args.add("--pedestrians")
        args.add("--max-distance", 2000)
        args.add_cmd("$SUMO_HOME/tools/randomTrips.py")

        runners.create_and_run(args.to_list(), sim_dir.out())

    runners.wait()

    print("done.")


def run_simulations(sim_dir: SimDir, net, scenario_prefix=""):

    routes = glob.glob(sim_dir.route(f"*{scenario_prefix}_ped_route.xml"))
    routes.sort(key=lambda x: int(os.path.basename(x)[0:3]))
    runners = ContainerList()
    for i, route in enumerate(routes):
        args = ArgList()
        args.add("sumo")
        args.add("--net-file", net)
        args.add("--route-files", route)
        args.add("--seed", "1234")
        args.add("--step-length", "0.4")
        args.add("--end", str(3600))  # 60 min. -> 3600 s
        args.add(
            "--fcd-output", sim_dir.fcd(f"{i:03}_{scenario_prefix}_fcd_muc.xml.gz")
        )
        args2 = ArgList.from_flat_list(args.to_list())
        args2.add(
            "--save-configuration",
            sim_dir.sumo_cfg(f"{i:03}_{scenario_prefix}.sumocfg"),
        )
        args2.add("--save-configuration.relative")
        cmd = [*args2.to_list(), "&&", *args.to_list()]
        runners.create_and_run(cmd, sim_dir.out())

    runners.wait()


def convert_fcd_to_bonnmotion(sim_dir: SimDir, net, scenario_prefix=""):
    fcd_output = glob.glob(sim_dir.fcd(f"*{scenario_prefix}_fcd_muc.xml.gz"))
    fcd_output.sort(key=lambda x: int(os.path.basename(x)[0:3]))

    runners = ContainerList()
    for i, fcd in enumerate(fcd_output):
        bm = sim_dir.bm(
            os.path.basename(fcd).replace("fcd", "").replace("xml", "bonnmotion")
        )
        args = ArgList()
        args.add(f"{os.environ['SUMO_HOME']}/tools/traceExporter.py")
        args.add("--fcd-input", fcd)
        args.add("--bonnmotion-output", bm)
        args.add("--net-input", net)
        args.add("--person")
        args.add("--bonnmotion-min-trace-length", "2")
        args.add("--bonnmotion-skip-min-trace-length")
        args.add("--bonnmotion-omnet-origin")

        runners.create_and_run(args.to_list(), sim_dir.out())

    runners.wait()
    print("done.")


def run_diff(root):

    base = join(root, "muc_base.net.xml.gz")
    working_copy = join(root, "muc.net.xml.gz")
    t = datetime.datetime.now()
    t = datetime.datetime.strftime(t, "%Y-%m-%d_%H-%M")
    diff_prefix = f"{t}_diff"
    args = ArgList()
    args.append("${SUMO_HOME}/tools/net/netdiff.py")
    args.append("--remove-plain")
    args.append("--patch-on-import")
    args.append(base)
    args.append(working_copy)
    args.append(diff_prefix)
    sumo = SumoRunner(detach=False)
    sumo.working_dir = join(root, "diff")
    sumo.set_log_callback(
        ContainerLogWriter(os.path.join(root, f"diff/{diff_prefix}.log"))
    )
    sumo.run(args.to_list())
    print("done.")


if __name__ == "__main__":
    root = "/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/sumo/munich/muc_cleaned/"

    # sim_dir = SimDir(sim_root=root, output_root="output/run_60_min/")
    sim_dir = SimDir(sim_root=root, output_root="output/run_60_min_cleaned")
    sim_dir.create_dirs()
    net_file = sim_dir.root("muc.net.xml.gz")
    scenario_prefix = ""

    # generate_routes(sim_dir, net_file, scenario_prefix=scenario_prefix, N=20)
    run_simulations(sim_dir, net_file, scenario_prefix=scenario_prefix)
    convert_fcd_to_bonnmotion(sim_dir, net_file, scenario_prefix=scenario_prefix)
    # run_diff(root)
