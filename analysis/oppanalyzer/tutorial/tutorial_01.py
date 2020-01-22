import sys

from tutorial.imports import *

# Structure based on suq-controller (Daniel Lehmberg)
# This is just to make sure that the systems path is set up correctly, to have correct imports.
# (It can be ignored)
sys.path.append(
    os.path.abspath(".")
)  # in case tutorial is called from the root directory
sys.path.append(os.path.abspath(".."))  # in tutorial directly


# Config object sets the path to the scavetool based on
# the $ROVER_MAIN environment variable. If $ROVER_MAIN is not set
# the config guesses the location an prints a warning. Below an
# example to override the default behaviour
cfg = Config()
# cfg = Config(
#     use_docker_container=True,              # If False the scave_tool_cmd must be in the $PATH
#     rover_main="/opt/rover-main",
#     opp_container_path="scripts/omnetpp",   # relative to $ROVER_MAIN
#     scave_tool_cmd="scavetool",
# )


def scavetool_help():
    """
    The oppanalyzer wraps the scavetool provided by OMNeT++ to read, parse and filter
    scalar and vector files. See the export_cmd() and cmd() methods to access the tool
    directly.
    """
    scavetool = ScaveTool(cfg)  # if default setting just use ScaveTool()
    scavetool.print_help()
    print("#" * 80)
    scavetool.print_export_help()
    print("#" * 80)
    scavetool.print_filter_help()


def read_from_csv_file():
    """
    load a pandas DataFrame from a csv file exported previously from one or more *.vec/*.sca files
    """
    scavetool = ScaveTool(cfg)
    dataframe = scavetool.load_csv(os.path.join(path2data, "tutorial_01_result.csv"))
    print(f"Columns: {dataframe.columns}")
    print("#" * 80)
    print(dataframe.head())
    return dataframe


def create_csv_fiel_from_sca_and_vec_files():
    """
    csv_path: path to new or existing csv file
    input_paths: list of glob patterns (paths) to search for *.vec and *.sca files to combine
    override: check if csv_path already exist and override it if set
    scave_filter: filter content of input_paths based on the syntax shown in scavetool.print_filter_help()
    recursive: decide if input_paths should be search recursively
    """
    scavetool = ScaveTool(cfg)
    dataframe = scavetool.create_or_get_csv_file(
        csv_path=os.path.join(path2data, "tutorial_01_result.csv"),
        input_paths=[os.path.join(path2data, "tutorial_01.*")],
        override=True,
        scave_filter=None,
        recursive=True,
    )
    return dataframe


def apply_scavetool_filters():
    """
    load_df_from_scave will NOT create a csv file but will read data allays from
    OMNeT++ result files. This will save disk space because the data is not duplicated
    but development will be slower, because on each start of the script all input files
    must be read from disk. Use this method in automated environments only.

    The use of scave_filter can be used by the import methods shown above.

    Match expression example:
      module("**.sink") AND (name("queueing time") OR name("transmission time"))
    :return:
    """
    scavetool = ScaveTool(cfg)
    dataframe = scavetool.load_df_from_scave(
        input_paths=[os.path.join(path2data, "tutorial_01.*")],
        scave_filter='module("*lteNic.channelModel*")',
        recursive=True,
    )
    print(dataframe.head())
    return dataframe


if __name__ == "__main__":
    # scavetool_help()
    df = read_from_csv_file()
    # df = create_csv_fiel_from_sca_and_vec_files()
    # df = apply_scavetool_filters()
