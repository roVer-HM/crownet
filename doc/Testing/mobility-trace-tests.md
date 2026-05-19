# Mobility trace tests

Mobility trace tests verify that standalone mobility simulators still produce
stable trace outputs after dependency or container upgrades.

The checks run Vadere and SUMO without OMNeT++ coupling, generate
BonnMotion-compatible traces, and compare hashes against stored baselines.

## Test concept

Vadere suite behavior:

- Run scenario with BonnMotionTrajectoryProcessor enabled
- Collect produced files
- Exclude non-deterministic helper files (`*.scenario`, `command.out`,
  `container*.out`)
- Compare remaining file hashes to ground truth

SUMO suite behavior:

- Run SUMO with FCD output
- Convert FCD output via `traceExporter.py` to `trace.bonnMotion`
- Compare `trace.bonnMotion` hash to baseline

## Running mobility trace tests

Expected suite location and commands:

```bash
cd $CROWNET_HOME/crownet/tests/mobility

# Run all mobility trace tests
./fingerprints_mobility

# Run Vadere-only cases
./fingerprints_mobility vadere_mobility.yml

# Run SUMO-only cases
./fingerprints_mobility sumo_mobility.yml
```
