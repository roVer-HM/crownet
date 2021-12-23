import unittest

from numpy.testing import assert_almost_equal

from uq.PreProcessing.Parameter import GaussianParameter, ExponentialParameter2, ExponentialParameter
from uq.PreProcessing.sampling.MonteCarlo import *

PRECISION = 4

class TestParameter(unittest.TestCase):

    def setUp(self):
        delta = 10**-6
        x = np.linspace(delta,1-delta, 1000000)
        x = x.reshape(len(x), -1)
        self.sampling_1d = x
        self.sampling_2d = np.repeat(x, 2, axis=1)
        print()

    def get_sampling_2d(self):
        return self.sampling_2d.copy()

    def get_sampling_1d(self):
        return self.sampling_1d.copy()

    def test_param_gaussian(self):
        mean_should = 0
        std_should = 0.2
        gaussian_param = GaussianParameter(mean=mean_should, std=std_should)
        samples_trans = gaussian_param.transform(self.get_sampling_1d())
        assert_almost_equal(samples_trans.mean(), gaussian_param.get_mean(), PRECISION)
        assert_almost_equal(samples_trans.std(), gaussian_param.get_std(), PRECISION)

    def test_param_expon_1(self):
        min_value = 4.2
        inter_arrival_time = 2
        expon_param = ExponentialParameter(x_shift=min_value, lamda=inter_arrival_time)
        samples_trans = expon_param.transform(self.get_sampling_1d())
        assert_almost_equal(samples_trans.min(), min_value, PRECISION)
        assert_almost_equal(samples_trans.std(), expon_param.get_std(), PRECISION)
        print()


    def test_param_expon_2(self):
        mean = 10
        std = 2
        expon_param = ExponentialParameter2(mean=mean, std=std)
        samples_trans = expon_param.transform(self.get_sampling_1d())
        assert_almost_equal(samples_trans.mean(), expon_param.get_mean(), PRECISION)
        assert_almost_equal(samples_trans.std(), expon_param.get_std(), PRECISION)
        print()

    def test_mapper_scalar(self):
        g_c = ParameterMapper.map(GaussianParameter())
        assert_almost_equal (g_c.get_mom_parameters()['shift'][0], 0.) # mean
        assert_almost_equal(g_c.get_mom_parameters()['scale'][0], 1.) # std


    def test_mapper_types(self):
        lamda = 0.6
        e1 = ExponentialParameter(x_shift=2.0, lamda=lamda)
        e2 = ExponentialParameter2(mean=3.0, std=0.2)
        u = UniformParameter()
        g = GaussianParameter(std=2.2)
        vec = ParameterMapper.map([e1, e2, u, g])

        e1_c, e2_c, u_c, g_c = vec
        assert_almost_equal(e1_c.get_mom_parameters()['shift'][0] , e1.get_shift())
        assert_almost_equal(e1_c.get_mom_parameters()['scale'][0] , e1.get_scale())
        assert_almost_equal(e1_c.get_mom_parameters()['scale'][0] , 1/lamda)

        assert_almost_equal(e2_c.get_mom_parameters()['shift'][0] , e2.get_shift())
        assert_almost_equal(e2_c.get_mom_parameters()['scale'][0] , e2.get_scale())

        assert ( u_c.lower[0] == 0.0 and u_c.upper[0] == 1.0)
        assert_almost_equal(g_c.get_mom_parameters()['scale'][0], 2.2)


if __name__ == '__main__':
    unittest.main()
