#!/usr/bin/env python3
#!/usr/bin/python3

import os, sys

sys.path.append(os.path.abspath(""))

import time
from datetime import timedelta

sys.path.append(os.path.abspath(""))
import shutil
import docker
import string, random

from suqc.CommandBuilder.VadereOppCommand import VadereOppCommand
from suqc.request import CoupledDictVariation
from suqc.utils.SeedManager.OmnetSeedManager import OmnetSeedManager
import chaospy, numpy
import pandas as pd
import matplotlib.pyplot as plt


def run_simulations(
    par_var, summary_dir, quantity_of_interest, simulation_dir, para_processes=1
):

    start_time = time.time()
    print(f"Simulation started at {time.ctime()}")

    path2ini = os.path.join(
        os.environ["CROWNET_HOME"],
        "crownet/simulations/simple_detoure_suqc_traffic/omnetpp.ini",
    )

    network_id = f"sensitivity_analysis_{''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))}"

    client = docker.from_env(timeout=120, max_pool_size=10)
    try:
        client.networks.get(network_id=network_id).remove()
    except:
        pass
    client.networks.create(network_id)

    vadere_tag = os.getenv("CROWNET_VADERE_CONT_TAG")
    omnet_tag = os.getenv("CROWNET_OPP_CONT_TAG")

    model = (
        VadereOppCommand()
        .create_vadere_container()
        .experiment_label("output")
        .vadere_tag(vadere_tag)
        .omnet_tag(omnet_tag)
        .qoi(quantity_of_interest)
        .write_container_log()
        .network_name(network_id)
    )

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

    par, data = setup.run(para_processes)

    print(
        f"Time to run all simulations: {timedelta(seconds=time.time() - start_time)} (hh:mm:ss)."
    )

    os.makedirs(summary_dir)

    par.to_csv(os.path.join(summary_dir, f"parameters.txt"))

    for qoi_, vals_ in data.items():
        file_path = os.path.join(summary_dir, qoi_)
        print(f"Export result {qoi_} to {file_path}.")
        vals_.to_csv(file_path)

    try:
        client.networks.get(network_id=network_id).remove()
    except Exception as error:
        print(f"Warning: Could not remove newly created docker network {network_id}.")
        print(error)

    return par, data


def analyze_scalar_qoi(expansion, abscissas, weights, sim_results, qoi, output_dir):

    print(
        f"Start sensitivity analysis and qoi of qoi = {qoi} \n Qoi values: {sim_results}"
    )

    if numpy.all(numpy.isfinite(sim_results)) == False:
        print("There are non numerical values in the output. Skip qoi.")
        return

    approx = chaospy.fit_quadrature(expansion, abscissas, weights, sim_results)

    e1 = chaospy.E(approx, distribution).round(DIGIT)
    std1 = chaospy.Std(approx, distribution).round(DIGIT)
    skew1 = chaospy.Skew(approx, distribution).round(DIGIT)

    first_order_sobol = chaospy.Sens_m(approx, distribution).round(DIGIT)
    second_order_sobol = chaospy.Sens_m2(approx, distribution).round(DIGIT)
    total_order_sobol = chaospy.Sens_t(approx, distribution).round(DIGIT)

    output_file_name = os.path.join(output_dir, f"SA_and_FW_{qoi}.txt")

    is_constant = std1 < 0.001

    with open(output_file_name, "w") as f:
        f.write("Parameters: Number of peds, repeat Interval, power \n")
        f.write(f"Quantity of interest: {qoi} \n")
        f.write(f"Qoi (stats):  E = {e1}, std = {std1}, skew = {skew1}\n\n")

        if is_constant:
            f.write(
                "Warning. The sensitivity indices are rounding errors, since the std is appro. = 0.\n"
                "DO NOT USE THEM \n"
            )

        f.write(f"Sensitivity indices \n")
        f.write(f"First order \n")
        f.write(numpy.array2string(first_order_sobol, precision=DIGIT))
        f.write(f"\n Second order \n")
        f.write(numpy.array2string(second_order_sobol, precision=DIGIT))
        f.write(f"\n Total order \n")
        f.write(numpy.array2string(total_order_sobol, precision=DIGIT))

    if is_constant == False:
        qoi_dist = chaospy.QoI_Dist(approx, distribution, 1000)
        plt.hist(qoi_dist.sample(500))
        plt.xlabel(qoi)
        plt.ylabel("Counts")
        plt.savefig(os.path.join(output_dir, f"{qoi}.pdf"))
        plt.show(block=False)
        plt.close()

    print(
        f"{qoi}: sensitivity analysis and forward propagation results exported to {output_file_name} "
    )


