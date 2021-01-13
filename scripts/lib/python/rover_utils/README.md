# Rover Python Utils

## Install

Install requirements. If you use a virtual environment ensure you activated it before executing the line below

```
pip3 install -r requirements.txt
```

## Usage in Scripts under `crownet/scripts`

add the following snippet at the top of each python script in crownet/scripts to load
the rover_utils package

```python
# check CROWNET_HOME and add rover_utils to package search path
if "CROWNET_HOME" not in os.environ:
    print("CROWNET_HOME not set")
    exit(-1)
rover_utils_path = os.path.join(
    os.environ["CROWNET_HOME"], "scripts/lib/python/rover_utils/rover_utils"
)
if os.path.isdir(rover_utils_path):
    sys.path.append(rover_utils_path)
else:
    print(f"Cannot find rover_utils at {rover_utils_path}")
    exit(-1)
try:
    import rover_utils as utl
except ModuleNotFoundError as e:
    rover_utils_readme = os.path.join(
        os.environ["CROWNET_HOME"], "scripts/lib/python/rover_utils/README"
    )
    print(e.msg)
    print(
        "  -> some requirements of rover_utils are not met. Did you install all requirements?"
    )
    print(f"  -> see {rover_utils_readme} for help")
    exit(-1)
```
