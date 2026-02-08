#!/usr/bin/env python3
"""
Generate BonnMotion trace files for fingerprint tests.

Because the adaptiveMap scenarios use one-way coupling (Vadere -> OMNeT++ only,
no feedback), pre-recorded traces produce identical mobility to live coupling.

Prerequisites:
    - Docker must be running
    - The Vadere Docker image must be available
"""

import os
import shutil
import subprocess
import sys

SCENARIOS = [
    "corridor_2x5m_d20",
    "corridor_2x5m_d20_5perSpawn",
    "corridor_2x5m_d20_5perSpawn_ramp_down",
]

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIM_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
TRACE_DIR = os.path.join(SIM_DIR, "trace")


def generate_trace(scenario_name: str) -> None:
    scenario_rel = f"vadere/scenarios/{scenario_name}.scenario"
    scenario_abs = os.path.join(SIM_DIR, scenario_rel)
    if not os.path.isfile(scenario_abs):
        raise FileNotFoundError(f"Scenario file not found: {scenario_abs}")

    print(f"\n{'='*60}")
    print(f"Generating trace for: {scenario_name}")
    print(f"{'='*60}")

    cmd = [
        sys.executable,
        os.path.join(SIM_DIR, "run_script.py"),
        "vadere",
        "--create-vadere-container",
        "--scenario-file", scenario_rel,
    ]
    result = subprocess.run(cmd, cwd=SIM_DIR)
    if result.returncode != 0:
        raise RuntimeError(f"Vadere failed for {scenario_name} (exit code {result.returncode})")

    # Find the most recent trace.bonnMotion in results
    candidates: list[str] = []
    for root, _dirs, files in os.walk(SIM_DIR):
        if "trace.bonnMotion" in files:
            candidates.append(os.path.join(root, "trace.bonnMotion"))

    if not candidates:
        raise FileNotFoundError(f"trace.bonnMotion not found for {scenario_name}")

    trace_src = max(candidates, key=os.path.getmtime)
    trace_dst = os.path.join(TRACE_DIR, f"{scenario_name}.bonnMotion")
    os.makedirs(TRACE_DIR, exist_ok=True)
    shutil.copy2(trace_src, trace_dst)
    print(f"  Saved: {trace_dst}")


def main():
    print("Generating BonnMotion traces for fingerprint tests")
    print(f"Trace output dir: {TRACE_DIR}")
    os.makedirs(TRACE_DIR, exist_ok=True)

    generated = []
    for scenario in SCENARIOS:
        try:
            generate_trace(scenario)
            generated.append(scenario)
        except Exception as e:
            print(f"\nERROR: {scenario}: {e}")

    print(f"\nSummary: {len(generated)}/{len(SCENARIOS)} traces generated")
    for name in generated:
        print(f"   trace/{name}.bonnMotion")


if __name__ == "__main__":
    main()
