# Mobility Trace Fingerprint Tests

## Background

CrowNet integrates the mobility simulators **Vadere** and **SUMO** in dedicated Docker containers. When these containers are upgraded, it is critical to verify that the simulators still produce identical mobility traces.

These tests run each simulator **standalone** (without OMNeT++ coupling), generate BonnMotion trace files, and compare MD5 hashes against stored ground truth. If the traces differ, the test fails, indicating a behavioral change in the simulator.

## Test Approach

### Vadere Tests (`vadere_mobility.yml`)

The Vadere scenarios include a `BonnMotionTrajectoryProcessor` that outputs `trace.bonnMotion`. All output files (except `*.scenario`, `command.out`, `container*.out`) are hashed and compared.

| Test | Scenario | Simulation |
|---|---|---|
| `corridor_ramp_down` | `corridor_2x5m_d20_5perSpawn_ramp_down.scenario` | `adaptiveMap` |
| `simple_detour` | `simple_detour_100x177_miat1.25.scenario` | `simple_detoure_suqc_traffic` |
| `bottleneck` | `bottleneck.scenario` | `cmp_vadere_sumo` |

### SUMO Tests (`sumo_mobility.yml`)

Each test launches SUMO in a container with FCD output and converts the result to BonnMotion format using SUMO's `traceExporter.py`. Only `trace.bonnMotion` is fingerprinted.

| Test | Config | Simulation |
|---|---|---|
| `bottleneck_sumo` | `bottleneck.sumo.cfg` | `cmp_vadere_sumo/sumo/bottleneck` |
| `combined` | `combined.sumo.cfg` | `cmp_vadere_sumo/sumo/combined` |
| `luitpoldpark` | `osm.sumocfg` | `vruMec/sumo/luitpoldpark` |

## Running the Tests

```bash
cd /path/to/crownet/crownet/tests/mobility

# Run all mobility tests
./fingerprints_mobility

# Run only Vadere tests
./fingerprints_mobility vadere_mobility.yml

# Run only SUMO tests
./fingerprints_mobility sumo_mobility.yml
```