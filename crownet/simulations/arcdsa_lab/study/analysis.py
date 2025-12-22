#!/usr/bin/env python
# coding: utf-8
# result analysis script
import math
# TODO:
# Performance: calculate rates only once (multiple threads)
# 1) Tables for variance and mean for all
# 2) Adaptation delay sim-NR, sim-WLAN, meas-WLAN  (first: define "adapted" - std.dev for 1s <= ...


from dataclasses import dataclass
from typing import Any

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pylab as pylab
from matplotlib.ticker import MaxNLocator
import tarfile
import os
import logging
from pathlib import Path

from pandas import DataFrame
import warnings

SIM_RESULTS_PATH = "../results"
MEAS_RESULTS_PATH = "../measurements"
USE_CACHE = True

# dictionary with all measurement configurations and corresponding simulation configurations (one or more)
config_meas_to_sim = {"05-09-2025/scenarios/always_rate" : ["upto-14-ue-WLAN", "upto-14-ue-NR"],
                   "05-09-2025/scenarios/ramp_rate" : ["upto-14-ue-ramp-WLAN", "upto-14-ue-ramp-NR"],
                   "05-09-2025/scenarios/burst_rate" : ["upto-14-ue-burst-WLAN", "upto-14-ue-burst-NR"],
                   "03-09-2025/scenarios/always": ["noarc-upto-14-ue-WLAN"],
                   "03-09-2025/scenarios/ramp": ["noarc-upto-14-ue-ramp-WLAN"],
                   "03-09-2025/scenarios/burst": ["noarc-upto-14-ue-burst-WLAN"],
                    }

# config_meas_to_sim = {"05-09-2025/scenarios/burst_rate" : [ "upto-14-ue-burst-WLAN", "upto-14-ue-burst-NR" ] }



repeat  = 4  # number of repetitions (see omnetpp.ini)
ue_indexes = np.arange(0,14)  # maximum number of UEs in our scenarios: 14
# use the next line for labels corresponding to the OMNeT++ nomenclature (ue[0]...)
# ue_labels = ["total"] + [f"ue[{i}]" for i in range(14)]
# use the next line for labels corresponding to the testbed nomenclature (N1, N2, ...)
ue_labels = ["total"] + [f"N{i+1}" for i in range(14)]
# application indexes to be evaluated
# (Note that the index must be mapped to the corresponding name in the measurements!)
app_indexes = { 0 : "beacon",  # beacon app
                # 1 : "message"  # map app
                }
app_limits  = [16000, 500000]
averaging_window_size = 5  # [s]
show_plots = False
plot_resolution = 1000     # number of plotted data points in plot  (set to -1 to disable resampling)

# colors for plots
PLOT_COLORS = ['black', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown',
                'tab:pink', 'tab:gray', 'tab:olive', 'tab:cyan', 'tab:blue',
                'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown']

logger = logging.getLogger(__name__)

def parse_if_number(s):
    try: return float(s)
    except ValueError: return True if s=="true" else False if s=="false" else s if s else None

def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None

# Data class for simulation and measurement data
@dataclass
class SMData:
    vectimes: np.ndarray
    vecvalues: np.ndarray
    vecreps: np.ndarray
    vecues: np.ndarray
    
    @staticmethod
    def create_empty() -> "SMData":
        return SMData(
            vectimes=np.empty([0]),
            vecvalues=np.empty([0]),
            vecreps=np.empty([0]),
            vecues=np.empty([0])
        )

    def append_smdata(self, other: "SMData"):
        self.append_smvalues(other.vectimes, other.vecvalues, other.vecreps, other.vecues)
    
    def append_smvalues(self, times, values, reps, ues):
        self.vectimes = np.concatenate([self.vectimes, times])
        self.vecvalues = np.concatenate([self.vecvalues, values])
        self.vecreps = np.concatenate([self.vecreps, reps])
        self.vecues = np.concatenate([self.vecues, ues])
    
    def to_dataframe(self):
        df = pd.DataFrame(
            np.column_stack([self.vecreps, self.vectimes, self.vecvalues, self.vecues]),
            columns=['vecrep', 'vectime', 'vecvalue', 'vecue']
        )
        df.set_index('vectime', inplace=True)
        return df

    def convert_vectime_to_relative_time(self):
        minimum_time = np.min(self.vectimes)
        if minimum_time > 0:
            self.vectimes = self.vectimes - minimum_time

