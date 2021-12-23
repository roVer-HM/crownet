import unittest

from numpy.testing import assert_almost_equal, assert_array_equal, assert_equal

from uq.PreProcessing.sampling.GaussianQuadrature import SampleBaseQuadraturePoints
from uq.PreProcessing.sampling.MonteCarlo import *
from uq.PreProcessing.Parameter import GaussianParameter, UniformParameter
from uq.PreProcessing.sampling.utils import read_json

PRECISION = 4

class TestSampling(unittest.TestCase):

    def setUp(self) -> None:
        self.parameters_1 = [GaussianParameter()]
        self.parameters_2 = [GaussianParameter(), GaussianParameter(mean=2.0)]
        self.parameters_3 = [GaussianParameter(), GaussianParameter(mean=2.0), GaussianParameter()]

    def test_transformation(self):
        samples = SampleBaseHalton(parameters=self.parameters_2, sample_size=1000000).get_samples()
        par1 = samples[:, 0]
        par2 = samples[:, 1]
        assert_almost_equal(par1.mean(),self.parameters_2[0].get_mean(), decimal=PRECISION)
        assert_almost_equal(par1.std(),self.parameters_2[0].get_std(), decimal=PRECISION)
        assert_almost_equal(par2.mean(),self.parameters_2[1].get_mean(), decimal=PRECISION)


    # test Sobol sampling
    def test_allowed_sample_sizes_Sobol(self):
        samples = SampleBaseSobol(parameters=self.parameters_2)
        sizes_is = samples.get_sample_sizes_for_convergence()
        sizes_should = np.array([1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536])
        assert_array_equal(sizes_is, sizes_should)


    def testSobolSampling(self):
        par_num = 1
        sample_size = 32
        samples = SampleBaseSobol(parameters=self.parameters_1, sample_size=sample_size).get_samples()
        assert(samples.shape[0] == sample_size)
        assert(samples.shape[1] == par_num)

    @unittest.expectedFailure
    def testSobolSamplingFail(self):
        par_num = 1
        sample_size = 33
        samples = SampleBaseSobol(parameters=self.parameters_1, sample_size=sample_size).get_samples()
        assert(samples.shape[0] == sample_size)
        assert(samples.shape[1] == par_num)

    def testSobolSamplingIgnoreError(self):
        par_num = 2
        sample_size = 33
        samples = SampleBaseSobol(parameters=self.parameters_2, sample_size=sample_size, is_ignore_convergence=True).get_samples()
        assert(samples.shape[0] == sample_size)
        assert(samples.shape[1] == par_num)


    # The Saltelli Sampling procedure is based on sobol sequences
    def testSaltelliSampling(self):
        par_num = 2
        sample_size = 48
        samples = SampleBaseSaltelli(parameters=self.parameters_2, sample_size=sample_size).get_samples()
        assert(samples.shape[0] == sample_size)
        assert(samples.shape[1] == par_num)

    @unittest.expectedFailure
    def test_saltelli_sampling(self):
        par_num = 2
        sample_size = 100
        samples = SampleBaseSaltelli(parameters=self.parameters_2, sample_size=sample_size).get_samples()
        assert (samples.shape[0] == sample_size)
        assert (samples.shape[1] == par_num)

    def test_saltelli_sampling_2(self):
        par_num = 3
        sample_size = 512
        samples = SampleBaseSaltelli(parameters=self.parameters_3,
                                     sample_size=sample_size,
                                     is_use_for_sensitivity_analysis=True).get_samples()
        assert (samples.shape[0] == sample_size)
        assert (samples.shape[1] == par_num)

    @unittest.expectedFailure
    def test_saltelli_sampling_55(self):
        par_num = 3
        sample_size = 240
        samples = SampleBaseSaltelli(parameters=self.parameters_3,
                                     sample_size=sample_size,
                                     is_use_for_sensitivity_analysis=True).get_samples()
        assert (samples.shape[0] == sample_size)
        assert (samples.shape[1] == par_num)

    def test_saltelli_sampling_3(self):
        par_num = 3
        sample_size = 73
        samples = SampleBaseSaltelli(parameters=self.parameters_3,
                                     sample_size=sample_size,
                                     is_use_for_sensitivity_analysis=False,
                                     is_ignore_convergence=True).get_samples()
        assert (samples.shape[0] == 80)
        assert (samples.shape[1] == par_num)


    def test_allowed_sizes(self):
        samples = SampleBaseSaltelli(parameters=self.parameters_2)
        sizes_is = samples.get_sample_sizes_for_convergence()
        sizes_should = np.array([6, 12, 24, 48, 96, 192, 384, 768, 1536, 3072, 6144, 12288, 24576, 49152, 98304])
        assert_array_equal(sizes_is, sizes_should)


    def test_writer(self):
        p1 = UniformParameter(lower_bound=20, upper_bound=100)
        p2 = UniformParameter(lower_bound=100, upper_bound=2000)

        file_path = "pMCHalton.json"
        SampleBaseHalton(sample_size=100, parameters=[p1, p2]).write(file_path)

        sampling = read_json(file_path)
        assert_equal(p1, sampling.parameters[0])
        assert_equal(p2, sampling.parameters[1])


    def test_writer_2(self):
        p1 = UniformParameter(lower_bound=20, upper_bound=100)
        p2 = GaussianParameter(mean=20, std=0.3)

        file_path = "pMCQuadPoints.json"
        SampleBaseQuadraturePoints(quad_order=4, parameters=[p1, p2]).write(file_path)

        sampling = read_json(file_path)
        assert_equal(p1, sampling.parameters[0])
        assert_equal(p2, sampling.parameters[1])