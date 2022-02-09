import unittest
import subprocess
import os
import sys

curPath = os.path.abspath(os.path.dirname(__file__))
rootPath = os.path.split(curPath)[0]
sys.path.append(rootPath)

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
		self._dir = os.path.dirname(os.path.abspath(__file__))
		self.study_1 = os.path.join(self._dir, "forward_propagation" )
		self.study_2 = os.path.join(self._dir, "forward_propagation_and_sensitvity_analysis")

	# TODO add these tests to pipeline
	# def test_forward_propagation_step_1(self):
	# 	ret = call_script(sim_dir=self.study_1,script_name="step1_generate_parameter_combinations.py")
	# 	assert(ret==0)
	#
	# def test_forward_propagation_step_2(self):
	# 	ret = call_script(sim_dir="forward_propagation",script_name="step2_run_simulations.py")
	# 	assert(ret==0)
	#
	# def test_forward_propagation_step_3(self):
	# 	ret = call_script(sim_dir=self.study_1,script_name="step3_forward_propagation.py")
	# 	assert(ret==0)
	#
	#
	# def test_sensitivity_analysis_step1(self):
	# 	ret = call_script(sim_dir=self.study_2,script_name="step1_generate_parameter_combinations.py")
	# 	assert(ret==0)
	#
	#
	# def test_sensitivity_analysis_step3(self):
	# 	ret = call_script(sim_dir=self.study_2,script_name="step3_sensitivity_analysis.py")
	# 	assert(ret==0)