def read_simulation_data(input_file: str) -> pd.DataFrame:
    logger.info(f'Reading simulation results {input_file}')
    data = pd.read_csv(f"{input_file}", converters = {
    'attrvalue': parse_if_number,
    'binedges': parse_ndarray,
    'binvalues': parse_ndarray,
    'vectime': parse_ndarray,
    'vecvalue': parse_ndarray})
    vectors = data[data.type=='vector']

    return vectors

def read_measurement_data(input_file: str) -> pd.DataFrame:
    base_name, _ = os.path.splitext(input_file)

    # Check if the file exists
    if not os.path.exists(input_file):
        # Construct tar.gz filename from the input file path
        file_path = Path(input_file)
        directory = file_path.parent
        tgz_directory = Path(MEAS_RESULTS_PATH)

        # check if we have compressed file and can unpack it
        tar_gz_files = list(tgz_directory.glob("*.tar.gz"))
        if tar_gz_files:
            tar_gz_file = tar_gz_files[0]
        else:
            raise FileNotFoundError(f"Neither {input_file} nor tar.gz archive found in {directory}")
        
        # Extract the tar.gz file
        logger.info(f"Extracting {tar_gz_file} to {tgz_directory}...")
        with tarfile.open(tar_gz_file, 'r:gz') as tar:
            tar.extractall(path=tgz_directory)
        logger.info(f"Extraction complete.")
    
    # Read the JSON file
    with open(input_file, 'r') as f:
        logger.info(f'Reading measurement data {input_file}')
        data = pd.read_json(f)

    # Convert timestamp column to datetime if it's not already
    data['timestamp'] = pd.to_datetime(data['timestamp'])

    # Convert to seconds since Unix epoch (1970-01-01)
    data['vectime'] = data['timestamp'].astype('int64') / 1e9

    # Measurements are in bytes, convert to bits
    data.rename(columns={'sizeBytes': 'vecvalue'}, inplace=True)
    data['vecvalue'] = data['vecvalue'] * 8

    return data


def read_measurement_dataframe(app_idx: int, conf_meas: str) -> DataFrame:
    cache_path = f"{MEAS_RESULTS_PATH}/{conf_meas}.h5"
    if os.path.exists(f"{cache_path}") and USE_CACHE:
        logger.info(f"Reading measurement data from cache {cache_path}")
        return pd.read_hdf(f"{cache_path}", key='df_meas')

    # read the corresponding measurement data from individual log files
    meas_data = SMData.create_empty()
    for i in range(repeat):
        sum_vectime = np.empty([0])
        sum_vecvalue = np.empty([0])
        new_data = SMData.create_empty()
        for ue in ue_indexes:  # iterate over all ues
            uedata = read_measurement_data(
                f"{MEAS_RESULTS_PATH}/{conf_meas}/{app_indexes[app_idx]}/node-{(ue + 1)}/{(i + 1):02d}.json")

            vecrep = np.ones(uedata.timestamp.size, dtype=int) * i
            vecue = np.ones(uedata.timestamp.size, dtype=int) * ue

            new_data.append_smvalues(uedata.vectime, uedata.vecvalue, vecrep, vecue)

            sum_vecvalue = np.concatenate([sum_vecvalue, uedata.vecvalue])
            sum_vectime = np.concatenate([sum_vectime, uedata.vectime])

        # additional rows with all values and ue == -1
        # for the sum over all (use ue index -1 later for calculating sum)
        vecrep = np.ones(sum_vectime.size, dtype=int) * i
        vecue = np.ones(sum_vectime.size, dtype=int) * -1
        new_data.append_smvalues(sum_vectime, sum_vecvalue, vecrep, vecue)

        # now we have all data for one repetition - convert to relative time
        # (repetitions may have different start times)
        new_data.convert_vectime_to_relative_time()
        meas_data.append_smdata(new_data)

    df_meas = meas_data.to_dataframe()

    # store in HDF5 as cache
    logger.info(f'Storing measurement data in HDF5 cache {cache_path}')
    df_meas.to_hdf(f"{cache_path}", key='df_meas', mode='w')

    return df_meas


