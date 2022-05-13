#!/usr/bin/env python3
#!/usr/bin/python3

from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from uq.PreProcessing.Parameter import UniformParameter

run_local = True



if __name__ == "__main__":

    # Step 1

    # In this tutorial, we assess how the two uncertain parameters
    # 1) number of agents
    # 2) message length
    # affect the information dissemination time the simple_detour simulation example
    # Method: forward uq, pseudo Monte Carlo approach

    # We consider two uncertain parameters
    p1 = UniformParameter(lower_bound=20, upper_bound=100) # number of agents spawned/min: 20...100 agents
    p2 = UniformParameter(lower_bound=100, upper_bound=2000)
    p3 = UniformParameter(lower_bound=10, upper_bound=33)# message length: 100B ...2000B

    # We use a pseudo Monte Carlo approach (Halton sequence) to generate 100 parameter combinations (samples)
    file_path = "saltellisExtensionSamples.json"
    sample_size = 256
    sampling = SampleBaseSaltelli(parameters=[p1, p2, p3], sample_size= sample_size)
    sampling.write(file_path)
