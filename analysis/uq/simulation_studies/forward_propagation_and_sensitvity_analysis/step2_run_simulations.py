#!/usr/bin/env python3
#!/usr/bin/python3

import os, sys
sys.path.append(os.path.abspath(""))


import time
from datetime import timedelta
sys.path.append(os.path.abspath(""))
import shutil

from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.request import CoupledDictVariation
import chaospy, numpy
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager

import matplotlib.pyplot as plt
import pandas as pd

run_local = True

mnt = os.environ["OPP_EXTERN_DATA_MNT"].split(":")[0]
assert os.path.isdir(mnt), "Please specify OPP_EXTERN_DATA_MNT. See CROWNET_HOME/config"
simulation_dir = os.path.join(mnt, "direct_communication")


def run_simulations(par_var, summary_dir, quantity_of_interest):

    start_time = time.time()
    print(f"Simulation started at {start_time}")

    path2ini = os.path.join(
        os.environ["CROWNET_HOME"],
        "crownet/simulations/simple_detoure_suqc_traffic/omnetpp.ini",
    )

    model = VadereOppCommand() \
        .create_vadere_container() \
        .experiment_label("output") \
        .vadere_tag("e3ac6411") \
        .omnet_tag("6.0.1") \
        .qoi(quantity_of_interest)

    if os.path.isdir(simulation_dir):
        print(f"Remove {simulation_dir}")
        shutil.rmtree(simulation_dir, ignore_errors=True)

    setup = CoupledDictVariation(
        ini_path=path2ini,
        config="final",
        parameter_dict_list=par_var,
        qoi=quantity_of_interest,
        model=model,
        post_changes=None,
        output_path=os.path.dirname(simulation_dir),
        output_folder=os.path.basename(simulation_dir),
        remove_output=False,
    )

    par, data = setup.run(6)

    print(f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss).")

    os.makedirs(summary_dir)

    par.to_csv(os.path.join(summary_dir, f"parameters.txt"))

    for qoi_, vals_ in data.items():
        file_path = os.path.join(summary_dir, qoi_)
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

def analyze_scalar_qoi(expansion, abscissas,weights, sim_results, qoi, output_dir):



    approx = chaospy.fit_quadrature(expansion, abscissas, weights, sim_results)

    e1 = chaospy.E(approx, distribution).round(DIGIT)
    std1 = chaospy.Std(approx, distribution).round(DIGIT)
    skew1 = chaospy.Skew(approx, distribution).round(DIGIT)

    first_order_sobol = chaospy.Sens_m(approx, distribution).round(DIGIT)
    second_order_sobol = chaospy.Sens_m2(approx, distribution).round(DIGIT)

    output_file_name = os.path.join(output_dir, f"SA_and_FW_{qoi}.txt")

    with open(output_file_name, "w") as f:
        f.write("Parameters: Number of peds, message length, power \n")
        f.write(f"Quantity of interest: {qoi} \n")
        f.write(f"Qoi (stats):  E = {e1}, std = {std1}, skew = {skew1}\n\n")
        f.write(f"Sensitivity indices \n")
        f.write(f"First order \n")
        f.write(numpy.array2string(first_order_sobol, precision=DIGIT))
        f.write(f"\n Second order \n")
        f.write(numpy.array2string(second_order_sobol, precision=DIGIT))

    qoi_dist = chaospy.QoI_Dist(approx, distribution, 10000)
    # e = chaospy.E(qoi_dist)  # should be similar to e1
    # std = chaospy.Std(qoi_dist)  # should be similar to e2
    plt.hist(qoi_dist.sample(5000))
    plt.xlabel(qoi)
    plt.ylabel("Counts")
    plt.savefig(os.path.join(output_dir, f"{qoi}.pdf"))
    plt.show(block=False)
    plt.close()

    print(f"{qoi}: sensitivity analysis and forward propagation results exported to {output_file_name} ")