def read_simulation_dataframe(app_idx: int, conf_sim: str) -> DataFrame:
    cache_path = f"{SIM_RESULTS_PATH}/{conf_sim}.h5"
    if os.path.exists(f"{cache_path}") and USE_CACHE:
        logger.info(f"Reading simulation data from cache {cache_path}")
        return pd.read_hdf(f"{cache_path}", key='df_sim')

    sim_data = SMData.create_empty()
    for i in range(repeat):  # iterate over all repetitions of the simulation
        alldata = read_simulation_data(f"{SIM_RESULTS_PATH}/{conf_sim}/rep_{i}/{conf_sim}_vec.csv")
        for ue in ue_indexes:  # iterate over all ues
            vec_plt = extract_tx_packets_data(alldata, [ue], app_idx)
            if vec_plt is None:
                continue
            vecrep = np.ones(vec_plt.vectime.size, dtype=int) * i
            vecue = np.ones(vec_plt.vectime.size, dtype=int) * ue
            sim_data.append_smvalues(vec_plt.vectime, vec_plt.vecvalue, vecrep, vecue)

        # add one item for the sum over all (use ue index -1 for this)
        vec_plt = extract_tx_packets_data(alldata, ue_indexes, app_idx)
        if vec_plt is None:
            logger.error("No data to analyse - did you convert the correct result files to .csv?")
            exit(100)
        vecrep = np.ones(vec_plt.vectime.size, dtype=int) * i
        vecue = np.ones(vec_plt.vectime.size, dtype=int) * -1
        sim_data.append_smvalues(vec_plt.vectime, vec_plt.vecvalue, vecrep, vecue)

    # combine all values and corresponding times in one dataframe
    df_sim = sim_data.to_dataframe()

    # store in HDF5 as cache
    logger.info(f'Storing simulation data in HDF5 cache {cache_path}')
    df_sim.to_hdf(f"{cache_path}", key='df_sim', mode='w')

    return df_sim

def cut_dataframes(df_meas: DataFrame, df_sim: DataFrame, start: float, end: float) -> tuple[DataFrame, DataFrame]:
    df_meas.reset_index(drop=True, inplace=True)
    df_meas = df_meas.iloc[(math.floor(start * len(df_meas))):(math.ceil(end * len(df_meas)))]
    df_meas.reset_index(drop=True, inplace=True)
    df_sim.reset_index(drop=True, inplace=True)
    df_sim = df_sim.iloc[(math.floor(start * len(df_sim))):(math.ceil(end * len(df_sim)))]
    df_sim.reset_index(drop=True, inplace=True)
    logger.info(f"Dataframe size after reducing: simulation={len(df_sim)}, measurement={len(df_meas)}")
    return df_meas, df_sim

def extract_tx_packets_data(vectors, ue_index_array, app_index):
    # note: in the Lab, all UEs are in the same location - we observe incoming data at one of the nodes
    # vec = vectors[(vectors.module == f"LabSingleCellCrowNet.ue[{ue_index}].app[{app_index}].app") ]
    # vec_app_out = vec[vec.name == 'packetSent:vector']
    # vec = vectors[(vectors.module == f"LabSingleCellCrowNet.ue[{ue_index_array}].app[{app_index}].packetMeterIn")]
    # vec_app_out = vec[vec.name == 'outgoingPacketLengths:vector']
    # vec = vectors[ vectors.module.isin([f"LabSingleCellCrowNet.ue[{x}].app[{app_index}].packetMeterIn" for x in ue_index_array])]
    vec = vectors[
        vectors.module.isin([f"LabSingleCellCrowNet.ue[{x}].app[{app_index}].packetMeterOut" for x in ue_index_array])
        & vectors.name.isin(['outgoingPacketLengths:vector'])]
    # vec_app_out = vec[vec.name == 'outgoingPacketLengths:vector']
    # vec = vectors[(vectors.module == f"LabSingleCellCrowNet.ue[{ue_index}.app[{app_index}].packetMeterOut")]
    # vec_app_out = vec[vec.name == 'outgoingPacketLengths:vector']

    # vec_data = vec.iloc[0]
    rel_data = vec.iloc[:,6:]

    # if we have no data, do not return anything
    if rel_data.shape[0] == 0:
        return None

    # Concatenate all series in 'vectime' and 'vecvalue'
    merged_vectime = np.concatenate(rel_data['vectime'].to_numpy())
    merged_vecvalue = np.concatenate(rel_data['vecvalue'].to_numpy())

    # create a new DataFrame from the concatenated series
    merged_df = pd.DataFrame({'vectime': merged_vectime, 'vecvalue': merged_vecvalue})
    return merged_df

