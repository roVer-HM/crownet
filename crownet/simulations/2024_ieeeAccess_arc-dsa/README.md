# Adaptive Transmission Rate Control for Decentralized Sensing Applications

The following simulation directories where used for this publication.

## Scenario S1/S2

* Base simulation folder: [simulation/adaptiveMap](../adaptiveMap)
* Simulation study folder: [simulation_1](./simulation_s1)
* Simulation study folder: [simulation_2](./simulation_s2)
* Simulation study generation: 
    * [s0_corridor_500kbps_map_table_count_est_and_no_rate_limit.py](../adaptiveMap/study/s0_corridor_500kbps_map_table_count_est_and_no_rate_limit.py)
    * [s1_corridor_5_to_500kbps_map_count_est_only.py](../adaptiveMap/study/s1_corridor_5_to_500kbps_map_count_est_only.py)
* Simulation analysis:
    * [adaptiveMap.py](../adaptiveMap/analysis/adaptiveMap.py)
    * [rnd_map.py](../adaptiveMap/analysis/rnd_map.py)
    * [s1_corridor_ramp_down.py](../adaptiveMap/analysis/s1_corridor_ramp_down.py)
* Trace generation:
    * [gen_corridor_trace.py](../adaptiveMap/study/gen_corridor_trace.py)

* Output space needed:
    * S1: 2.1 TB
    * S2: 2.8 TB

## Scenario S3/S4

Only 6 seeds are used in this paper. This is the reason for non consecutive sample ids in [simulation_s3_4](./simulation_s3_4).

* Base simulation folder: [simulation/multi_enb](../multi_enb)
* Simulation study folder: [simulation_3_4](./simulation_s3_4)
* Simulation study generation: 
    * [s2_ttl_and_stream.py](../multi_enb/study/s2_ttl_and_stream.py)
* Simulation analysis:
    * [arc_analysis_single_seed.py](../multi_enb/analysis/arc_analysis_single_seed.py)
    * [plot_sumo_traces.py](../multi_enb/analysis/plot_sumo_traces.py)
    * [plot_topography_scenario_s3-4.py](../multi_enb/analysis/plot_topography_scenario_s3-4.py)
* Trace generation:
    * [clean_sumo_traces.py](crownet/simulations/multi_enb/sumo/clean_sumo_traces.py)
    * [plot_trace_map.py](crownet/simulations/multi_enb/sumo/plot_trace_map.py)
    * [run_sumo_simulations.py](crownet/simulations/multi_enb/sumo/run_sumo_simulations.py)
* Output space needed:
    * S3/4: 2.4 TB

## Rerun instructions

In `simulation_sX` the simulation study config is provided. Extract
the trace.tar.gz file in `simulation_sX/additional_rover_files` such that the
symbolic links in `simulation_sX/simulation_runs/Sample__*_0/trace` are valid. 
Replace the `X` with 1,2, or 3_4.

Update runContext.json as this requires absolute paths. Use the crownetutils for this. 
Ensure that the python environment is sourced as described in the documentation. 

```
(crownet_user)$ cd crownet/simulations/2024_ieeeAccess_arc-dsa/simulation_sX
(crownet_user)$ python3 -m crownetutils suqc-context --suqc-dir .
```

To rerun the simulation execute. 

```
(crownet_user)$ cd crownet/simulations/2024_ieeeAccess_arc-dsa/simulation_sX
(crownet_user)$ python3 -m crownetutils suqc-rerun --suqc-dir . --jobs 6

```
**Note space requirements for each scenario!**