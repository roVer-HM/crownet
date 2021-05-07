# Fingerprint Test 
This folder contains fingerprint-based tests for various models.

When a fingerprint test passes, that indicates that with very high
probability, the simulation has followed the same trajectory as when
the fingerprint was recorded, i.e. the times and modules of the events
were the same. That is, a passing fingerprint tests means that the
simulation model logic works the same as before.

However, a passing test does not guarantee that e.g. result recording
has not changed.

Fingerprints are fragile to NED changes and parameter value changes.

When a fingerprint test fails, the simulation's correctness has to be
verified by some other means, and the fingerprints in the tests
updated.

## Containerized Fingerprint Tests

For simulations in the rover research project coupled simulations with 
Vadere and Sumo are necessary. These simulations cannot be started 
a single container but multiple container (one for each simulator) must
be created. Therefore, the fingerprint script is adapted to allow these 
tests.

### Container selection

The test use the default csv file description of test. In the last column (`tag`)
the container tags are encoded in key-value pairs with `;` as delimiter

```
# workingdir,    args,                      [...], tags
/simulation/XX,  -f omnetpp.ini -c fTest01, [...], vadere:latest;omnetpp:latest
/simulation/XX,  -f omnetpp.ini -c fTest02, [...], sumo:latest;omnetpp:latest
```

With the specified tags `vadere:latest;omnetpp:latest` the crownet_fingerprint script 
will create corresponding containers and executes the simulation.

### Configuration and TraCI Hostname conventions

To allow parallel test execution with multiple container pairs the configuration `-c`
and TraCI host names must use the following naming schema:

In /simulation/XX/omnetpp.ini:

```
[Config fTest01]
...
*.traci.launcher.hostname="vadere_simulations_XX_fTest01"
...

[Config fTest02]
...
*.traci.launcher.hostname="sumo_simulations_XX_fTest02"
...

```
The hostname of the traci connection must follow the structure `[simulator]_[workingdir]_[config-name]`
with path separators replaced with `_`. 

## CROWNET Fingerprint Tests

For CROWNET simulations utilizing the control system fingerprint test must be described with yaml files.
Control simulations are configured and executed using a `run_script.py` which creates all necessary 
containers, executes post and pre processing and configures the traci connections between all containers.

See `run_script.py --help` for all possible settings. All arguments starting with `--opp.` will be passed 
to the OMNeT++ executabel used in the simulation. See `opp_run -h` for possible arguments.

### Test definition

One yaml file may contain multiple fingerprint test from multiple locations. Each top level yaml key is one 
test case. 

```yaml
<test_name>:
  wd: simulatioins/XX               (1)
  args:                             (2)
    - run_script.py                 
    - --delete-existing-containers  (3)
    - --create-vadere-container
    - --opp.-c                      (4)
    - final_control
    - --with-control                (5)
    - control.py
  simtimelimit: 100s                (6)
  expectedResult: PASS              (7)
  fingerprint: "file:"              (8)
  tags:                             (9)
    exec: python3
    type: shell
```

1. **wd:** define working directory relative to `$CROWNET_HOME/crownet`. This is the simulation working directory
2. **args:** define the command which will be executed. In most cases this will be `run_script.py` which will start
             all necessary containers.
3. Add all necessary arguments with correct `--` names. 
4. Use `--opp.c` arguments to configure the OMNET executabel. Use `--opp-exec` to set the executable. The default
   OMNeT executable is `$CROWNET_HOME/crownet/src/CROWNET`
5. If this simulation contains control elements configure the control script to execute in the control container.
6. **simtimelimit:** OMNeT++ based simulation time limit after which the test is considered failed.
7. **expectedResult:** Test is successfull if result matches the set value.
8. **fingerprint:** Path to fingerprint file for this test. The default location if only `"file:"` is given will be 
   `hash.d/<yaml-file-name>/<test-name>.csv` relative to the yaml file in which this test is defined. Structure of this 
   see below. If the file does not exist the first run of the test will fail but 
   `hash.d/<yaml-file-name>/<test-name>.csv.UPDATED` will be created.
9. **tags:** Free key-value store to configure test.  


### Fingerprint file definition

```.env
path;               md5_digest
[[omnetpp]];        d1c4-6b00/tplx
/dcdMap_266.csv;    9dd458944c1d57aae8cdd20ac1d2bcd5
/dcdMap_360.csv;    77089921f7afe4a5b74aa159dc696bab
```

The fingerprint file is a simple 2 column csv file with `;` delimiter. The `path` referer to the 
output files created by the given simulation. By default `container_[xxx].out`, `*.sca` and `*.vec` files
are excluded. If OMNeT++ is tested during the test case add the `[[omnet]]` key with the respective 
fingerprint to test against (list of space or comma separated list is accepted, similar to the standard setup)

