# Running a simulation

CrowNet simulation can be run with and without IDE. Docker containers are required for all variants.
For running a simulation, multiple Docker containers need to be run in parallel.
This can be done in headless or IDE mode, see:
- [Headless mode](StartUp-Terminal.md): the containers are executed in parallel, debugging not possible
- [IDE mode](StartUp-GUI.md) (recommended for developers), each container is executed in a respective IDE which allows debugging

If you do not want to start each container manually in headless mode, you can run the `python run_script.py` in the corresponding simulation folder.
For more information on this mode, see:
- [Simple headless mode](StartUp-RunScript.md)





