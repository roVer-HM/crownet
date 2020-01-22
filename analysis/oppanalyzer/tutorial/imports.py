import glob
import os
import urllib
import zipfile

from oppanalyzer import *

if not os.path.isdir("test_data"):
    print(f"Downloading test_data.zip")
    # downloads file if it does not exist in this folder
    urllib.request.urlretrieve(
        "https://syncandshare.lrz.de/dl/fiMAmiaumaW3ASu9xf9JepYi/oppanalyzer_test_data_20200122.zip",
        "test_data.zip",
    )
    with zipfile.ZipFile("test_data.zip", "r") as zip_ref:
        zip_ref.extractall("test_data")

    os.remove("test_data.zip")

    files = glob.glob("./test_data/**", recursive=True)
    print(f"{os.path.abspath('.')}:")
    for f in files:
        print(f"  |-- {f[2:]}")


path2tutorial = os.path.dirname(os.path.relpath(__file__))
path2data = os.path.join(path2tutorial, "test_data")