def configure_font_sizes():
    plt.rc('font', size=14)
    plt.rc('axes', titlesize=16)
    plt.rc('axes', labelsize=14)
    plt.rc('xtick', labelsize=12)
    plt.rc('ytick', labelsize=12)
    plt.rc('legend', fontsize=12)
    plt.rc('figure', titlesize=18)

    # for camera-ready submissions, often only type 1 fonts are allowed
    params = {'pdf.fonttype': 42}
    pylab.rcParams.update(params)

def calculate_resampling_factor(nr_data_points: int):
    if plot_resolution > 0:
        factor = math.floor(nr_data_points / plot_resolution)
        if factor == 0:
            factor = 1
    else:
        factor = 1

    return factor

def plot_packet_size(vec_plt_tmp: pd.DataFrame, conf_name: str, ue_idxs: np.ndarray[tuple[Any, ...]]):
    # Plot 1: packet sizes
    vec_plt_tmp = vec_plt_tmp.sort_index()
    fig1, ax1 = plt.subplots()
    # loop over all ue indexes to be plotted (index -1 is for sum)
    for u in ue_idxs:
        if (vec_plt_tmp['vecue'] == u) is None:
            continue
        v = vec_plt_tmp.loc[(vec_plt_tmp['vecue'] == u)]
        # plot data for single ue (index 0 is label for -1: sum)
        # value devided by 8 since we need to convert bits to bytes
        ax1.plot(v["vectime"], v["vecvalue"] / 8, 'o', label=ue_labels[u+1])
    # plt.xlim(0,100)
    fig1.savefig(f"{conf_name}_packets_out.png", format='png', dpi=300)
    fig1.savefig(f"{conf_name}_packets_out.pdf", format='pdf')
    if show_plots:
        plt.show()
    else:
        plt.close(fig1)

def calculate_rates(avg_window_size: int, ue_idxs: np.ndarray[tuple[Any, ...], np.dtype[Any]], vec_plt: DataFrame) -> \
tuple[list[Any], list[Any], list[Any], list[Any]]:

    ue_rate = []
    ue_rate_stddev = []
    all_time_points = []
    rate_calc_time_points = np.arange(vec_plt.index.min(), vec_plt.index.max(), avg_window_size)

    # Store active status data for stacked plot
    active_data = []

    for u in ue_idxs:
        v = vec_plt.loc[(vec_plt['vecue'] == u)]

        # Skip if no data for this UE
        if v.empty:
            continue

        average_rate = np.zeros(repeat)
        data_rate = []
        time_points = []
        data_rate_stddev = []

        # Calculate the data rate averaged over the window
        for start_time in rate_calc_time_points:
            end_time = start_time + avg_window_size
            for r in range(repeat):
                mask = (v.index > start_time) & (v.index <= end_time) & (v.vecrep == r)

                # Sum the packet sizes in this window
                total_bits = np.sum(v.vecvalue[mask])

                # Calculate the data rate (bits per second)
                average_rate[r] = total_bits / avg_window_size

            data_rate_stddev.append(np.std(average_rate))
            data_rate.append(np.mean(average_rate))
            time_points.append(start_time)

        # Convert the data rate list to a numpy array
        data_rate = np.array(data_rate)
        data_rate_stddev = np.array(data_rate_stddev)
        time_points = np.array(time_points)

        ue_rate.append(data_rate)
        ue_rate_stddev.append(data_rate_stddev)
        all_time_points.append(time_points)

        # Store active status (1 if data_rate > 0, else 0) for stacked plot
        active_status = np.where(data_rate > 0, 1, 0)
        active_data.append(active_status)

    return all_time_points, ue_rate, ue_rate_stddev, active_data