if __name__ == "__main__":

    DIGIT = 3
    summary_dir = os.path.join(simulation_dir, "results_summary")
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "PolynomialChaos_results")

    # paratemeters
    low = 10
    up = 2000
    b = 4
    p1_ = chaospy.TruncExponential(upper=up, shift=low, scale=(up - low) / b)
    p2_ = chaospy.Uniform(lower=0, upper=4000) #"*.hostMobile[*].app[1].messageLength"
    p3_ = chaospy.Uniform(lower=0.5,upper=2.0) # "**wlan[*].radio.transmitter.power"

    distribution = chaospy.J(p1_,p2_,p3_)

    order = 2
    abscissas, weights = chaospy.generate_quadrature( order, distribution, rule="gaussian")
    expansion = chaospy.orth_ttr(order, distribution)

    samples = abscissas.T

    par_var = list()
    for val1, val2, val3 in samples:

        # Parameter 1: number of agents
        # The number of agents is a parameter that is specified in the vadere simulator.
        # There are four sources in the simulation that spawn agents according to Poisson processes.
        # The unit of the Poisson parameter p is [agents/1 seconds].
        # Since the 'number of agents' parameter refers to 100s and there are four sources, p must be:
        p = round(val1 * 0.01 / 4, 4) # source parameters must be of type >list< in vadere, other parameters require other types

        # Parameter 2: message length
        # The message length is a parameter in the omnet simulator.
        # It must be an integer value with unit B.
        message_length_parameter_val = f"{int(val2)}B" # note: parameter values must be always strings in omnet

        power = f"{round(val3,4)}mW"

        sample = {
            'omnet': {"*.misc[0].app[0].messageLength": message_length_parameter_val,
                      "**wlan[*].radio.transmitter.power": power},
            'vadere': {'sources.[id==1].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==2].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==5].spawner.distribution.numberPedsPerSecond': p,
                    'sources.[id==6].spawner.distribution.numberPedsPerSecond': p}
            }

        par_var.append(sample)

    par_var = OmnetSeedManager(par_variations=par_var, rep_count=1, vadere_fixed=True, omnet_fixed=True).get_new_seed_variation()

    quantity_of_interest = [
        "degree_informed_extract.txt",
        "poisson_parameter.txt",
        "time_95_informed.txt",
        "packet_age.txt",
        "number_of_peds.txt"
    ]

    # par_var = par_var[:2] just for test purpose
    run_simulations(par_var,summary_dir, quantity_of_interest)

    # apply forward prop and sensitivity analysis
    if os.path.isdir(output_dir):
        print(f"Remove {output_dir}")
        shutil.rmtree(output_dir, ignore_errors=True)
    os.makedirs(output_dir)

    df = pd.read_csv(os.path.join(summary_dir,"time_95_informed.txt"))
    time95 = df["timeToInform95PercentAgents"].values
    analyze_scalar_qoi(expansion, abscissas, weights, time95, "Time95PercentInformed", output_dir)

    df = pd.read_csv(os.path.join(summary_dir, "packet_age.txt"))
    analyze_scalar_qoi(expansion, abscissas, weights, df["50%"].values, "MedianLifetime", output_dir)
    analyze_scalar_qoi(expansion, abscissas, weights, df["max"].values, "MaxLifetime", output_dir)
    analyze_scalar_qoi(expansion, abscissas, weights, df["min"].values, "MinLifetime", output_dir)
    analyze_scalar_qoi(expansion, abscissas, weights, df["mean"].values, "MinLifetime", output_dir)

    df = pd.read_csv(os.path.join(summary_dir, "number_of_peds.txt"))
    analyze_scalar_qoi(expansion, abscissas, weights, df["mean"].values, "NumberOfPeds", output_dir)

    plt.hist(p1_.sample(1000))
    plt.xlabel("Parameter: number of agents")
    plt.yscale("Counts")
    plt.savefig(os.path.join(output_dir, "ParameterNumberOfAgents.pdf"))
    plt.show(block=False)
    plt.close()

    sys.exit(0)







