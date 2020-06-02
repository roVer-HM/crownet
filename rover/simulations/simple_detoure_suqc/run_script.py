#!/usr/bin/python3
# import roveranalzer
# import logging # <-- runSim.py erzeuge logfile runSim.log
import os
import subprocess
import sys

import matplotlib.pyplot as plt
import pandas as pd
import time
import glob
import argparse
import shutil

class SimulationRun:

    def __init__(self, qoi, remove = False, config = "final" ):

        self.qoi = qoi
        self._set_old_files()
        self.remove = remove
        self.config = config

    def get_config(self):
        return self.config

    def _set_old_files(self):
        filepath = f"{os.getcwd()}/results/**/*.scenario"
        old_files = list()
        for file in glob.glob(filepath, recursive=True):
            old_files.append(os.path.dirname(file))
        self.old_files = old_files

    def _get_old_files(self):
        return self.old_files

    def run_sim_old(self):
        os.system("chmod +x runScript.sh")
        return_code = subprocess.check_call(
            ["./runScript.sh"],
            env=os.environ,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )

    def run_sim(self):
        # !/bin/bash

        if os.getenv("ROVER_MAIN") is None:
            print("Variable ROVER_MAIN not set.")
            sys.exit(-1)


        SIM_DIR = os.getcwd()
        OPP_INI = "omnetpp.ini"

        CONFIG = self.get_config()
        EXPERIMENT = "out"
        RESULT_DIR = os.path.join(SIM_DIR,"results")

        if not os.path.exists(RESULT_DIR):
            os.mkdir(RESULT_DIR)

        rv = os.environ["ROVER_MAIN"]
        os.chdir(f"{rv}/scripts")
        subprocess.check_call("./nedpath")

        os.chdir(SIM_DIR)



        CMD_ARR = f'exec opp_runall -j 1 opp_run'
        CMD_ARR += f' -u Cmdenv'
        CMD_ARR += f' -c "{CONFIG}"'
        CMD_ARR += f' --experiment-label={EXPERIMENT}"'
        CMD_ARR += f' --result-dir={RESULT_DIR}"'
        CMD_ARR += f' -l "{rv}/inet4/src/INET"'
        CMD_ARR += f' -l "{rv}/rover/src/ROVER"'
        CMD_ARR += f' -l "{rv}/simulte/src/lte"'
        CMD_ARR += f' -l "{rv}/veins/src/veins"'
        CMD_ARR += f' -l "{rv}/veins/subprojects/veins_inet/src/veins_inet"'
        CMD_ARR += f' "{OPP_INI}"'

        os.chdir(f"{rv}/scripts")
        print(os.listdir(os.getcwd()))
        command = f'./omnetpp_rnd "{CMD_ARR}"'


        subprocess.check_call(command)


        print("fsdvsd")










    def _get_current_filepath(self, qoi):
        filepath = f"{os.getcwd()}/results/**/{qoi}"
        filepaths = glob.glob(filepath, recursive=True)
        old_files = self._get_old_files()
        old_files = [os.path.join(path,qoi) for path in old_files]
        new_path = [i for i in filepaths if i not in old_files]
        return new_path


    def wait_for_qoi(self,filename):

        file = self._get_current_filepath(filename)

        secs = 0
        while (len(file) < 1) and (secs <= 60):
            file = self._get_current_filepath(filename)
            time.sleep(1)
            secs += 1

        if (len(file) < 1):
            file = None  # prevent from infinite loop
        else:
            file = file[0]

        return file


    def get_DegreeInformed_Dataframe(self,filepath):
        df_r = pd.read_csv(filepath, delimiter=" ", header=[0], comment="#")
        dt = 0.4  # one time frame = 0.4s
        df_r = df_r.iloc[249:, :]
        df_r["timeStep"] = (
                dt * df_r["timeStep"] - 100.0
        )  # information dissemination starts at 100s
        df_r.index = df_r.index - 249
        return df_r


    def DegreeInformed_extract(self):
        filename = "DegreeInformed.txt"
        # wait for file
        filepath = self.wait_for_qoi(filename)
        # replace it with file extraction
        df_r = self.get_DegreeInformed_Dataframe(filepath)
        df_r.to_csv(os.path.join(os.path.dirname(filepath), "DegreeInformed_extract.txt"), sep=" ")

        # plot
        plt.plot(df_r.iloc[:, 0], df_r.iloc[:, 3])
        plt.axhline(y=0.95, c="r")
        plt.xlabel("Time [s] (Time = 0s : start of information dissemination)")
        plt.ylabel("Percentage of pedestrians informed [%]")
        plt.title("Information degree")
        plt.savefig(os.path.join(os.path.dirname(filepath), "InformationDegree.png"))


    def Time95Informed(self):
        filename = "DegreeInformed.txt"
        # wait for file
        filepath = self.wait_for_qoi(filename)
        # replace it with file extraction
        df_r = self.get_DegreeInformed_Dataframe(filepath)

        dt = 0.4
        time95 = 0.0
        for perc in df_r.iloc[:, 3]:
            if perc >= 0.95:
                break
            time95 += dt

        f = open(os.path.join(os.path.dirname(filepath), "Time95Informed.txt"), "x")
        f.write(f" timeToInform95PercentAgents\n0 {time95}")
        f.close()


    def post_processing(self):
        for q in self.qoi:
           eval("self."+q+"()")

        if self.remove is True:
            self.clean_dir()

    def pre_processing(self):
        pass

    def clean_dir(self):

        shutil.rmtree("vadere")
        shutil.rmtree("sumo")

        fileList = []
        for root, d_names, f_names in os.walk(os.getcwd()):
            for f in f_names:
                fileList.append(os.path.join(root, f))

        for filePath in fileList:
            fileName = os.path.basename(filePath)
            if os.path.splitext(fileName)[0] not in self.qoi:
                os.remove(filePath)






if __name__ == "__main__":

    sim_storage = "/home/christina/repos/rover-main/rover/simulations/simple_detoure_suqc"

    # parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--qoi", action="append",nargs='+', help="specify qoi files", type=str)
    parser.add_argument('--remove', dest='keep', action='store_true')
    parser.add_argument('--keep', dest='keep', action='store_false')
    args = parser.parse_args()

    qoi = ["DegreeInformed_extract.txt","Time95Informed.txt"]
    if args.qoi:
        qoi = list(args.qoi[0])
    qoi = [os.path.splitext(q)[0] for q in qoi]

    remove = False
    if args.keep:
        remove = args.keep
    if os.getcwd() == sim_storage:
        print("Cannot delete original definition of simulation.")
        remove = False

    # start simulation
    simulation_run = SimulationRun(qoi=qoi, remove = remove)
    os.environ["ROVER_MAIN"] = "/home/christina/repos/rover-main"

    simulation_run.run_sim()

    try:
        simulation_run.pre_processing()
        simulation_run.run_sim()
        simulation_run.post_processing()
    except:
        sys.exit(-1)
    sys.exit(0)  # ok