# Note: For calculating the rate difference and variance, we assume stochastic independence of
#       the two inputs A and B.
def calculate_delta(all_time_points_a, ue_plot_data_a, ue_plot_stddev_a,
                    all_time_points_b, ue_plot_data_b, ue_plot_stddev_b) -> \
tuple[list[Any], list[Any], list[Any], list[Any]]:
    epsilon = 0.02
    ue_rate_delta = []
    ue_rate_stddev = []
    all_time_points = []
    active_data = []

    for ue_idx in range(len(ue_plot_data_a)):
        delta_rate = []
        time_points = []
        delta_rate_stddev = []
        delta_active_status = []
        for i in range(len(all_time_points_a[ue_idx])):
            if abs(all_time_points_a[ue_idx][i] - all_time_points_b[ue_idx][i]) > epsilon:
                logger.warning(f"Time points for A and B are not equal: {all_time_points_a[ue_idx][i]} "
                               f"and {all_time_points_b[ue_idx][i]} - skipping time point")
                continue
            delta_rate.append(ue_plot_data_a[ue_idx][i] - ue_plot_data_b[ue_idx][i])
            delta_rate_stddev.append(ue_plot_stddev_a[ue_idx][i] + ue_plot_stddev_b[ue_idx][i])
            if ue_plot_data_a[ue_idx][i] > 0 or ue_plot_data_b[ue_idx][i] > 0:
                delta_active_status.append(1)
            else:
                delta_active_status.append(0)
            time_points.append(all_time_points_a[ue_idx][i])
        delta_rate = np.array(delta_rate)
        delta_rate_stddev = np.array(delta_rate_stddev)
        time_points = np.array(time_points)
        delta_active_status = np.array(delta_active_status)

        ue_rate_delta.append(delta_rate)
        ue_rate_stddev.append(delta_rate_stddev)
        all_time_points.append(time_points)
        active_data.append(delta_active_status)

    return all_time_points, ue_rate_delta, ue_rate_stddev, active_data


def plot_rates(df: pd.DataFrame, avg_window_size: int, conf_name: str,
               ue_idxs: np.ndarray, target_rate: float) -> None:
    df = df.sort_index()
    fig1, (ax1_top, ax1_bottom) = plt.subplots(2, 1, figsize=(10, 8), sharex=True,
                                               gridspec_kw={'height_ratios': [3, 1]})

    # plot reference line in top subplot
    ax1_top.axhline(y=target_rate/1000, color='r', linestyle='--',
                label=f'Target')
                # label=f'Target ({target_rate/1000} kbit/s)')

    all_time_points, ue_plot_data, ue_plot_stddev, active_data = calculate_rates(avg_window_size, ue_idxs, df)

    for i in range(len(ue_plot_data)):
        # Plot the average data rate over time in top subplot
        ax1_top.plot(all_time_points[i], ue_plot_data[i] / 1000.0, label=ue_labels[i], color=PLOT_COLORS[i])
        # Plot the standard deviation of the data rate over time in top subplot
        ax1_top.fill_between(all_time_points[i], ue_plot_data[i] / 1000.0 - ue_plot_stddev[i] / 1000.0,
                            ue_plot_data[i] / 1000.0 + ue_plot_stddev[i] / 1000.0, alpha=0.2, color=PLOT_COLORS[i])

    # Create stacked plot in bottom subplot (ignore first in active data list, it is the sum activity)
    if active_data:
        ax1_bottom.stackplot(all_time_points[0], *active_data[1:], colors=PLOT_COLORS[1:], alpha=0.8)
        # ax1_bottom.legend(loc='upper left', bbox_to_anchor=(1, 1))

    ax1_bottom.set_xlabel('Time (s)')
    ax1_bottom.set_ylabel('Active UEs (stacked)')
    ax1_bottom.yaxis.set_major_locator(MaxNLocator(integer=True))
    ax1_top.set_ylabel('Data Rate (kbit/s)')
    # plt.title('Average Data Rate')
    # Place the legend outside
    ax1_top.legend(loc='upper left', bbox_to_anchor=(1, 1))
    # Adjust the plot area to remove space for the legend
    plt.tight_layout()
    # plt.legend()
    ax1_top.grid(True)
    ax1_bottom.grid(True, axis='x')
    fig1.savefig(f"{conf_name}_rate_{avg_window_size}.png", format='png', dpi=300)
    fig1.savefig(f"{conf_name}_rate_{avg_window_size}.pdf", format='pdf')
    if show_plots:
        plt.show()
    else:
        plt.close(fig1)

