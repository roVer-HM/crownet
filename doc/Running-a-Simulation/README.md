# Running a simulation

CrowNet simulations can be run with or without an IDE. Docker containers are required for all variants.
To run a simulation, multiple Docker containers must run in parallel.
This can be done in headless mode or IDE mode:
- [Headless mode](StartUp-Terminal.md): containers run in parallel, debugging is not available.
- [IDE mode](StartUp-GUI.md) (recommended for developers): each container runs with IDE support, which enables debugging.

If you do not want to start each container manually in headless mode, run `python run_script.py` in the corresponding simulation folder.
For more information, see:
- [Simple headless mode](StartUp-RunScript.md)





