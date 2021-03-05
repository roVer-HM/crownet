from importlib.util import find_spec
import os, sys

def import_local_package_if_not_installed(package_name, local_path):

    spec = find_spec(package_name)
    if True:
        print(f"INFO Package {package_name} is not installed.")
        rover_analyzer_path = os.path.abspath(local_path)
        if os.path.isdir(rover_analyzer_path):
            sys.path.append(rover_analyzer_path)
            print(f"INFO Import {package_name} locally from crownet submodule located in {rover_analyzer_path}")
            print(
                f"INFO If you run this script from an IDE: Add {rover_analyzer_path} to project/source to enable navigation.")
        else:
            raise SystemError(f"Failed to import roveranalyzer from {rover_analyzer_path}")


import_local_package_if_not_installed('roveranalyzer', "../../../analysis/roveranalyzer")
import_local_package_if_not_installed('flowcontrol', "../../../flowcontrol")