def plot_rate_comp(df_sim: pd.DataFrame, df_meas: pd.DataFrame, avg_window_size: int, conf_name: str,
               ue_idxs: np.ndarray, target_rate: float) -> None:
    plot_rate_comp_a_to_b(df_sim, df_meas, "Simulation", "Measurement", avg_window_size, conf_name,
                          ue_idxs, target_rate)

def plot_rate_difference(df_a: pd.DataFrame, df_b: pd.DataFrame, label_delta: str, avg_window_size: int,
                         conf_name: str, ue_idxs: np.ndarray) -> None:
    plot_rate_comp_a_to_b(df_a, df_b, label_delta, "", avg_window_size, conf_name, ue_idxs, 0.0, delta=True)

def plot_rate_comp_a_to_b(df_a: pd.DataFrame, df_b: pd.DataFrame, label_a: str, label_b: str,
                          avg_window_size: int, conf_name: str,
                          ue_idxs: np.ndarray, target_rate: float, delta: bool = False) -> None:
    df_a  = df_a.sort_index()
    df_b = df_b.sort_index()

    fig1, ax1 = plt.subplots()

    logger.info(f"Calculating rates (A) for {conf_name} with avg_window_size={avg_window_size}")
    all_time_points_a, ue_plot_data_a, ue_plot_stddev_a, active_data_a \
        = calculate_rates(avg_window_size, ue_idxs, df_a)
    logger.info(f"Calculating rates (B) for {conf_name} with avg_window_size={avg_window_size}")
    all_time_points_b, ue_plot_data_b, ue_plot_stddev_b, active_data_b \
        = calculate_rates(avg_window_size, ue_idxs, df_b)

    if delta:
        logger.info(f"Calculating delta (|A-B|) for {conf_name} with avg_window_size={avg_window_size}")
        all_time_points_delta, ue_plot_data_delta, ue_plot_stddev_delta, active_data_delta \
            = calculate_delta(all_time_points_a, ue_plot_data_a, ue_plot_stddev_a,
                              all_time_points_b, ue_plot_data_b, ue_plot_stddev_b)
        logger.info(f"Plotting delta (A-B) for {conf_name} with avg_window_size={avg_window_size}")
        resampling_factor = calculate_resampling_factor(len(ue_plot_data_delta[0]))
        logger.info(f"Resampling data (A-B) to {plot_resolution} points")
        ax1.plot(all_time_points_delta[0][::resampling_factor],
                 (ue_plot_data_delta[0][::resampling_factor]) / 1000.0)
        logger.info(f"Plotting standard deviation for {conf_name} with avg_window_size={avg_window_size}")

        ax1.fill_between(all_time_points_delta[0][::resampling_factor],
                         ue_plot_data_delta[0][::resampling_factor] / 1000.0 - ue_plot_stddev_delta[0][
                             ::resampling_factor] / 1000.0,
                         ue_plot_data_delta[0][::resampling_factor] / 1000.0 + ue_plot_stddev_delta[0][
                             ::resampling_factor] / 1000.0, alpha=0.2)
        ax1.set_ylabel('Difference in Total Rate (kbit/s)')
    else:
        # plot reference line
        ax1.axhline(y=target_rate / 1000, color='r', linestyle='--',
                    label=f'Target Rate')
        # plot graphs itself
        logger.info(f"Plotting rates (A) for {conf_name} with avg_window_size={avg_window_size}")
        resampling_factor = calculate_resampling_factor(len(ue_plot_data_a[0]))
        logger.info(f"Resampling data (A) to {plot_resolution} points")
        ax1.plot(all_time_points_a[0][::resampling_factor], (ue_plot_data_a[0][::resampling_factor]) / 1000.0,
                 label=label_a)
        logger.info(f"Plotting standard deviation (A) for {conf_name} with avg_window_size={avg_window_size}")

        ax1.fill_between(all_time_points_a[0][::resampling_factor],
                         ue_plot_data_a[0][::resampling_factor] / 1000.0 - ue_plot_stddev_a[0][
                             ::resampling_factor] / 1000.0,
                         ue_plot_data_a[0][::resampling_factor] / 1000.0 + ue_plot_stddev_a[0][
                             ::resampling_factor] / 1000.0, alpha=0.2)
        logger.info(f"Plotting rates (B) for {conf_name} with avg_window_size={avg_window_size}")
        resampling_factor = calculate_resampling_factor(len(ue_plot_data_b[0]))
        logger.info(f"Resampling data (B) to {plot_resolution} points")
        ax1.plot(all_time_points_b[0][::resampling_factor], ue_plot_data_b[0][::resampling_factor] / 1000.0,
                 label=label_b)

        logger.info(f"Plotting standard deviation (B) for {conf_name} with avg_window_size={avg_window_size}")
        ax1.fill_between(all_time_points_b[0][::resampling_factor],
                         ue_plot_data_b[0][::resampling_factor] / 1000.0 - ue_plot_stddev_b[0][
                             ::resampling_factor] / 1000.0,
                         ue_plot_data_b[0][::resampling_factor] / 1000.0 + ue_plot_stddev_b[0][
                             ::resampling_factor] / 1000.0, alpha=0.2)
        ax1.set_ylabel('Data Rate (kbit/s)')
        if "noarc" not in conf_name:
            ax1.set_ylim(0, (target_rate / 1000.0) + 2.0)
        else:
            ax1.set_ylim(0)
        plt.legend()

    ax1.set_xlabel('Time (s)')
    ax1.grid(True)
    plt.tight_layout()
    fig1.savefig(f"{conf_name}_rate_{avg_window_size}.png", format='png', dpi=300)
    fig1.savefig(f"{conf_name}_rate_{avg_window_size}.pdf", format='pdf')
    if show_plots:
        plt.show()
    else:
        plt.close(fig1)

