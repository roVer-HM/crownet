
import os, sys, subprocess

# use in dubg mode => otherwise it does not work in pycharm

## start TRACI first
# os.system("TRACI_GUI=false vadere")


## make sure that ROVER_MAIN is accessable for python => /etc/environment !
if print(os.getenv('ROVER_MAIN')) is None:
    #print(os.environ)
    print("Please add variable ROVER_MAIN to your e.g. /etc/environment")

    print("Alternative: add ROVER_MAIN with python")
    os.environ['ROVER_MAIN'] = '/home/christina/repos/rover-main'

print(os.getenv('ROVER_MAIN'))

## run simulation
os.chdir(os.path.join(os.getcwd(),"id_0001"))
print(os.getcwd())
subprocess.Popen(["./startsim.sh"], env=os.environ)

print("finished.")
