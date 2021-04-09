import unittest
import os
import shutil
import glob

from suqc import Parameter
from .tutorial_simple_detour import first_examples_rover_01

class RunSimulationStudiesTests(unittest.TestCase):


	def test_simple_detour(self):

		sim_dir = glob.glob("**/tutorial_simple_detour")
		if len(sim_dir) == 1:
			sim_dir = os.path.abspath(sim_dir[0])
		else:
			raise ValueError("Simulation directory not found.")

		result_dir = os.path.join(sim_dir, "results")

		if os.path.isdir(result_dir):
			shutil.rmtree(result_dir)

		parameter = [
			Parameter(
				name="number_of_agents_mean", simulator="dummy", stages=[15, 20],
			)
		]
		first_examples_rover_01.main(parameter=parameter)


		for sample in [0,1]:
			output_sample = os.path.abspath(f"{sim_dir}/results/simulation_runs/Sample__{sample}_0/results/final_out/vadere.d")
			dir_content = os.listdir(output_sample)

			self.assertTrue("degree_informed_extract.txt" in dir_content)
			self.assertTrue("postvis.traj" in dir_content)
			self.assertTrue("DegreeInformed.txt" in dir_content)
			self.assertTrue("simple_detour_100x177_miat1.25.scenario" in dir_content)
			self.assertTrue("footsteps.csv" in dir_content)
			self.assertTrue("startEndtime.csv" in dir_content)
			self.assertTrue("InformationDegree.png" in dir_content)
			self.assertTrue("targetIds.csv" in dir_content)

		summary_output = os.path.abspath(f"{sim_dir}/results_summary")
		dir_content = os.listdir(summary_output)
		self.assertTrue("degree_informed_extract.txt" in dir_content)
		self.assertTrue("poisson_parameter.txt" in dir_content)
		self.assertTrue("parameters.csv" in dir_content)
		self.assertTrue("time_95_informed.txt" in dir_content)












