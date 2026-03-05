#!/usr/bin/env python3
"""
Generate mobility traces for Vadere and SUMO simulators and create
ground-truth MD5 hashes for fingerprint comparison.

Usage:
    python3 gen_mobility_traces.py [--vadere-mobility] [--sumo-mobility] [--scenario NAME] [--resultdir DIR]
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

from crownetutils.dockerrunner.dockerrunner import DockerCleanup
from crownetutils.dockerrunner.simulators.sumorunner import SumoRunner
from crownetutils.utils.dirdiff import create_dir_diff, diffdict_to_csv

SCRIPT_DIR = Path(__file__).resolve().parent
CROWNET_HOME = SCRIPT_DIR.parent.parent.parent
SIMULATIONS_DIR = CROWNET_HOME / "crownet" / "simulations"
HASH_DIR = SCRIPT_DIR / "hash.d"


SIM_TIME = 110.0

VADERE_SCENARIOS = [
    {
        "name": "corridor_ramp_down",
        "sim_dir": "adaptiveMap",
        "scenario_file": "vadere/scenarios/corridor_2x5m_d20_5perSpawn_ramp_down.scenario",
        "bounds": {"width": 415.0, "height": 394.0},
    },
    {
        "name": "simple_detour",
        "sim_dir": "simple_detoure_suqc_traffic",
        "scenario_file": "vadere/scenarios/simple_detour_100x177_miat1.25.scenario",
        "bounds": {"width": 177.0, "height": 120.0},
    },
    {
        "name": "bottleneck",
        "sim_dir": "cmp_vadere_sumo",
        "scenario_file": "vadere/scenarios/bottleneck.scenario",
        "bounds": {"width": 400.0, "height": 600.0},
    },
]

SUMO_SCENARIOS = [
    {
        "name": "bottleneck_sumo",
        "sim_dir": "cmp_vadere_sumo",
        "sumo_subdir": "sumo/bottleneck",
        "sumo_cfg": "bottleneck.sumo.cfg",
        "net_file": "bottleneck.net.xml",
        "sim_end": str(int(SIM_TIME)),
    },
    {
        "name": "combined",
        "sim_dir": "cmp_vadere_sumo",
        "sumo_subdir": "sumo/combined",
        "sumo_cfg": "combined.sumo.cfg",
        "net_file": "combined.net.xml",
        "sim_end": str(int(SIM_TIME)),
    },
    {
        "name": "luitpoldpark",
        "sim_dir": "vruMec",
        "sumo_subdir": "sumo/luitpoldpark",
        "sumo_cfg": "osm.sumocfg",
        "net_file": "osm.net.xml",
        "sim_end": str(int(SIM_TIME)),
    },
]


def write_hashes(diff: dict, csv_path: str):
    os.makedirs(os.path.dirname(csv_path), exist_ok=True)
    diffdict_to_csv(csv_path, diff)
    print(f"    Wrote hashes to {csv_path}")


def verify_bonnmotion_trace(trace_path: str, bounds: dict = None) -> bool:
    if not os.path.exists(trace_path):
        print(f"   MISSING: {trace_path}")
        return False

    size = os.path.getsize(trace_path)
    if size == 0:
        print(f"   EMPTY: {trace_path} (0 bytes)")
        return False

    line_count = 0
    with open(trace_path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            line_count += 1
            parts = line.split()
            if len(parts) < 3 or len(parts) % 3 != 0:
                print(f"   MALFORMED line (len={len(parts)}, not divisible by 3)")
                return False

            if bounds:
                for i in range(0, min(len(parts), 30), 3):
                    x, y = float(parts[i + 1]), float(parts[i + 2])
                    margin = 50.0
                    if (
                        x < -margin
                        or x > bounds["width"] + margin
                        or y < -margin
                        or y > bounds["height"] + margin
                    ):
                        print(
                            f"   Coordinate ({x}, {y}) seems out of bounds "
                            f"(0,0)-({bounds['width']},{bounds['height']})"
                        )

    print(f"    {trace_path}: {size:,} bytes, {line_count} agent traces")
    return True


def create_temp_scenario(scenario_path: Path, finish_time: float) -> Path:
    with open(scenario_path, "r") as f:
        scenario_json = json.load(f)

    original_time = scenario_json["scenario"]["attributesSimulation"]["finishTime"]
    scenario_json["scenario"]["attributesSimulation"]["finishTime"] = finish_time
    print(f"  Overriding finishTime: {original_time} -> {finish_time}")

    temp_path = scenario_path.parent / f"_tmp_{scenario_path.name}"
    with open(temp_path, "w") as f:
        json.dump(scenario_json, f, indent=2)
    return temp_path


def generate_vadere_traces(result_dir=None, scenario_filter=None):
    print("\n" + "=" * 70)
    print("VADERE TRACE GENERATION")
    print("=" * 70)

    hash_dir = HASH_DIR / "vadere_mobility"
    os.makedirs(hash_dir, exist_ok=True)

    for scenario in VADERE_SCENARIOS:
        name = scenario["name"]
        if scenario_filter and name != scenario_filter:
            continue
        sim_dir = SIMULATIONS_DIR / scenario["sim_dir"]
        scenario_file = scenario["scenario_file"]

        print(f"\n--- {name} ---")
        print(f"  Working dir: {sim_dir}")
        print(f"  Scenario:    {scenario_file}")

        original_path = sim_dir / scenario_file
        temp_scenario = create_temp_scenario(original_path, SIM_TIME)
        temp_scenario_rel = str(temp_scenario.relative_to(sim_dir))

        try:
            container_name = f"mob_test_{name}"
            subprocess.run(
                ["docker", "rm", "-f", f"vadere_{container_name}"],
                capture_output=True, timeout=10,
            )
            cmd = [
                sys.executable,
                str(sim_dir / "run_script.py"),
                "vadere",
                "--create-vadere-container",
                "--run-name", container_name,
                "--cleanup-policy", "remove",
                "--scenario-file",
                temp_scenario_rel,
            ]
            print(f"  Running: {' '.join(cmd[:6])}...")
            result = subprocess.run(
                cmd,
                cwd=str(sim_dir),
                capture_output=True,
                text=True,
                timeout=1800,
            )
            if result.returncode != 0:
                print(f"  Vadere failed with exit code {result.returncode}")
                print(f"  stderr: {result.stderr[:500]}")
                continue
        except subprocess.TimeoutExpired:
            print(f"   Vadere timed out after 30 minutes")
            continue
        finally:
            if temp_scenario.exists():
                os.remove(str(temp_scenario))

        trace_candidates = []
        for root, _dirs, files in os.walk(str(sim_dir)):
            if "trace.bonnMotion" in files:
                trace_candidates.append(Path(root))

        if trace_candidates:
            vadere_result_dir = max(trace_candidates, key=lambda p: p.stat().st_mtime)
        else:
            print(f"  trace.bonnMotion not found in {sim_dir}")
            continue

        target_dir = Path(result_dir) if result_dir else SCRIPT_DIR / "output" / "vadere" / name
        if target_dir.exists():
            shutil.rmtree(target_dir)
        os.makedirs(target_dir, exist_ok=True)

        for f in sorted(vadere_result_dir.iterdir()):
            if f.is_file():
                shutil.copy2(str(f), str(target_dir / f.name))

        trace_path = target_dir / "trace.bonnMotion"
        verify_bonnmotion_trace(str(trace_path), scenario.get("bounds"))

        print(f"  Output files in {target_dir}:")
        for f in sorted(target_dir.iterdir()):
            if f.is_file():
                print(f"    {f.name}: {f.stat().st_size:,} bytes")

        if not result_dir:
            diff = create_dir_diff(f"{target_dir}/**", remove_front=str(target_dir) + "/")
            csv_path = str(hash_dir / f"{name}.csv")
            write_hashes(diff, csv_path)


def generate_sumo_traces(result_dir=None, scenario_filter=None):
    print("\n" + "=" * 70)
    print("SUMO TRACE GENERATION")
    print("=" * 70)

    hash_dir = HASH_DIR / "sumo_mobility"
    os.makedirs(hash_dir, exist_ok=True)

    for scenario in SUMO_SCENARIOS:
        name = scenario["name"]
        if scenario_filter and name != scenario_filter:
            continue
        sumo_dir = SIMULATIONS_DIR / scenario["sim_dir"] / scenario["sumo_subdir"]
        sumo_cfg = scenario["sumo_cfg"]
        net_file = scenario["net_file"]
        sim_end = scenario["sim_end"]

        print(f"\n--- {name} ---")
        print(f"  SUMO dir: {sumo_dir}")
        print(f"  Config:   {sumo_cfg}")

        cfg_path = str(sumo_dir / sumo_cfg)
        net_path = str(sumo_dir / net_file)
        fcd_path = str(sumo_dir / "fcd_output.xml")
        bm_path = str(sumo_dir / "trace.bonnMotion")

        suffix = SumoRunner.random_suffix()
        sumo_runner = SumoRunner(
            name=f"sumo_trace_{name}_{suffix}",
            detach=False,
            cleanup_policy=DockerCleanup.REMOVE,
        )
        sumo_runner.set_working_dir(str(sumo_dir))

        sumo_cmd = [
            "sumo",
            "--configuration-file", cfg_path,
            "--fcd-output", fcd_path,
            "--seed", "1234",
            "--end", sim_end,
            "--no-step-log",
            "--quit-on-end",
        ]

        print(f"  Step 1: Running SUMO simulation...")
        try:
            sumo_runner.run(sumo_cmd)
        except Exception as e:
            print(f"   SUMO simulation failed: {e}")
            continue

        sumo_runner2 = SumoRunner(
            name=f"sumo_convert_{name}_{suffix}",
            detach=False,
            cleanup_policy=DockerCleanup.REMOVE,
        )
        sumo_runner2.set_working_dir(str(sumo_dir))

        convert_cmd = [
            "python3",
            "$SUMO_HOME/tools/traceExporter.py",
            "--fcd-input", fcd_path,
            "--bonnmotion-output", bm_path,
            "--net-input", net_path,
            "--person",
            "--bonnmotion-min-trace-length", "2",
            "--bonnmotion-skip-min-trace-length",
        ]

        print(f"  Step 2: Converting FCD to BonnMotion...")
        try:
            sumo_runner2.run(convert_cmd)
        except Exception as e:
            print(f"   FCD conversion failed: {e}")
            continue

        fcd_local = Path(fcd_path)
        bm_local = Path(bm_path)
        target_dir = Path(result_dir) if result_dir else SCRIPT_DIR / "output" / "sumo" / name
        if target_dir.exists():
            shutil.rmtree(target_dir)
        os.makedirs(target_dir, exist_ok=True)

        if bm_local.exists():
            shutil.copy2(str(bm_local), str(target_dir / bm_local.name))
            print(f"   Copied {bm_local.name}: {bm_local.stat().st_size:,} bytes")

        bm_output = target_dir / "trace.bonnMotion"
        if bm_output.exists():
            verify_bonnmotion_trace(str(bm_output))

        if not result_dir:
            diff = create_dir_diff(f"{target_dir}/**", remove_front=str(target_dir) + "/")
            csv_path = str(hash_dir / f"{name}.csv")
            write_hashes(diff, csv_path)

        for tmp in [fcd_local, bm_local]:
            if tmp.exists():
                os.remove(str(tmp))


def main():
    parser = argparse.ArgumentParser(
        description="Generate mobility traces and ground-truth hashes"
    )
    parser.add_argument(
        "--vadere-mobility", action="store_true", help="Generate Vadere traces only"
    )
    parser.add_argument(
        "--sumo-mobility", action="store_true", help="Generate SUMO traces only"
    )

    parser.add_argument(
        "--resultdir", default=None,
        help="Output directory for framework integration (used by fingerprint tests)",
    )
    parser.add_argument(
        "--scenario", default=None,
        help="Run only the specified scenario name",
    )
    args, _unknown = parser.parse_known_args()

    run_vadere = args.vadere_mobility or (not args.vadere_mobility and not args.sumo_mobility)
    run_sumo = args.sumo_mobility or (not args.vadere_mobility and not args.sumo_mobility)

    if run_vadere:
        generate_vadere_traces(
            result_dir=args.resultdir,
            scenario_filter=args.scenario,
        )

    if run_sumo:
        generate_sumo_traces(
            result_dir=args.resultdir,
            scenario_filter=args.scenario,
        )

    print("\n" + "=" * 70)
    print("DONE")
    print("=" * 70)


if __name__ == "__main__":
    main()
