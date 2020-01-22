# oppanalyzer

Small collection of tools to access `*.vec` and 
`*.sca` files produced by OMNeT++. The *oppanalyzer*
wraps the `scavetool` provided by OMNeT++ to 
access, filter and combine results from OMNeT++ simulation
campaigns.

## Usage

```python
from oppanalyzer import *

cfg = Config()
scavetool = ScaveTool(cfg) 

scavetool.load_csv('some.csv')

```

The default configuration `Config()` will use 
containrized a version of the scavetool, which means
the `$ROVER_MAIN` variable must be set to the 
root of this repository. See [tutorial/README](tutorial/README.md)
for configuration setup if you dont use the containers.

## ToDo's

- [ ] Define standard transformation as helpers.
- [ ] Read Vadere output
- [ ] Integrate [suq-controler](https://gitlab.lrz.de/vadere/suq-controller)