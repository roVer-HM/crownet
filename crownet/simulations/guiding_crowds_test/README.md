# Crowd Guiding Simulation

This simulation contains test configurations for pedestrian flow control features in CrowNet, enabling external crowd movement guidance through communication infrastructure.

## Simulation Context

The scenario models a crowd control system where:
- Pedestrians carry mobile devices with density map and beacon applications
- A central flow controller analyzes crowd conditions
- Guidance messages are broadcast to redirect pedestrian flow
- The Vadere pedestrian simulator responds to route guidance

## Network Configuration

- **Network Type**: LTE D2D
- **Channel Model**: Urban Microcell
- **Carrier Frequency**: 2.6 GHz
- **Number of Resource Blocks**: 25 (5 MHz bandwidth)
- **Single eNB**: Positioned at (20m, 20m)

## Applications

### On Pedestrian Nodes (AidSetup configs)
1. **DensityMapAppSimple**: Collects and shares crowd density
2. **BeaconDynamic**: Periodic beacons for neighbor discovery

### On Pedestrian Nodes (simplControlUdp configs)
1. **BroadcastControlApp**: Broadcasts control messages via UDP multicast

### External Flow Controller (`control.py`)
- Implements a `PingPong` controller that alternates pedestrian targets
- Uses `FixedTimeStepper` with 4s time steps
- Connects via FlowControl container

## Available Configurations

| Configuration | Description |
|--------------|-------------|
| `AidSetup` | Base configuration with AID applications (density map + beacon) |
| `simplControlUdp` | Simple UDP-based crowd control broadcast using `BroadcastControlApp` |
| `fullControl` | Full control with D2D, UDP broadcast, and flow controller integration |
| `flowcontrol` | D2D + AID applications with flow controller |
| `final` | Coupled Vadere+OMNeT++ with AID applications |
| `final_control` | Coupled Vadere+OMNeT++ with AID applications and flow controller |
| `ide_vadere_signs` | IDE-runnable: Vadere with navigation signs |
| `ide_final` | IDE-runnable: Coupled Vadere+OMNeT++ |
| `ide_final_control` | IDE-runnable: Coupled Vadere+OMNeT++ with flow control |

## How to Run

### Using `run_script.py` (command line)
```bash
# Vadere-only (no OMNeT++)
python3 run_script.py vadere --override-host-config --scenario-file vadere/scenarios/test001.scenario

# Vadere-only with navigation signs
python3 run_script.py vadere --override-host-config --scenario-file vadere/scenarios/test001_with_signs.scenario

# Coupled Vadere + OMNeT++
python3 run_script.py vadere-opp --create-vadere-container --override-host-config --opp.-c final

# Coupled Vadere + OMNeT++ with flow control
python3 run_script.py vadere-opp-control --create-vadere-container --opp.-c final_control --control-use-local --override-host-config --with-control control.py

# Vadere + flow control (no OMNeT++)
python3 run_script.py vadere-control --create-vadere-container --control-use-local --override-host-config --with-control control.py --scenario-file vadere/scenarios/test001.scenario
```

### Using the OMNeT++ IDE

The `ide_*` configs extend `withVadere` for proper TraCI setup and can be run directly from the IDE.

#### Running `ide_final` or `ide_vadere_signs` (without flow control)

1. **Terminal 1** — Start the OMNeT++ IDE:
   ```bash
   omnetpp-ide
   ```
2. **Terminal 2** — Start the Vadere mobility container:
   ```bash
   vadere
   ```

3. **In the OMNeT++ IDE**:
   - Navigate to `crownet/simulations/guiding_crowds_test/omnetpp.ini`
   - Right-click → **Run As** → **OMNeT++ Simulation**
   - Select `ide_final` or `ide_vadere_signs` from the config dropdown

#### Running `ide_final_control` (with flow control)

1. **Terminal 1** — Start the OMNeT++ IDE:
   ```bash
   omnetpp-ide
   ```
2. **Terminal 2** — Start the FlowControl IDE (PyCharm):
   ```bash
   flowcontrol-ide
   ```
3. **Terminal 3** — Start the Vadere mobility container:
   ```bash
   vadere
   ```

4. **In PyCharm**:
   - Verify the Python interpreter is set to `crownet_dev`.
   - Open `crownet/simulations/guiding_crowds_test/control.py`
   - Set up a Run Configuration: **Run** → **Edit Configurations** → **+** → **Python**
     - **Script path**: `control.py`
     - **Parameters**: `--port 9997 --host-name 0.0.0.0`
     - **Working directory**: `crownet/simulations/guiding_crowds_test`
   - Run the configuration. The controller starts listening for connections.

5. **In the OMNeT++ IDE**:
   - Right-click `omnetpp.ini` → **Run As** → **OMNeT++ Simulation**
   - Select `ide_final_control`