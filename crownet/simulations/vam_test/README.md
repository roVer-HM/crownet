# VAM/CAM/DEN Test Simulation

This simulation folder contains test scenarios for V2X messaging including Cooperative Awareness Messages (CAM) and Decentralized Environmental Notifications (DEN), implemented via the Artery V2X stack.

The test simulations verify:
1. CAM generation and reception via Artery
2. DEN broadcasting via Artery
3. Integration between SUMO mobility (pedestrians and vehicles) and V2X messaging
4. LTE D2D as a bearer for V2X messages

## Simulation Context

The scenario models a simple crossing with mixed traffic managed by SUMO:
- 3 pedestrians (pNode) walking across a crossing
- 2 vehicles (vNode) driving through the crossing
- An eNodeB base station providing LTE D2D communication
- Both node types use Artery middleware with CaService and DenService

## Network Configuration

- **Network Type**: LTE D2D
- **Channel Model**: Urban Microcell
- **Carrier Frequency**: 2.6 GHz
- **Number of Resource Blocks**: 25
- **TX Power**: 20 dBm

## V2X Message Types

### CAM
- Periodic broadcast of position and status
- Enables mutual awareness between road users
- Implemented via `artery.application.CaService`

### DEN
- Event-triggered hazard warnings
- Broadcast to relevant geographic area
- Implemented via `artery.application.DenService`

## Available Configurations

| Configuration | Description |
|--------------|-------------|
| `ArterySetup` | Base Artery configuration for both pedestrians and vehicles with SUMO. Extends `withSumoBoth` and `D2D_General` |
| `simple_cam_den` | CAM and DEN message test with a simple crossing SUMO scenario. Extends `ArterySetup` |

Both configurations require the SUMO container to be running.

## Running the Simulation

The simulation can either be run in the OMNeT++ IDE or via command line.

### Running in the OMNeT++ IDE

Right click on the `omnetpp.ini` file and select "Debug as > OMNeT++ Simulation" for running in debug mode or "Run as > OMNeT++ Simulation" for running in release mode. Then select the desired configuration.

### Running via Command Line

The `run_script.py` is the CrowNet simulation runner used for containerized execution and by the fingerprint testing framework. For example:
```bash
python3 run_script.py -f omnetpp.ini -c simple_cam_den
```