from unittest import TestCase

import numpy as np
import pandas as pd
from numpy.testing import assert_array_almost_equal

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from ..PostProcessing.UQMethods.SensitivityAnalysis import SensitivityAnalysis
from ..PreProcessing.Parameter import UniformParameter


class TestSensitivityAnalysis(TestCase):

    def setUp(self) -> None:
        s = 256
        sampling = SampleBaseSaltelli(parameters=[UniformParameter(), UniformParameter(), UniformParameter()], sample_size=s)

        # compute dummy quantity of interest
        samples = sampling.get_samples()
        y = 3 * samples[:, 0] ** 2 + 10 * samples[:, 1]
        y2 = y + (2 + samples[:, 2]) ** 3
        y = y.reshape([len(y), -1])  # time 0.0s
        y2 = y2.reshape([len(y2), -1])  # time 0.4s

        qoi_static = QuantityOfInterest(values=y, name="static_qoi")  # assume the qoi is static

        index = pd.MultiIndex.from_product([[0.0, 0.4], np.arange(s), [0]], names=["timeStep", "run_id", "id"])
        qoi_time_dep = QuantityOfInterest(values=pd.DataFrame(data=np.vstack([y, y2]), index=index),
                                          name="time_dependent_qoi")
        self.y_static = y
        self.sa_static = SensitivityAnalysis(sampling, qoi_static)
        self.sa_time_dep = SensitivityAnalysis(sampling, qoi_time_dep)

        self.col_vals = ['Value', 'Confidence_level_low', 'Confidence_level_high']
        # from SALib
        self.sobol_indices = {'S1': np.array([0.5, 0.4, 0.]),
                              'S1_conf': np.array([0.2, 0.01, 0.]),
                              'ST': np.array([0.9, 0.9, 0.]),
                              'ST_conf': np.array([0.2, 0.1, 0.]),
                              'S2': np.array([[np.nan, 0.2, 0.3],
                                              [np.nan, np.nan, 0.1],
                                              [np.nan, np.nan, np.nan]]),
                              'S2_conf': np.array([[np.nan, 0.01, 0.02],
                                                   [np.nan, np.nan, 0.05],
                                                   [np.nan, np.nan, np.nan]])}


    def test_add_time_index_key(self):
        name = "testIndex"
        time_step = 0.65
        df_ = pd.DataFrame(data=[-2, 3], index=pd.Index([11, 12], name=name))
        df = self.sa_static._add_time_index_key(df_, time_key=time_step)
        assert (df.index.names == ['time', name])
        assert (all(df.index.get_level_values(0) == time_step))

    def test_get_first_order_sens(self):
        si = self.sobol_indices
        name = 'FirstOrderSensitivityIndex'
        df = self.sa_static._extract_indices_from_dict(name, si)
        should = np.array([[0.5, 0.3, 0.7],
               [0.4, 0.39, 0.41],
               [0., 0., 0.]])
        assert_array_almost_equal(should, df.values)
        assert(all(df.index.get_level_values(0)==name))
        assert (self.col_vals == df.columns.to_list())


    def test_get_total_order_sens(self):
        si = self.sobol_indices
        name = 'TotalOrderSensitivityIndex'
        df = self.sa_static._extract_indices_from_dict(name, si)
        should = np.array([[0.9, 0.7, 1.1],
                [0.9, 0.8, 1. ],
                [0. , 0. , 0. ]]
                )
        assert_array_almost_equal(should, df.values)
        assert(all(df.index.get_level_values(0)==name))
        assert (self.col_vals == df.columns.to_list())


    def test_second_order_index(self):
        df = self.sa_static._get_second_order_indices_from_dict(self.sobol_indices)
        should = np.array([[0.2, 0.19, 0.21],
               [0.3, 0.28, 0.32],
               [0.1, 0.05, 0.15]])
        assert_array_almost_equal(should, df.values)
        interaction_possibilities = ['Parameter0_Parameter1', 'Parameter0_Parameter2', 'Parameter1_Parameter2']
        assert_array_almost_equal(should, df.values)
        assert(df.index.get_level_values(1).to_list() == interaction_possibilities)
        assert (self.col_vals == df.columns.to_list())

    def test_get_time(self):
        shape = (9, 3) # 3x 1.order, 3x total order, 3 interactions (1-2,1-3,2-3) and 3 cols
        df = self.sa_static.get_sensitivity_indices_for_time(self.y_static)
        assert (self.col_vals == df.columns.to_list())
        assert (df.shape == shape)

        df = self.sa_time_dep.get_sensitivity_indices_for_time(self.y_static)
        assert (self.col_vals == df.columns.to_list())
        assert (df.shape == shape)


    def test_get_indices_nan(self):
        y = self.y_static
        y[1:4,:] = np.nan
        df = self.sa_static.get_sensitivity_indices_for_time(y)
        assert (self.col_vals == df.columns.to_list())


    def test_get_static_results(self):
        res = self.sa_static.get_results()
        assert (res.shape == (9,3))
        assert all(res.index.get_level_values(0).to_numpy() == "static")


    def test_get_time_dep_results(self):
        res = self.sa_time_dep.get_results()
        assert(res.shape == (18, 3))
        assert(res.index.get_level_values(0).to_numpy().dtype == np.dtype('float64'))

    def test_plot_static_results(self):
        # TODO use backend
        return
        self.sa_static.plot_results()

    def test_plot_time_dep_results(self):
        # TODO use backend
        return
        self.sa_time_dep.plot_results()




