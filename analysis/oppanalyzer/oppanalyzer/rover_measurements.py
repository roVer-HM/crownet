import pandas as pd
import numpy as np
import os
import sys


class OaiMeasurement:
    """
    Process  OpenAirInterface Measurements

    Reads and processes measurement traces recorded with the SAM measurement tool.
    """

    @staticmethod
    def read_packet_trace(csv_file: str) -> pd.Series:
        """
        #read_packet_trace reads a packet trace and returns it in form of a series which can be
        processed by the plotting methods in this package.

        :param csv_file:    Path to csv file
        :return:            pd.Series with vectime and vecvalue (as used within simulation traces)
        """
        if not os.path.isfile(csv_file):
            sys.exit(f"File: \"{csv_file}\" not found.\n" +
                     "Measurement data not found - maybe you need to download the data by executing the download script first?")

        measure = pd.read_csv(csv_file, names=["r_seq", "vectime", "s_seq", "s_time", "vecvalue"], header=0)

        return  pd.Series({
            "vectime": (measure['vectime'] - np.min(measure['vectime'])),
            "vecvalue": measure['vecvalue']
        })
