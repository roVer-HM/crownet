#!/usr/bin/env python3
#!/usr/bin/python3

from uq.PreProcessing.sampling.MonteCarlo import SampleBaseHalton
from uq.PreProcessing.Parameter import UniformParameter

run_local = True
import seaborn as sns
import matplotlib.pyplot as plt


if __name__ == "__main__":
    # Step 1

    # In this tutorial, we assess how the two uncertain parameters
    # 1) number of agents
    # 2) message length
    # affect the information dissemination time the simple_detour simulation example
    # Method: forward uq, pseudo Monte Carlo approach

    # We consider two uncertain parameters
    p1 = UniformParameter(lower_bound=20, upper_bound=100, name="Number of agents [1]") # number of agents spawned/min: 20...100 agents
    p2 = UniformParameter(lower_bound=100, upper_bound=2000, name="Message length [B]") # message length: 100B ...2000B

    # We use a pseudo Monte Carlo approach (Halton sequence) to generate 100 parameter combinations (samples)
    file_path = "pMCHalton.json"
    sample_size = 100
    sampling = SampleBaseHalton(sample_size=sample_size, parameters=[p1, p2])
    sampling.write(file_path)

    # Plot joint distribution of uncertain parameters
    samples = sampling.get_samples()
    #h = sns.jointplot(x=samples[:, 0], y=samples[:, 1])
    #h.set_axis_labels(p1.get_name(), p2.get_name() , fontsize=12)
    #plt.show()