import os
import subprocess
import sys
import unittest

curPath = os.path.abspath(os.path.dirname(__file__))
rootPath = os.path.split(curPath)[0]
sys.path.append(rootPath)
import shutil
import time
from datetime import timedelta


def call_script(sim_dir, script_name):
    terminal_command = ['python3', script_name]

    if os.path.isdir(sim_dir) == False:
        raise EnvironmentError(f"Simulation dir {sim_dir} not found.")

    print(f"Run {terminal_command} in {sim_dir}")

    return_code = subprocess.check_call(
        terminal_command,
        cwd=sim_dir,
        timeout=12000,  # stop simulation after 20min
    )
    return return_code


class SimulationStudiesTests(unittest.TestCase):

    def setUp(self) -> None:
        print(f"Start intergation tests at {time.ctime()}")
        self._start_time = time.time()

        self.number_of_parralel_sims = 4  # change this to 3..10 (depending on your system. in the CI, we choose 1)
        self._dir = os.path.dirname(os.path.abspath(__file__))
        self._sim_dir = os.path.join(self._dir, "unittest_sim_output")

        if os.path.isdir(self._sim_dir):
            print(f"Remove {self._sim_dir}")
            shutil.rmtree(self._sim_dir, ignore_errors=True)

        os.makedirs(self._sim_dir)
        print(f"Create directory for sim output: {self._sim_dir}")

    def tearDown(self) -> None:
        print(f"Finised intergation tests at {time.ctime()}")
        print(
            f"Time to run all tests (jobs= {self.number_of_parralel_sims}: {timedelta(seconds=time.time() - self._start_time)} (hh:mm:ss).")

    def test_all_sim_studies(self):
        # !!do not parallize this: container names would be duplicate, ressources not enough!
        self.guiding_crowds_forward_propagation()
        self.forward_propagation()
        self.forward_propagation_and_sensitvity_analysis()

    def guiding_crowds_forward_propagation(self):

        start_time = time.time()
        sim_name = "guiding_crowds_forward_propagation"
        sim_dir = os.path.join(self._dir, sim_name)
        if os.path.isdir(sim_dir) == False:
            raise EnvironmentError(f"Simulation dir {sim_dir} not found.")

        output_dir = os.path.join(self._sim_dir, sim_name)

        terminal_command = ['python3',
                            "run_simulations.py",
                            output_dir,
                            '3',  # number of samples
                            '1',  # spawnnumber
                            str(self.number_of_parralel_sims)  # number of parallel processes
                            ]
        print(f"Run {terminal_command} in {sim_dir}")

        return_code = subprocess.check_call(
            terminal_command,
            cwd=sim_dir,
            timeout=7200,  # stop simulation after 2h
        )

        print(
            f"Time to run {sim_name} (jobs= {self.number_of_parralel_sims}: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

        assert (return_code == 0)

    def forward_propagation(self):

        sim_name = "forward_propagation"
        start_time = time.time()

        sim_dir = os.path.join(self._dir, sim_name)
        if os.path.isdir(sim_dir) == False:
            raise EnvironmentError(f"Simulation dir {sim_dir} not found.")

        output_dir = os.path.join(self._sim_dir, sim_name)

        terminal_command = ['python3',
                            "run_simple_detour_shadowing.py",
                            output_dir,
                            '120',  # max. number of peds
                            '20',  # number of samples
                            str(self.number_of_parralel_sims)  # number of parallel processes
                            ]
        print(f"Run {terminal_command} in {sim_dir}")

        return_code = subprocess.check_call(
            terminal_command,
            cwd=sim_dir,
            timeout=7200,  # stop simulation after 2h
        )

        print(
            f"Time to run {sim_name} (jobs= {self.number_of_parralel_sims}: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

        assert (return_code == 0)

    def forward_propagation_and_sensitvity_analysis(self):

        start_time = time.time()
        sim_name = "forward_propagation_and_sensitvity_analysis"

        sim_dir = os.path.join(self._dir, sim_name)
        if os.path.isdir(sim_dir) == False:
            raise EnvironmentError(f"Simulation dir {sim_dir} not found.")

        output_dir = os.path.join(self._sim_dir, sim_name)

        terminal_command = ['python3',
                            "run_simple_detour_sa_fp.py",
                            output_dir,
                            '1',
                            # controls the number of samples: 1 -> 8 samples for 3 para, 2 -> 27 samples for 3 para, ..
                            '50',  # 50 peds min (controls the lower limit)
                            '80',  # 130 peds max (controls the upper limit)
                            str(self.number_of_parralel_sims)  # number of parallel processes
                            ]
        print(f"Run {terminal_command} in {sim_dir}")

        return_code = subprocess.check_call(
            terminal_command,
            cwd=sim_dir,
            timeout=7200,  # stop simulation after 2h
        )

        print(
            f"Time to run {sim_name} (jobs= {self.number_of_parralel_sims}: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

        assert (return_code == 0)


if __name__ == "__main__":
    unittest.main()
