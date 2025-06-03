import matplotlib.pyplot as plt
import numpy as np
from SALib.sample import saltelli
from SALib.sample import sobol
from scipy.stats import qmc, chisquare  # quasi-Monte Carlo for latin hypercube sampling

from uq.PreProcessing.Parameter import *
from uq.PreProcessing.sampling.Base import SampleBase

seed = 42


class PseudoMonteCarloSampleBase(SampleBase):
    def __init__(self, parameters=List[Parameter], sample_size=100):
        super().__init__(parameters=parameters)
        self.sample_size = sample_size
        self.max_sample_size = 10 ** 5  # default

    def get_samples(self):
        if self.samples is None:
            self.generate_samples()
            self.assign_parameter_index()
            self.assign_parameter_name_if_missing()
            self.transform_samples()
        return self.samples

    def generate_samples(self):
        raise NotImplementedError

    def transform_samples(self):
        samples = self.samples
        for index in range(len(self.parameters)):
            parameter = self.parameters[index]
            samples[:, index] = parameter.transform(samples[:, index])

        self.samples = samples

    def get_sample_size(self):
        return self.sample_size

    def set_sample_size(self, sample_size):
        self.sample_size = sample_size

    def args(self, sampling):
        sampling["sample_size"] = len(self.get_samples())
        return sampling

    def get_index_independent_samples(self):
        return np.arange(self.sample_size, dtype=int)

    def check_space_filling_measure(self):
        # see https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.qmc.discrepancy.html#scipy.stats.qmc.discrepancy
        return qmc.discrepancy(self.get_samples())

    def get_statistics(self):
        return self.check_uniform_distribution()

    def get_correlation(self):
        return np.corrcoef(self.get_samples().transpose())

    def check_uniform_distribution(self):
        # TODO: discuss necessity
        samples = self.get_samples()
        for col in range(samples.shape[1]):
            sample_col = samples[:, col]
            counts, __ = np.histogram(sample_col)
            plt.hist(sample_col)
            plt.show()
            x1_stat, p1 = chisquare(counts)
            print(p1)


class SampleBaseSobol(PseudoMonteCarloSampleBase):
    def __init__(self, parameters, sample_size=80, is_ignore_convergence=False):
        self.is_ignore_convergence = is_ignore_convergence
        super().__init__(parameters, sample_size=sample_size)

    def get_sample_sizes_for_convergence(self):
        sizes = 2 ** np.arange(20)
        return np.extract(sizes <= self.max_sample_size, sizes)

    def print_sample_sizes_needed_for_convergence(self):
        sizes = self.get_sample_sizes_for_convergence()
        print(f"Sample sizes that fulfil convergence criteria: {sizes}.")

    def _check_sample_size_convergence(self, sample_size, factor=1):

        if self.is_ignore_convergence:
            return

        rest = np.log2(sample_size)
        if rest.is_integer() is False:
            pass
            raise ValueError(
                f"Convergence of Sobol indices can not be guaranteed. "
                f"Choose a sample size of {int(2 ** np.floor(rest) * factor)} or {int(2 ** np.ceil(rest) * factor)}."
            )

    def generate_samples(self):
        self._check_sample_size_convergence(self.sample_size)
        # TODO replace by? https://chaospy.readthedocs.io/en/latest/user_guide/fundamentals/quasi_random_samples.html
        sampler = qmc.Sobol(d=len(self.parameters), seed=seed)  # d = dimension
        sample = sampler.random(self.sample_size)
        self.samples = sample


class SampleBaseSaltelli(SampleBaseSobol):
    def __init__(
        self,
        parameters,
        sample_size=80,
        is_use_for_sensitivity_analysis=True,
        is_ignore_convergence=False,
    ):
        self.is_use_for_sensitivity_analysis = is_use_for_sensitivity_analysis
        super().__init__(
            parameters=parameters,
            sample_size=sample_size,
            is_ignore_convergence=is_ignore_convergence,
        )

    def get_sample_sizes_for_convergence(self):
        sizes = 2 ** np.arange(20) * self.get_matrix_multiplicator()
        return np.extract(sizes <= self.max_sample_size, sizes)

    def generate_samples(self):
        self.check_multiplicator()
        parameter = self.get_parameters_dict()
        sample_size = self.get_saltelli_sample_size()
        sample = sobol.sample(parameter, N=sample_size)
        # deprecated removed after SALib 1.5
        # sample = saltelli.sample(parameter,
        #                          N=sample_size)  # do not use skip_values=seed, because this value can not be chosen freely

        self._check_sample_size_convergence(
            sample_size, factor=self.get_matrix_multiplicator()
        )
        self.check_number_of_produced_samples(sample)
        self.samples = sample

    def get_parameters_dict(self):
        parameter = {
            "num_vars": len(self.parameters),
            "bounds": [[0, 1]] * len(self.parameters),
            "names": [f"x{i}" for i in range(len(self.parameters))]
        }
        return parameter

    def check_number_of_produced_samples(self, sample):
        if sample.shape[0] > self.sample_size:
            print(
                f"WARNING Algorithm produced {sample.shape[0]}. "
                f"{sample.shape[0] - self.sample_size} samples more than required."
            )

    def get_matrix_multiplicator(self):
        # Saltelli's Algorithm
        matrix_multiplicator = 2 * len(self.parameters) + 2
        return matrix_multiplicator

    def get_saltelli_sample_size(self):
        sample_size = int(np.ceil(self.sample_size / self.get_matrix_multiplicator()))
        return sample_size

    def check_multiplicator(self):

        matrix_multiplicator = self.get_matrix_multiplicator()

        if self.sample_size % matrix_multiplicator != 0:
            if self.is_use_for_sensitivity_analysis:
                raise ValueError(
                    f"{self.sample_size} cross-samples required. "
                    f"The corresponding sample size for sensitivity analysis is {self.sample_size / matrix_multiplicator}. "
                    f"Make sure that the number of cross-samples is a multiple of {matrix_multiplicator}."
                )
            else:
                print(
                    f"WARNING The number of cross-samples is not a multiple of {matrix_multiplicator}. "
                    f"You can use this sampling for forward propagation only. "
                    f"However, for that purpose LHS, Sobol samplings should be used (accelerated convergence)."
                )

    def get_independent_samples(self):
        return self.get_samples()[self.get_independent_samples(), :]

    def get_index_independent_samples(self):
        # there are two independent sampling matrices A and B
        # extract corresponding samples
        return np.arange(
            2 * self.sample_size / self.get_matrix_multiplicator(), dtype=int
        )


class SampleBaseLatinHypercube(PseudoMonteCarloSampleBase):
    def __init__(self, parameters=List[Parameter], sample_size=100):
        super().__init__(parameters, sample_size=sample_size)

    def generate_samples(self):
        sampler = qmc.LatinHypercube(d=len(self.parameters), seed=seed)  # d = dimension
        sample = sampler.random(self.sample_size)
        self.samples = sample


class SampleBaseHalton(PseudoMonteCarloSampleBase):
    def __init__(self, parameters=List[Parameter], sample_size=100):
        super().__init__(parameters, sample_size=sample_size)

    def generate_samples(self):
        sampler = qmc.Halton(d=len(self.parameters), seed=0)  # d = dimension
        sample = sampler.random(self.sample_size)
        self.samples = sample


if __name__ == "__main__":
    pass
