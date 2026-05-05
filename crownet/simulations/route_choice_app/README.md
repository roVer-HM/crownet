# Route Choice Application (Vadere-Only)

This simulation tests pedestrian route choice control using Vadere's three-corridors scenario. It is does not include an OMNeT++ network, but a flow controller communicates directly with Vadere via TraCI to redirect pedestrians based on corridor density measurements.

## Vadere Scenario

The `three_corridors.scenario` provides:
- Three parallel corridors of different lengths (short, medium, long)
- Targets 11, 21, 31 corresponding to each corridor
- Psychology layer enabled (`ProbabilisticCognitionModel`) so pedestrians respond to `InformationStimulus` guidance

## Controller Types

The `control.py` script defines four control strategies:

| Controller | Algorithm | Description |
|---|---|---|
| `NoController` | — | Baseline: no guidance applied, natural corridor distribution |
| `OpenLoop` | `AlternateTargetAlgorithm` | Cycles through targets [11, 21, 31] regardless of state |
| `ClosedLoop` | `MinimalDensityAlgorithm` | Selects the corridor with the lowest measured density |
| `AvoidShort` | `AvoidCongestion` | Redirects to corridor 3 (long) when corridor 1 (short) is congested |

All controllers use `FixedTimeStepper` and measure density via Vadere data processors for the three corridors.

## Running the Simulation

### Prerequisites

- Vadere container
- Flow control container

### Running via Command Line

```bash
python3 run_script.py
```