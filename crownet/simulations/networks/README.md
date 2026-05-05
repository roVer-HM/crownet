# Shared Network Definitions

This folder contains shared network topology definitions and default configurations used by multiple simulations in CrowNet.

## Network Definitions

| File | Network | Description |
|------|---------|-------------|
| `World.ned` | `World` | Main network for cellular (LTE/5G NR) simulations with Vadere/SUMO coupling. Includes eNBs/gNBs, core network (PGW/UPF), TraCI manager, density maps, flow controller, BonnMotion server, and full-mesh X2 connections. Supports both 4G and 5G via `useNR` parameter. |
| `World_Wlan.ned` | `World_Wlan` | Extends `World` with WLAN-only mode. Adds optional stationary `ApplicationLayerPedestrian` nodes. |
| `LTE_d2d.ned` | `LTE_d2d` | Standalone LTE D2D network with a single eNB, core network, and configurable UE arrays. |
| `LabSingleCellBase.ned` | `LabSingleCellBase` | Minimal base network with only infrastructure modules (channel control, binder, configurator, router, server). |
| `LabSingleCellLte.ned` | `LabSingleCellLte` | Extends `LabSingleCellBase` with LTE eNodeB, PGW, and Simu5G UEs. |
| `LabSingleCellNR.ned` | `LabSingleCellNR` | Extends `LabSingleCellBase` with 5G NR gNodeB, UPF, and NRUe nodes. |
| `LabSingleCellCrowNet.ned` | `LabSingleCellCrowNet` | Full-featured lab network supporting both WLAN and cellular (5G NR). Includes CrowNet-specific modules: density maps, entropy maps, flow controller, file writer, visualizer. Uses `ApplicationLayerPedestrian` UEs. |

## Default Configurations (`default_configs.ini`)

### World-Level Configs

| Configuration | Network | Description |
|---|---|---|
| `World_lte` | World | Cellular-only, no density map, no flow controller |
| `World_lte_dcd` | World | Cellular with `GlobalDensityMap` enabled |
| `World_lte_dcd_control` | World | Cellular with density map + `ControlManager` (flow control) |
| `World_lte_control` | World | Cellular with `ControlManager` |

### Mobility Provider Configs

| Configuration | Description |
|---|---|
| `withSumoBase` | SUMO TraCI coupling (Core, SumoLauncher, SumoCombinedNodeManager) |
| `withSumoPeds` | Extends `withSumoBase`, pedestrians only |
| `withSumoVehicles` | Extends `withSumoBase`, vehicles only |
| `withSumoBoth` | Extends `withSumoBase`, pedestrians + vehicles |
| `withVadere` | Vadere TraCI coupling (VadereCore, VadereLauncher, VadereNodeManager) |
| `noTraCI` | Disables TraCI and flow controller |

### Application Configs

| Configuration | Description |
|---|---|
| `D2D_General` | D2D multicast groups, processing delays, CQI 7 |
| `simpelBroadcast` | Simple `BaseBroadcast` app |
| `simpelBroadcastUdp` | Extends `simpelBroadcast` with `CrownetUdpApp` |
| `simpelBroadcastAid` | Extends `simpelBroadcast` with `CrownetAidApp` |
| `aidDcd_withArtery` | Artery + DensityMap app with `AidPedestrian` nodes |
| `pNode_dcdMapSimple` | DensityMap (YMF) + Beacon applications for pNode |
| `pNode_dynamicDcdApp` | Same as `pNode_dcdMapSimple` (dynamic variant) |
| `pApp_AlterSenderReceiver` | AlertSender/AlertReceiver pair for pNode |
| `vApp_AlterSenderReceiver` | AlertSender/AlertReceiver pair for vNode |


## Usage

Other simulations use these shared definitions by importing networks and including configs.

```ini
# Example for a simulation using these shared definitions
network = crownet.simulations.networks.World
include ../networks/default_configs.ini
```