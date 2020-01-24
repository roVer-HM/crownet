
# use in dubg mode => otherwise it does not work in pycharm

import os, subprocess, suqc
# install the suqc package from suq-controller => README

setup = suqc.read_csvs_afterwards("/home/christina/repos/suq-controller/tutorial/testfolder")


## start TRACI first
# os.system("TRACI_GUI=false vadere")

## make sure that ROVER_MAIN is accessable for python => /etc/environment !
if print(os.getenv('ROVER_MAIN')) is None:
    #print(os.environ)
    print("Please add variable ROVER_MAIN to your e.g. /etc/environment")

    print("Alternative: add ROVER_MAIN with python")
    os.environ['ROVER_MAIN'] = '/home/christina/repos/rover-main'

print(os.getenv('ROVER_MAIN'))

## Input



## run simulation

for x in [1,2]:
    run_id = "id_000" + str(x)
    os.chdir(os.path.join(os.getcwd(),run_id))
    os.system('echo "started -----------------------"')
    process = subprocess.Popen(["./startsim.sh"], env=os.environ)
    process.wait()
    os.chdir('..')
    os.system('echo "finshed -----------------------"')


print("finished.")
