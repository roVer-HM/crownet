import os
import logging
import subprocess
import signal
import time
import pprint as pp
from typing import List

import numpy as np
import pandas as pd
import glob

from .configuration import Config


def parse_if_number(s):
    try:
        return float(s)
    except:
        return True if s == "true" else False if s == "false" else s if s else None


def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None


class ScaveTool:
    _EXPORT = 'x'
    _QUERY = 'q'
    _INDEX = 'i'
    _OUTPUT = '-o'
    _FILTER = '--filter'

    def __init__(self, config: Config = None):
        if config is None:
            self._config = Config()  # default
        else:
            self._config = config
        self._SCAVE_TOOL = self._config.scave_cmd

    def load_csv(self, csv_file):
        df = pd.read_csv(csv_file, converters=self._converters())
        df.opp.set_attr('scave_filter', 'N/A')
        return df

    @staticmethod
    def _converters():
        return {
            # 'run': ,
            # 'type': ,
            # 'module': ,
            # 'name': ,
            # 'attrname': ,
            'attrvalue': parse_if_number,
            # 'value': ,                    # scalar data
            # 'count': ,                    # scalar data
            # 'sumweights': ,               # scalar data
            # 'mean': ,                     # scalar data
            # 'stddev': ,                   # scalar data
            # 'min': ,                      # scalar data
            # 'max': ,                      # scalar data
            'binedges': parse_ndarray,      # histogram data
            'binvalues': parse_ndarray,     # histogram data
            'vectime': parse_ndarray,       # vector data
            'vecvalue': parse_ndarray}      # vector data

    @classmethod
    def _is_valid(cls, file: str):
        if file.endswith('.sca') or file.endswith('.vec'):
            if os.path.exists(file):
                return True
        return False

    def create_or_get_csv_file(self, csv_path, input_paths: List[str], override=False, scave_filter: str = None):
        if os.path.isfile(csv_path) and not override:
            return os.path.abspath(csv_path)

        cmd = self._SCAVE_TOOL
        cmd.append(self._EXPORT)
        cmd.append(self._OUTPUT)
        cmd.append(os.path.abspath(csv_path))
        if scave_filter is not None:
            cmd.append(self._FILTER)
            cmd.append(self._config.escape(scave_filter))

        if len(input_paths) == 0:
            raise ValueError("no *.vec or *.sca files given.")

        opp_result_files = list()
        for file in input_paths:
            opp_result_files.extend(glob.glob(file, recursive=True))

        opp_result_files = [f for f in opp_result_files if f.endswith(".vec") or f.endswith(".sca")]

        cmd.extend(opp_result_files)
        self.exec(cmd)

        return os.path.abspath(csv_path)

    def print_filter_help(self):
        cmd = self._SCAVE_TOOL
        cmd.append('help')
        cmd.append('filter')
        self.exec(cmd)

    @staticmethod
    def exec(cmd):
        scave_cmd = subprocess.Popen(cmd, cwd=os.path.curdir, shell=False,
                                           stdin=None,
                                           env=os.environ.copy(),
                                           stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE)

        try:
            scave_cmd.wait(20)
            if scave_cmd.returncode != 0:
                logging.error(f"return code was {scave_cmd.returncode}")
                logging.error("command:")
                logging.error(f"{pp.pprint(cmd)}")
                print(scave_cmd.stdout.read().decode('utf-8'))
                print(scave_cmd.stderr.read().decode('utf-8'))

            else:
                logging.info(f"return code was {scave_cmd.returncode}")
                print(scave_cmd.stdout.read().decode('utf-8'))

        except subprocess.TimeoutExpired:
            logging.info(f"scavetool timeout reached. Kill it")
            os.kill(scave_cmd.pid, signal.SIGKILL)
            time.sleep(0.5)
            if scave_cmd.returncode is None:
                logging.error("scavetool still not dead after SIGKILL")
                raise

        logging.info(f"return code: {scave_cmd.returncode}")

