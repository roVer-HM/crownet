# Density Map Simulation

The density map simulations provide comprehensive testing of density estimation approaches:
1. **Beacon-based density maps**: Estimate local density from beacon receptions
2. **Entropy-based density maps**: Use entropy measures for more accurate density estimation

## Simulation Context

Pedestrians equipped with mobile devices:
1. Send periodic beacons to neighboring devices
2. Count received beacons to estimate local crowd density
3. Share density observations with neighbors via D2D communication
4. Build a distributed density map of the environment

## Network Configuration

- **Network Type**: LTE D2D
- **Channel Model**: Urban Microcell with configurable shadowing
- **Carrier Frequency**: 2.6 GHz
- **Number of Resource Blocks**: 25 (5 MHz bandwidth)
- **Separate RNGs**: Dedicated random number generator for entropy map

## Available Configurations

### Dynamic Mobility (`omnetpp.ini`)

| Configuration | Mobility | Description |
|--------------|----------|-------------|
| `final_dynamic_m_vadere` | Vadere | Dynamic pedestrians with beacon-based map |
| `final_dynamic_m_sumo` | SUMO | SUMO mobility with beacon-based map |
| `final_dynamic_m_bonn_motion` | BonnMotion | BonnMotion mobility with beacon-based map |

### Static/Stationary Configurations (`omnetpp.ini`)

| Configuration | Description |
|--------------|-------------|
| `final_stationary_mf` | Stationary pedestrians for baseline testing |
| `final_1d` | 1D corridor configuration with Vadere |
| `final_1d_bonn_motion` | 1D corridor configuration with BonnMotion |
| `final_bonn_motion_entropy` | Entropy-based map with BonnMotion traces |

### Vadere Scenarios for Fingerprint Tests (`omnetpp.ini`)

These configurations include explicit `vadereScenarioPath` parameters for running directly from the OMNeT++ IDE or via fingerprint tests. A Vadere container must be running externally.

| Configuration | Scenario File | Description |
|--------------|---------------|-------------|
| `vadere_circle` | `circle.scenario` | Circle scenario with Vadere mobility |
| `vadere_1d_const` | `mf_1d_m_const_2x5m_d20m.scenario` | 1D corridor, constant spawn rate |

## Running the Simulation

The simulation can either be run in the OMNeT++ IDE or programmatically via Python scripts.

### Running in the OMNeT++ IDE

As with most other CrowNet++ simulations, simply do a right click on the `omnetpp.ini` file and select "Debug as > OMNeT++ Simulation" for running in debug mode or "Run as > OMNeT++ Simulation" for running in release mode.

The Vadere fingerprint test configurations (`vadere_circle`, `vadere_1d_const`, etc.) include explicit scenario paths and can be run directly from the IDE, provided a Vadere container is running externally on port 9998.

### Running via Python Scripts

The study-oriented configurations (`final_dynamic_m_vadere`, `final_dynamic_m_sumo`, etc.) are intended to be run via the Python study scripts in the `study/` subdirectory, which inject additional parameters at runtime.

## Evaluating Simulation Results

In order to evaluate the simulation results, two steps generally need to be performed:

* OMNeT++ will generate `.vec` and `.sca` simulation trace files containing all recorded scalars and vectors.
* The Python analysis script `run_script.py` extracts results from the `.vec` and `.sca` files, writing them into HDF5 format for plotting and further analysis.

The analysis scripts in `analysis/` can then be used to calculate results and create corresponding graphs.