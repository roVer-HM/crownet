import subprocess
import os
import filecmp

timeout_sec = 60


def test_1():

	# run vadere only

	subprocess_cmd = ["python3", "run_script.py"]
	settings = [
		"--experiment-label",
		"test_1",
		"--vadere-only",
		"--scenario-file",
		os.path.join(os.getcwd(), "vadere/scenarios/test001.scenario")
	]
	subprocess_cmd += settings

	subprocess.check_output(
		subprocess_cmd, timeout=timeout_sec, stderr=subprocess.PIPE
	)

	output_dir = os.path.abspath("results/vadere_only_test_1/vadere.d")
	test_dir = os.path.abspath("tests/vadere_only_test_1/vadere.d")

	assert len(filecmp.dircmp(output_dir, test_dir).diff_files) == 0

def test_2():

	# run vadere + omnet (no control)

	subprocess_cmd = ["python3", "run_script.py"]
	settings = [
		"--experiment-label",
		"test_2",
		"--delete-existing-containers",
		"--create-vadere-container",
		"--config",
		"final"
	]
	subprocess_cmd += settings

	subprocess.check_output(
		subprocess_cmd, timeout=timeout_sec, stderr=subprocess.PIPE
	)

	output_dir = os.path.abspath("results/final_test_2/vadere.d")
	test_dir = os.path.abspath("tests/final_test_2/vadere.d")

	assert len(filecmp.dircmp(output_dir, test_dir).diff_files) == 0

def test_3():

	# run vadere + omnet + control

	subprocess_cmd = ["python3", "run_script.py"]
	settings = [
		"--experiment-label",
		"test_3",
		"--delete-existing-containers",
		"--create-vadere-container",
		"--config",
		"final_control",
		"--with-control",
		"control.py",
	]
	subprocess_cmd += settings

	subprocess.check_output(
		subprocess_cmd, timeout=timeout_sec, stderr=subprocess.PIPE
	)

	output_dir = os.path.abspath("results/final_control_test_3/vadere.d")
	test_dir = os.path.abspath("tests/final_control_test_3/vadere.d")

	assert len(filecmp.dircmp(output_dir, test_dir).diff_files) == 0


def test_4():

	# run vadere + control

	subprocess_cmd = ["python3", "run_script.py"]
	settings = [
		"--experiment-label",
		"test_4",
		"--delete-existing-containers",
		"--create-vadere-container",
		"--with-control",
		"control.py",
		"--control-vadere-only",
	]
	subprocess_cmd += settings

	subprocess.check_output(
		subprocess_cmd, timeout=timeout_sec, stderr=subprocess.PIPE
	)

	output_dir = os.path.abspath("results/vadere_controlled_test_4/vadere.d")
	test_dir = os.path.abspath("tests/vadere_controlled_test_4/vadere.d")

	assert len(filecmp.dircmp(output_dir, test_dir).diff_files) == 0


def test_5():

	# run vadere (use signs as control element)
	# no control necessary; signs are modeled with target changers in vadere

	subprocess_cmd = ["python3", "run_script.py"]
	settings = [
		"--experiment-label",
		"signs_test_5",
		"--vadere-only",
		"--scenario-file",
		os.path.join(os.getcwd(), "vadere/scenarios/test001_with_signs.scenario")
	]
	subprocess_cmd += settings

	subprocess.check_output(
		subprocess_cmd, timeout=timeout_sec, stderr=subprocess.PIPE
	)

	output_dir = os.path.abspath("results/vadere_only_signs_test_5/vadere.d")
	test_dir = os.path.abspath("tests/vadere_only_signs_test_5/vadere.d")

	assert len(filecmp.dircmp(output_dir, test_dir).diff_files) == 0

if __name__=="__main__":
	test_1()
	test_2()
	test_3()
	test_4()
	test_5()