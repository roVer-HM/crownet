from unittest.case import TestCase

import numpy as np
import pandas as pd
from numpy.testing import assert_almost_equal, assert_array_almost_equal

from uq.PostProcessing.UQMethods.ForwardPropagation import ForwardPropagationAnalysis
from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from uq.PreProcessing.Parameter import UniformParameter


class TestForwardPropagationAnalysis(TestCase):

    def setUp(self) -> None:
        self.s = 2**16
        self.indep_samples = int(self.s/8*2) # 8 -> 2*n+2 (n=3 parameters), *2 -> 2matrices
        sampling = SampleBaseSaltelli(parameters=[UniformParameter(), UniformParameter(), UniformParameter()], sample_size=self.s)

        self.mean_1, self.mean_2, self.std_1, self.std_2 = 0, 2, 1, 0.2
        y = np.random.normal(self.mean_1, self.std_1, size=(self.s, 1))
        y2 = np.random.normal(self.mean_2,self.std_2, size=(self.s, 1))

        qoi_static = QuantityOfInterest(values=y, name="static_qoi")  # assume the qoi is static

        index = pd.MultiIndex.from_product([[0.0, 0.4], np.arange(self.s), [0]], names=["time", "run_id", "id"])
        qoi_time_dep = QuantityOfInterest(values=pd.DataFrame(data=np.vstack([y, y2]), index=index),
                                          name="time_dependent_qoi")

        self.sa_static = ForwardPropagationAnalysis(sampling, qoi_static)
        self.sa_time_dep = ForwardPropagationAnalysis(sampling, qoi_time_dep)

    def test_get_results(self):
        res = self.sa_static.get_results()
        assert(res['count'].values[0] == self.indep_samples)
        assert_almost_equal(res['mean'].values[0], self.mean_1, decimal=1)
        assert_almost_equal(res['std'].values[0], self.std_1, decimal=1)


    def test_get_time_dep_results(self):
        res = self.sa_time_dep.get_results()
        assert(all(res['count'].values == self.indep_samples))
        assert_array_almost_equal(res['mean'].to_numpy(), np.array([self.mean_1, self.mean_2]), decimal=1)
        assert_array_almost_equal(res['std'].to_numpy(), np.array([self.std_1, self.std_2]), decimal=1)


    def test_plot_time_dependent(self):
        #TODO use backend
        return
        self.sa_time_dep.plot_results()

    def test_plot_static(self):
        # TODO use backend
        return
        self.sa_static.plot_results()