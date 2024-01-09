# Parameter studies

For conducting parameter studies, we use the Python framework [suq-controller](../../analysis/suq-controller) which stands for `Surrogante and Uncertainty Quantification controller`.

## Setup and running parameter studies
To test parameter combinations (samples), the parameter values need to be specified in a list:
```
    par_var = [
        {'vadere': {'sources.[id==1].distributionParameters.numberPedsPerSecond': 0.0375}, # first list item
         'omnet': {'*.misc[0].app[0].incidentTime': '10s',
                   '*.radioMedium.obstacleLoss.typename': 'DielectricObstacleLoss'}},
        {'vadere': {'sources.[id==1].distributionParameters.numberPedsPerSecond': 0.05}, # second list item
         'omnet': {'*.misc[0].app[0].incidentTime': '12s',
                   '*.radioMedium.obstacleLoss.typename': 'DielectricObstacleLoss'}}
    ]
```
Note that each list item is a dictionary that assigns the parameters to the respective simulator. For example,
`*.misc[0].app[0].incidentTime` is a parameter of a network model (Omnet++ simulator) and `sources.[id==1].distributionParameters.numberPedsPerSecond` 
is a parameter of the mobility provider Vadere.

The quantities of interest (output variable) are also specified in a list:
```
    qoi = [
        "degree_informed_extract.txt",
        "poisson_parameter.txt",
        "time_95_informed.txt",
    ]
```
The corresponding quantities of interest will be automatically collected.

Before running the simulation, the corresponding simulation type needs to be specified. 
For example, a coupled simulation (Vadere, Omnet++) is specified over the command
```
    sim_type = VadereOppCommand() \
        .create_vadere_container() \
        .vadere_tag("latest") \
        .omnet_tag("latest") \
        .qoi(qoi) \
        .experiment_label("out") \
```
To run the parameter studies, simply run

```
        setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=qoi,
        model=sim_type,
        post_changes=None,
        output_path=path2tutorial,
        output_folder=output_folder,
        remove_output=False,
        env_remote=None,
    )
    setup.run()
```


## Tutorials

Please find the `*_rover_*` tutorials in the [suq-controller tutorials](../../analysis/suq-controller/tutorial).