# CoviCom21 - Crowd Density Communication Simulation

The CoviCom21 simulations model pedestrians exchanging crowd density information via LTE D2D communication. The goal is to evaluate:
- How pedestrians can collaboratively build and share density maps
- Communication performance in realistic urban pedestrian scenarios
- The effectiveness of different map aggregation strategies

## Applications

Each pedestrian node runs:
1. **Density Map**: Collects and distributes crowd density observations
2. **Beacon**: Periodic beacon messages for neighbor discovery

## Network Configuration

- **Network Type**: LTE D2D
- **Channel Model**: Urban Microcell
- **Carrier Frequency**: 2.6 GHz
- **Number of Resource Blocks**: 25
- **TX Power**: 20 dBm

## Available Simulation Configurations

| Configuration | Description |
|---|---|
| `AidSetup` | Abstract config: sets up Vadere connection and apps (DensityMap + Beacon) |
| `vadere_120` | Small area, extends `AidSetup` + `env_mf_001_small_120` |
| `vadere_60` | Small area, extends `AidSetup` + `env_mf_001_small_60` |
| `vadere_base_96` | Base area, extends `AidSetup` + `env_mf_001_base_96` |
| `vadere_base_72` | Base area, extends `AidSetup` + `env_mf_001_base_72` |
| `vadere_120_maptypes` | Extends `vadere_120`, tests different map aggregation strategies |

<table>
  <tr>
    <td align="center" width="90%"><img src="screenshots/env_mf_001_base_96.png" style="height:300px;object-fit:contain"/><br/><em>Base scenario with 96 pedestrians</em></td>
  </tr>
  <tr>
    <td align="center" width="90%"><img src="screenshots/vadere_120_maptypes.png" style="height:300px;object-fit:contain"/><br/><em>Vadere mobility to test different map aggregation strategies</em></td>
  </tr>
</table>

## Running the Simulation

All configs require the Vadere pedestrian simulator.

### Running via Command Line

```bash
# Default (vadere_120)
python3 run_script.py vadere-opp --create-vadere-container --opp.-c vadere_120

# Specific configuration
python3 run_script.py vadere-opp --create-vadere-container --opp.-c vadere_base_96
```

### Running in the OMNeT++ IDE

All configs require the Vadere container to be running.

1. **Terminal 1** — Start the OMNeT++ IDE:
   ```bash
   omnetpp-ide
   ```
2. **Terminal 2** — Start the Vadere mobility container:
   ```bash
   vadere
   ```

3. **In the OMNeT++ IDE**:
   - Right-click `omnetpp.ini` → **Run As** → **OMNeT++ Simulation**
   - Select a runnable config from the list

## Evaluating Simulation Results

The `analysis.py` script processes simulation results. The `.vec` trace files need to be converted to `.csv` format first for the analysis.