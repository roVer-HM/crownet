
import os, shutil,subprocess
from distutils.dir_util import copy_tree
from VariableExtraction import create_VariableExtraction


def overwrite_basis_input( FileName, basis_vector, samples_array,x ):

    for kk in range(0,len(basis_vector)):
        print(kk)

        if kk is 0:
            replace_vector = (basis_vector[0], samples_array[x][0])
        else:
            replace_vector = (replace_vector, (basis_vector[kk], samples_array[x][kk]))


    with open(FileName) as f:
        newText = f.read()

        for r in replace_vector:
            newText= newText.replace(*r)

    with open(FileName, "w") as f:
        f.write(newText)

def getVariables():
    print("started")

def sampling():
    basis_omnet = ['**.channelModel.ue_height = 1.5', '**.channelModel.building_height = 20']
    basis_vadere = ['"speedDistributionMean" : 1.34', '"speedDistributionStandardDeviation" : 0.26']

    samples_omnet = [['**.channelModel.ue_height = 1.7', '**.channelModel.building_height = 22'],
                     ['**.channelModel.ue_height = 1.4', '**.channelModel.building_height = 18']]
    samples_vadere = [['"speedDistributionMean" : 1.35', '"speedDistributionStandardDeviation" : 0.32'],
                      ['"speedDistributionMean" : 1.25', '"speedDistributionStandardDeviation" : 0.25']]
    return basis_omnet,basis_vadere,samples_omnet,samples_vadere


def main():
    ## 1. Sampling
    # use python lib for sampling
    basis_omnet, basis_vadere, samples_omnet, samples_vadere = sampling()

    # save samples, each folder contains one sample with x repetitions
    folderName = "simulation_000"

    for x in [0, 1]:
        run_id = folderName + str(x)
        shutil.rmtree(run_id, ignore_errors=True)
        os.mkdir(run_id)
        os.mkdir(run_id + "/input")
        copy_tree("basis_input_files", run_id + "/input")

        # omnet
        FileName = run_id + "/input/omnetpp.ini"
        overwrite_basis_input(FileName, basis_omnet, samples_omnet,x)

        FileName = run_id + "/input/vadere.scenario"
        overwrite_basis_input(FileName, basis_vadere, samples_vadere,x)
        shutil.copyfile("startsim.sh", run_id + "/startsim.sh")

    ## 2. Run simulations
    # 2.1. check system variables

    if print(os.getenv('CROWNET_HOME')) is None:
        # print(os.environ)
        print("Please add variable CROWNET_HOME to your e.g. /etc/environment")

        print("Alternative: add CROWNET_HOME with python")
        os.environ['CROWNET_HOME'] = '/home/christina/repos/crownet'

    print(os.getenv('CROWNET_HOME'))

    # 2.2. start simulations, each run produces an output folder in the sample folder

    for x in [0, 1]:
        run_id = folderName + str(x)
        os.chdir(os.path.join(os.getcwd(), run_id))
        os.system('echo "started -----------------------"')
        os.system("chmod +x startsim.sh")
        process = subprocess.Popen(["./startsim.sh"], env=os.environ)
        process.wait()
        os.chdir('..')
        os.system('echo "finshed -----------------------"')

    ## 3. read data from output folders

    print("finished")


if __name__ == "__main__":
    create_VariableExtraction("omnet")
    create_VariableExtraction("vadere")
    #main()
