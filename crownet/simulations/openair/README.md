# OpenAirInterface Experimental Validation

This simulation validates CrowNet's LTE-A and 5G NR models against the OpenAirInterface (OAI) 4G/5G testbed in the Mobile Communications Lab (R1.009a) at Munich University of Applied Sciences.

## Network Configuration

- **TX Power**: UE 20 dBm, eNB 20 dBm
- **Carrier Frequency**: 2.6 GHz
- **sim-time-limit**: 300s (warmup: 60s)
- **Repetitions**: 3 (with different seeds)

### Model Adaptations

Each scenario has an **adapted** and **unadapted** (-UA) variant:

- **Adapted**: Single RLC bearer (matching OAI), calibrated processing delays, OAI-matched queue sizes
- **Unadapted**: Default Simu5G parameters for comparison

## Running the Simulation

### In the OMNeT++ IDE

As with most other CrowNet++ simulations, right click on the `omnetpp.ini` file and select "Debug as > OMNeT++ Simulation" for running in debug mode or "Run as > OMNeT++ Simulation" for running in release mode.

### From the command line

```bash
# Run a single adapted scenario
./run -u Cmdenv -c S1

# Run a single unadapted scenario for comparison
./run -u Cmdenv -c S1-UA

# Run a 5G NR scenario
./run -u Cmdenv -f omnetpp-NR.ini -c S1-NR

# Run all LTE configurations
./runall_configs
```

## Available Configurations

### LTE Downlink Scenarios (`omnetpp.ini`)

| Configuration | Extends | UEs | Description |
|---|---|---|---|
| `S1` | S1-base, adaptedModel | 1 | Low-rate DL (300B @ 10 Hz), 50 RBs |
| `S1-UA` | S1-base, regularModel | 1 | S1 unadapted |
| `S1-25` | S1-base, adaptedModel25 | 1 | S1 with 25 RBs |
| `S1-25-UA` | S1-base, regularModel25 | 1 | S1 with 25 RBs unadapted |
| `S1-alert-variance` | S1 | 1 | S1 with varying alert period std-dev |
| `S2` | S2-base, adaptedModel | 1 | DL + 2.94 MB/s background traffic |
| `S2-UA` | S2-base, regularModel | 1 | S2 unadapted |
| `S2-25` | S2-base, adaptedModel25 | 1 | S2 with 25 RBs |
| `S2-25-UA` | S2-base, regularModel25 | 1 | S2 with 25 RBs unadapted |
| `S3` | S3-base, adaptedModel | 2 | UE-to-UE via eNB |
| `S3-UA` | S3-base, regularModel | 2 | S3 unadapted |
| `S4` | S4-base, adaptedModel | 2 | UE-to-UE + background overload |
| `S4-UA` | S4-base, regularModel | 2 | S4 unadapted |

### LTE D2D Sidelink Scenarios (`omnetpp.ini`)

| Configuration | UEs | Description |
|---|---|---|
| `D1` / `D1-UA` | 2 | Low-rate D2D via PC5 sidelink |
| `D2` / `D2-UA` | 3 | D2D + low-rate background (10 pkt/s) |
| `D3` / `D3-UA` | 3 | D2D + medium-rate background (100 pkt/s) |
| `D4` / `D4-UA` | 3 | D2D + high-rate background (5882 pkt/s, ~2.9 MB/s) |

### 5G NR Scenarios (`omnetpp-NR.ini`)

| Configuration | Direction | Description |
|---|---|---|
| `S1-NR` / `S1-UA-NR` | DL | Low-rate DL (300B @ 10 Hz) |
| `S1-APP-NR` / `S1-UA-APP-NR` | DL | High-rate DL (62.5 kB @ 40 Hz) |
| `S1-UP-NR` / `S1-UP-UA-NR` | UL | Low-rate UL (300B @ 10 Hz) |
| `S1-UP-App-NR` | UL | High-rate UL (31.25 kB @ 125 Hz) |
| `S1-alert-variance-NR` | DL | S1-NR with varying alert period |
| `S2-NR` / `S2-UA-NR` | DL | DL + background traffic |
| `S2-UP-NR` / `S2-UP-UA-NR` | UL | UL + background traffic |

## Evaluating Simulation Results

### Downloading Measurement Data

Testbed measurement traces can be downloaded for comparison:

```bash
./download_measurements.sh
```

### Analysis Script

The `analyse.py` script compares simulation results against testbed measurements. It produces four plot types for each scenario:

1. **Delay vs. time**: packet delay time series
2. **Delay histogram**: distribution of end-to-end delays
3. **Delay CDF**: cumulative distribution function of delays
4. **Inter-arrival time histogram**: packet inter-arrival time distribution

```bash
source activate_venv.sh
python3 analyse.py
```

## Bibliography

> L. Wischhof, F. Schaipp, S. Schuhbäck, G. Köster,
> *Simulation vs. Testbed: Small Scale Experimental Validation of an Open-Source LTE-A Model*,
> IEEE 31st Annual International Symposium on Personal, Indoor and Mobile Radio Communications (PIMRC), 2020.
> DOI: [10.1109/PIMRC48278.2020.9217163](https://doi.org/10.1109/PIMRC48278.2020.9217163)

The 5G NR extension is based on:

> M. Landes, *Experimentelle Validierung einer Open-Source 5G SA Simulation im kleinen Massstab*,
> Bachelor thesis, HM Munich University of Applied Sciences, Feb. 2024.