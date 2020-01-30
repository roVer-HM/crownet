import glob
import io
import logging
import os
import pprint as pp
import signal
import subprocess
import time
from typing import List

import numpy as np
import pandas as pd

from .configuration import Config


def parse_if_number(s):
    try:
        return float(s)
    except:
        return True if s == "true" else False if s == "false" else s if s else None


def parse_ndarray(s):
    return np.fromstring(s, sep=" ") if s else None


class ScaveTool:
    """
    Python wrapper for OMNeT++ scavetool.

    Allows simple access to query and export functions defined in the scavetool.
    See #print_help, #print_export_help and #print_filter_help for scavetool usage.

    Use #create_or_get_csv_file to create (or use existing) csv files from one or
    many OMNeT++ result files. The method  accepts multiple glob patters which are
    search recursive (default) for files ending in *.vec and *.sca.
    If given a scave_filter is applied to reduce the amount of imported data. See #print_print_filter_help
    on usage.

    Use #load_csv to load an existing OMNeT++ csv file. The following columns are expected to exist.
      'run', 'type', 'module', 'name', 'attrname', 'attrvalue', 'value', 'count', 'sumweights',
      'mean', 'stddev', 'min', 'max', 'binedges', 'binvalues', 'vectime', 'vecvalue'.
    """

    _EXPORT = "x"
    _QUERY = "q"
    _INDEX = "i"
    _OUTPUT = "-o"
    _FILTER = "--filter"

    def __init__(self, config: Config = None, timeout=60):
        if config is None:
            self._config = Config()  # default
        else:
            self._config = config
        self._SCAVE_TOOL = self._config.scave_cmd
        self.timeout = timeout

    @staticmethod
    def _converters():
        return {
            # 'run': ,
            # 'type': ,
            # 'module': ,
            # 'name': ,
            # 'attrname': ,
            "attrvalue": parse_if_number,
            # 'value': ,                    # scalar data
            # 'count': ,                    # scalar data
            # 'sumweights': ,               # scalar data
            # 'mean': ,                     # scalar data
            # 'stddev': ,                   # scalar data
            # 'min': ,                      # scalar data
            # 'max': ,                      # scalar data
            "binedges": parse_ndarray,  # histogram data
            "binvalues": parse_ndarray,  # histogram data
            "vectime": parse_ndarray,  # vector data
            "vecvalue": parse_ndarray,
        }  # vector data

    @classmethod
    def _is_valid(cls, file: str):
        if file.endswith(".sca") or file.endswith(".vec"):
            if os.path.exists(file):
                return True
        return False

    def load_csv(self, csv_file) -> pd.DataFrame:
        """
        #load_csv to load an existing OMNeT++ csv file. The following columns are expected to exist.
          'run', 'type', 'module', 'name', 'attrname', 'attrvalue', 'value', 'count', 'sumweights',
          'mean', 'stddev', 'min', 'max', 'binedges', 'binvalues', 'vectime', 'vecvalue'.
        :param csv_file:    Path to csv file
        :return:            pd.DataFrame with extra namespace 'opp' (an OppAccessor object with helpers)
        """
        df = pd.read_csv(csv_file, converters=self._converters())
        df.opp.attr["csv_path"] = csv_file
        return df

    def create_or_get_csv_file(
        self,
        csv_path,
        input_paths: List[str],
        override=False,
        scave_filter: str = None,
        recursive=True,
    ):
        """
        #create_or_get_csv_file to create (or use existing) csv files from one or
        many OMNeT++ result files. The method  accepts multiple glob patters which are
         search recursive (default) for files ending in *.vec and *.sca.
        If given a scave_filter is applied to reduce the amount of imported data. See #print_print_filter_help
        on usage.
        :param csv_path:        path to existing csv file or path to new csv file (see :param override)
        :param input_paths:     List of glob patters search for *.vec and *.sca files
        :param override:        (default: False) override existing csv_path
        :param scave_filter:    (default: None) string based filter for scavetool see #print_filter_help for syntax
        :param recursive:       (default: True) use recursive glob patterns
        :return:
        """
        if os.path.isfile(csv_path) and not override:
            return os.path.abspath(csv_path)

        cmd = self.export_cmd(
            input_paths=input_paths,
            output=os.path.abspath(csv_path),
            scave_filter=scave_filter,
            recursive=recursive,
        )
        self.exec(cmd)

        return os.path.abspath(csv_path)

    def load_df_from_scave(
        self, input_paths: List[str], scave_filter: str = None, recursive=True,
    ) -> pd.DataFrame:
        """
         Directly load data into Dataframe from *.vec and *.sca files without creating a
         csv file first. Use stdout of scavetool to create Dataframe.

         Helpful variant for automated scripts to reduce memory footprint.

        :param input_paths:     List of glob patters search for *.vec and *.sca files
        :param scave_filter:    (default: None) string based filter for scavetool see #print_filter_help for syntax
        :param recursive:       (default: True) use recursive glob patterns
        :return:
        """
        cmd = self.export_cmd(
            input_paths=input_paths,
            output="-",  # read from stdout of scavetool
            scave_filter=scave_filter,
            recursive=recursive,
            options=["-F", "CSV-R"],
        )
        stdout, stderr = self.read_stdout(cmd)
        if stdout == b"":
            logging.error("error executing scavetool")
            print(str(stderr, encoding="utf8"))
            return pd.DataFrame()

        df = pd.read_csv(
            io.BytesIO(stdout), encoding="utf-8", converters=self._converters()
        )
        df.opp.attr["cmd"] = cmd
        return df

    def export_cmd(
        self, input_paths, output, scave_filter=None, recursive=True, options=None
    ):
        cmd = self._SCAVE_TOOL
        cmd.append(self._EXPORT)
        cmd.append(self._OUTPUT)
        cmd.append(output)
        if scave_filter is not None:
            cmd.append(self._FILTER)
            cmd.append(self._config.escape(scave_filter))

        if options is not None:
            cmd.extend(options)

        if len(input_paths) == 0:
            raise ValueError("no *.vec or *.sca files given.")

        opp_result_files = list()
        for file in input_paths:
            opp_result_files.extend(glob.glob(file, recursive=recursive))

        opp_result_files = [
            f for f in opp_result_files if f.endswith(".vec") or f.endswith(".sca")
        ]

        cmd.extend(opp_result_files)
        return cmd

    def print_help(self):
        cmd = self._SCAVE_TOOL
        cmd.append("--help")
        self.exec(cmd)

    def print_export_help(self):
        cmd = self._SCAVE_TOOL
        cmd.append(self._EXPORT)
        cmd.append("--help")
        self.exec(cmd)

    def print_filter_help(self):
        cmd = self._SCAVE_TOOL
        cmd.append("help")
        cmd.append("filter")
        self.exec(cmd)

    def read_stdout(self, cmd):
        scave_cmd = subprocess.Popen(
            cmd,
            cwd=os.path.curdir,
            stdin=None,
            env=os.environ.copy(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        try:
            out, err = scave_cmd.communicate(timeout=self.timeout)
            return out, err
        except subprocess.TimeoutExpired:
            logging.error("Timout reached")
            scave_cmd.kill()
            return b"", io.StringIO("timeout reached")

    def exec(self, cmd):
        scave_cmd = subprocess.Popen(
            cmd,
            cwd=os.path.curdir,
            shell=False,
            stdin=None,
            env=os.environ.copy(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        try:
            scave_cmd.wait(self.timeout)
            if scave_cmd.returncode != 0:
                logging.error(f"return code was {scave_cmd.returncode}")
                logging.error("command:")
                logging.error(f"{pp.pprint(cmd)}")
                print(scave_cmd.stdout.read().decode("utf-8"))
                print(scave_cmd.stderr.read().decode("utf-8"))

            else:
                logging.info(f"return code was {scave_cmd.returncode}")
                print(scave_cmd.stdout.read().decode("utf-8"))

        except subprocess.TimeoutExpired:
            logging.info(f"scavetool timeout reached. Kill it")
            os.kill(scave_cmd.pid, signal.SIGKILL)
            time.sleep(0.5)
            if scave_cmd.returncode is None:
                logging.error("scavetool still not dead after SIGKILL")
                raise

        logging.info(f"return code: {scave_cmd.returncode}")


tex_module_item_tmpl = """
\t\t\multicolumn{2}{l}{$module}   \\\\
\t\tScalars: $scalar & Vectors: $vector & Histograms: $histogram \\\\[2ex]
"""

tex_module_tmpl = r"""
%% Autogenerated from oppanalyzer. \usepackage{longtable} is needed.
\begin{longtable}{p{0.32\textwidth}p{0.32\textwidth}p{0.32\textwidth}}
    \toprule
    \multicolumn{3}{c}{\textbf{$run}}  \\
    \midrule
    \multicolumn{3}{c}{\emph{Modules Summary}}  \\
    \midrule
$module_items
    \bottomrule
\end{longtable}
"""

tex_tabluar_tmpl = r"""
%% Autogenerated from oppanalyzer. \usepackage{longtable} is needed.
\begin{longtable}{ll}
    \toprule
    \multicolumn{2}{c}{\textbf{$run}}  \\
    \midrule
    \multicolumn{2}{c}{\emph{Attributes}}  \\
    \midrule
$runattrs \\
    \midrule
    \multicolumn{2}{c}{\emph{Iteration varibales}}  \\
    \midrule
$itervars \\
    \midrule
    \multicolumn{2}{c}{\emph{Parameters}}  \\
    \midrule
$params \\
    \bottomrule
\end{longtable}
"""