def calc_tx_interval(df: pd.DataFrame, ue_idxs: np.ndarray[tuple[Any, ...]], max_int: float) \
        -> list[Any]:
    tx_intervals = []
    for r in range(repeat):
        for u in ue_idxs:
            v = df.loc[(df['vecue'] == u) & (df['vecrep'] == r)]

            # Skip if no data for this UE
            if v.empty:
                continue

            v = v.sort_index()

            prev_tx = -1.0
            for index, row in v.iterrows():
                if prev_tx == -1.0:
                    prev_tx = index
                else:
                    current_interval = index - prev_tx
                    prev_tx = index
                    if current_interval > max_int:
                        logger.warning(f"Tx interval for UE {u} is very long (outlier): {current_interval}s - assuming {max_int}")
                        current_interval = max_int
                    tx_intervals.append(current_interval)

    return tx_intervals



def plot_txinterval_hist(df_sim: pd.DataFrame, df_meas: pd.DataFrame, conf_name: str, ue_idxs: np.ndarray[tuple[Any, ...]],
                         max_interval: float = 10.0):
    tx_intervals_sim = calc_tx_interval(df_sim, ue_idxs, max_interval)
    tx_intervals_meas = calc_tx_interval(df_meas, ue_idxs, max_interval)

    # plot histogram of tx intervals: simulation
    fig1, ax1 = plt.subplots(constrained_layout=True)
    n1, bins1, patches1 = ax1.hist(tx_intervals_sim, bins=50, range=(0, max_interval))
    # ax1.set_xlim(0, 0.5)
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Count')

    # plot histogram of tx intervals: measurement
    fig2, ax2 = plt.subplots(constrained_layout=True)
    n2, bins2, patches2 = ax2.hist(tx_intervals_meas, bins=50, range=(0, max_interval))
    # ax2.set_xlim(0, 0.5)
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Count')

    # set same y-axis limits for both plots
    y_max = max(n1.max(), n2.max()) * 1.1
    ax1.set_ylim(0, y_max)
    ax2.set_ylim(0, y_max)

    fig1.savefig(f"sim_{conf_name}_txinterval_hist.png", format='png', dpi=300)
    fig1.savefig(f"sim_{conf_name}_txinterval_hist.pdf", format='pdf')
    fig2.savefig(f"meas_{conf_name}_txinterval_hist.png", format='png', dpi=300)
    fig2.savefig(f"meas_{conf_name}_txinterval_hist.pdf", format='pdf')

    # plot cumulative distribution of tx intervals
    fig3, ax3 = plt.subplots(constrained_layout=True)
    if np.max(tx_intervals_sim) < np.max(tx_intervals_meas):
        # add one datapoint so both end at the same value
        tx_intervals_sim.append(np.max(tx_intervals_meas))
    sorted_intervals = np.sort(tx_intervals_sim)
    cdf = np.arange(1, len(sorted_intervals) + 1) / len(sorted_intervals)
    plt.plot(sorted_intervals, cdf, label="Simulation")
    # Alternative:
    # n, bins, patches = ax1.hist(tx_intervals_sim, 100, density=True, histtype='step',
    #                            cumulative=True, label='Simulation')

    if np.max(tx_intervals_meas) < np.max(tx_intervals_sim):
        # add one datapoint so both end at the same value
        tx_intervals_sim.append(np.max(tx_intervals_sim))
    sorted_intervals = np.sort(tx_intervals_meas)
    cdf = np.arange(1, len(sorted_intervals) + 1) / len(sorted_intervals)
    plt.plot(sorted_intervals, cdf, label="Measurement")
    # ax1.set_xlim(0, 0.5)
    plt.legend()
    ax3.set_xlabel('Time (s)')
    ax3.set_ylabel('Probability')
    fig3.savefig(f"{conf_name}_txinterval_cdf.png", format='png', dpi=300)
    fig3.savefig(f"{conf_name}_txinterval_cdf.pdf", format='pdf')