if __name__ == "__main__":

    if len(sys.argv) == 1:
        # default behavior of script
        file_path = os.path.dirname(os.path.abspath(__file__))
        simulation_dir = os.path.join(file_path, "direct_communication")
        order = 2
        low = 100
        up = 2000
        para_process = 6
    elif len(sys.argv) == 6:
        # use arguments from command line
        simulation_dir = sys.argv[1]
        order = int(sys.argv[2])
        low = float(sys.argv[3])
        up = float(sys.argv[4])
        para_process = int(sys.argv[5])
    else:
        raise ValueError(
            "Call the script >python3 script_name.py abspathoutputdir 2 100 2000 1< "
            "where abspathoutputdir is the path to the output directory"
            "where 2 is the order for the polynomial chaos expansion."
            "where 100 is the lower limit of the number of peds parameter."
            "where 2000 is the upper limit of the number of peds parameter."
            "where 1 is the number of parallel processes."
        )

    print(
        f"Write output to {simulation_dir}. \n"
        f"Use following values: \n"
        f"polynomial chaos expansion: order =  {order}, number of peds (max): {up}. Number of parallel runs: {para_process}"
    )

    DIGIT = 3
    summary_dir = os.path.join(simulation_dir, "results_summary")
    output_dir = os.path.join(simulation_dir, "PolynomialChaos_results")

    # paratemeters
    # p1_ = chaospy.TruncExponential(upper=up, shift=low, scale=(up - low) / b) b=4
    p1_ = chaospy.Uniform(lower=low, upper=up)  # number of pedestrians
    p2_ = chaospy.Uniform(
        lower=0.8, upper=2.8
    )  # *.misc[0].app[0].repeateInterval  "*.hostMobile[*].app[1].messageLength"
    p3_ = chaospy.Uniform(lower=2.0, upper=20.0)  # "**wlan[*].radio.transmitter.power"

    distribution = chaospy.J(p1_, p2_, p3_)

    abscissas, weights = chaospy.generate_quadrature(
        order, distribution, rule="gaussian"
    )
    expansion = chaospy.orth_ttr(
        order, distribution
    )  # TODO replace with orth_ttr with chaospy.expansion.stieltjes

    samples = abscissas.T

    par_var = list()
    for val1, val2, val3 in samples:

        # Parameter 1: number of agents
        # The number of agents is a parameter that is specified in the vadere simulator.
        p = round(
            4 * 1 * 100 / val1, 4
        )  # = 4 sources* 1 Person*100s / (number of persons in scenario after 100s)

        # Parameter 2: message length
        # The message length is a parameter in the omnet simulator.
        # It must be an integer value with unit B.
        repeateInterval = f"{round(val2,4)}s"  # note: parameter values must be always strings in omnet

        power = f"{round(val3,4)}mW"

        sample = {
            "omnet": {
                "*.misc[0].app[0].repeateInterval": repeateInterval,
                "**wlan[*].radio.transmitter.power": power,
            },
            "vadere": {
                "sources.[id==1].spawner.distribution.updateFrequency": p,
                "sources.[id==2].spawner.distribution.updateFrequency": p,
                "sources.[id==5].spawner.distribution.updateFrequency": p,
                "sources.[id==6].spawner.distribution.updateFrequency": p,
            },
        }

        par_var.append(sample)

    par_var = OmnetSeedManager(
        par_variations=par_var, rep_count=1, vadere_fixed=True, omnet_fixed=True
    ).get_new_seed_variation()

    quantity_of_interest = [
        "degree_informed_extract.txt",
        "poisson_parameter.txt",
        "time_95_informed_redirection_area.txt",
        "time_95_informed_all.txt",
        "packet_age.txt",
        "number_of_peds.txt",
    ]

    info, qoi = run_simulations(
        par_var, summary_dir, quantity_of_interest, simulation_dir, para_process
    )

    # apply forward prop and sensitivity analysis
    if os.path.isdir(output_dir):
        print(f"Remove {output_dir}")
        shutil.rmtree(output_dir, ignore_errors=True)
    os.makedirs(output_dir)

    df1 = qoi["time_95_informed_redirection_area.txt"]
    df2 = qoi["time_95_informed_all.txt"]
    df3 = qoi["packet_age.txt"]
    df4 = qoi["number_of_peds.txt"]

    analyze_scalar_qoi(
        expansion,
        abscissas,
        weights,
        df1["timeToInform95PercentAgents"].values,
        "Time95PercentInformedRedirecationArea",
        output_dir,
    )
    analyze_scalar_qoi(
        expansion,
        abscissas,
        weights,
        df2["timeToInform95PercentAgents"].values,
        "Time95PercentInformedAll",
        output_dir,
    )
    analyze_scalar_qoi(
        expansion, abscissas, weights, df3["50%"].values, "MedianLifetime", output_dir
    )
    analyze_scalar_qoi(
        expansion, abscissas, weights, df3["max"].values, "MaxLifetime", output_dir
    )
    analyze_scalar_qoi(
        expansion, abscissas, weights, df3["min"].values, "MinLifetime", output_dir
    )
    analyze_scalar_qoi(
        expansion, abscissas, weights, df3["mean"].values, "MinLifetime", output_dir
    )
    analyze_scalar_qoi(
        expansion, abscissas, weights, df4["mean"].values, "NumberOfPeds", output_dir
    )

    plt.hist(p1_.sample(1000))
    plt.xlabel("Parameter: number of agents")
    plt.ylabel("Counts")
    plt.savefig(os.path.join(output_dir, "ParameterNumberOfAgents.pdf"))
    plt.show(block=False)
    plt.close()

    print(info.columns)
    if (info[("MetaInfo", "return_code")] == 0).all():
        sys.exit(0)
    else:
        print("Some of the simulations failed.")
        sys.exit(-1)
