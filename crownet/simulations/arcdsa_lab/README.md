# ARC-DSA Experimental Validation

This scenario has been created for an experimental validation of the ARC-DSA algorithm proposed in [1], as part of the bachelor thesis of Vincenzo Auricchio, Munich University of Applied Sciences. It simulates a small scenario with up to 14 nodes applying the ARC-DSA algorithm to control the assigned bandwidth.

## Applications 

The scenario models a beacon app and a geospacial mapping application (entropy map) at each node:
* The beacon app is required to recognize the number of neighbours in the local area.
* The entropy map application models an arbitrary mobility oriented application disseminating random geospacial data values.

In addition, the abstract configuration "Defaults-Density-Map" in `defaults.ini` can be applied to model a decentralized pedestrian density map application [1] at each node. For validation of the ARC-DSA algorithm this model is not activated since it would only add an addtional constant data rate and make the scenario more complicated.

## Running the simulation

The simulation can either be run in the OMNeT++ IDE or via command line.

Available configurations:

The `omnetpp.ini` file includes configurations for three different activity scenarios and each scenario can be run either with WLAN or with 5G NR cellular sidelink:

1. Scenario 1 - "Always Sending" (upto-14-ue-WLAN, upto-14-ue-NR): In this scenario, all UEs are constantly generating and sending beacons and application data packets. This would be the case if all UEs are present in a small room and constantly activated, i.e. neither new UEs are coming into transmission rango nor are UEs leaving the respective resource domain.
2. Scenario 2 - "Ramp Up, Ramp Down" (upto-14-ue-ramp-WLAN, upto-14-ue-ramp-NR): Here, we model a scenario where more and more UEs become active/are coming into the modeled resource domain (ramp-up). Afterwards, all UEs are active for a while. Then, the number of active UEs gradually decreases (ramp-down).
3. Scenario 3 - "Bursts" (upto-14-ue-burst-WLAN, upto-14-ue-burst-NR): Here we model that UEs do not become active individually but instead a group of UEs becomes joins or leaves the considered resource domain. This is particularly demanding for rate control due to the sudden "spikes" in demanded resources and evaluates how well the rate control can adapt to rapid changes in the local area.

### Running in the OMNeT++ IDE
As with most other CrowNet++ simulations, simply do a right click on the `omnetpp.ini` file and select "Debug as > OMNeT++ Simulation" for running in debug mode or "Run as > OMNeT++ Simulation" for running in release mode.

### Running via Command Line
For this CrowNet++ simulation, we do not need any coupled mobility simulator. Therfore, we do not need to use the CrowNet++ tools to run the simulations but can use plain OMNeT++ and the `opp_runall` tool. 

For convenience, we provide a small script: simply execute `./run_study.sh` in the simulation folder.

### Evaluating Simulation Results
In order to evaluate the simulation results, two steps need to be performed:

* The .vec simulation trace files need to be converted to csv format, so they can be read by the Python analysis script.
* The Python anaylsis script `analyis.py` is called to calculate the results and create corresponding graphs.

Both steps can be performed by running the script `run_analyis.sh` in the `study` subfolder.

## Wireless Technologies

The scenario can either use 5G sidelink New Radio (NR) communication or WLAN in ad-hoc mode.

## Coordinate Conversion Script

Since we model geospacial data dissemination, all node positions must be defined using geographic coordinates. To simplify the calculation of these coordinates, we provide a coordinate conversion script,

This script needs pyproj to be installed in your local python environment. You can install pyproj with `pip3 install pyproj`.

### Geographic Coordinates (WGS84) of Lab R1.009a

Lat: 48.154413
Lon: 33.409740

### UTM Coordinates of Lab

Z: 32U
E: 690151.224
N: 5336624.439

### Shielded Area

* usable: 6m x 6m
* assumption: max. 8 UEs on each side
* distance (N) between UEs on the same side: 0,75m
* distance (E) between two table sides in lab: 6m
* base station (gNB) in middle of room: N+3m, E+

## Bibliography

[1] S. Schuhbäck, L. Wischhof, J. Ott (2024): *Adaptive Transmission Rate Control for Decentralized Sensing Applications*, IEEE Access. [doi: 10.1109/ACCESS.2024.3495504](https://doi.org/10.1109/ACCESS.2024.3495504)