# ###################################################################################
# start of main script
# ###################################################################################

def main():
    logging.basicConfig(level=logging.INFO, format='%(asctime)s %(message)s')
    warnings.simplefilter(action='ignore', category=pd.errors.PerformanceWarning)
    # read all simulation configurations
    for conf_meas, conf_sims in config_meas_to_sim.items():
        for app_idx in app_indexes:
            df_sims = []
            for conf in conf_sims:
                sim_dataframe = read_simulation_dataframe(app_idx, conf)
                sim_dataframe = sim_dataframe.sort_index()
                df_sims.append(sim_dataframe)

            df_meas = read_measurement_dataframe(app_idx, conf_meas)
            df_meas = df_meas.sort_index()

            configure_font_sizes()

            logger.info("Creating simulation plot: individual send rates")
            for conf in conf_sims:
                plot_rates(df_sims[conf_sims.index(conf)],
                            averaging_window_size, f"sim_{conf}_app{app_idx}",
                            np.concatenate([[-1], ue_indexes]), app_limits[app_idx])

            logger.info("Creating measurement plot: individual send rates")
            plot_rates(df_meas,
                        averaging_window_size, f"meas_{conf_sims[0]}_app{app_idx}",
                        np.concatenate([[-1], ue_indexes]), app_limits[app_idx])

            logger.info("Creating comparison plot: total send rates")
            for conf in conf_sims:
                plot_rate_comp(df_sims[conf_sims.index(conf)], df_meas,
                            averaging_window_size, f"comp_{conf}_app{app_idx}",
                            np.concatenate([[-1], ue_indexes]), app_limits[app_idx])

            logger.info("Creating plot: TX interval histogram")
            for conf in conf_sims:
                plot_txinterval_hist(df_sims[conf_sims.index(conf)], df_meas, f"{conf}_app{app_idx}",
                                     ue_indexes, 0.4)

            if len(conf_sims) > 1:
                logger.info("Creating plot: difference between WLAN and NR simulation")
                plot_rate_difference(
                    df_sims[0], df_sims[1], "Mean Rate Difference: WLAN vs. 5G NR",
                    averaging_window_size, f"delta_{conf_sims[0]}_{conf_sims[1]}", np.concatenate([[-1], ue_indexes]))

            logger.info("Done.")


if __name__ == '__main__':
    main()

