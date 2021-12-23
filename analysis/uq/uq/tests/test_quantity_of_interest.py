from unittest.case import TestCase

import numpy as np
import pandas as pd

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest


class TestForwardPropagationAnalysis(TestCase):

    def test_array(self):
        s=100
        y = np.random.normal(0, 2, size=(s, 1))
        qoi_static = QuantityOfInterest(values=y, name="static_qoi")
        assert(qoi_static.is_static())

    def test_array_2(self):
        s=100
        y = np.random.normal(0, 2, s)
        qoi_static = QuantityOfInterest(values=y)
        assert(qoi_static.is_static())
        assert(qoi_static.get_name()=="qoi")


    def test_df_wrong_time_step_index(self):
        s = 100
        y = np.random.normal(0, 2, size=(s, 1))
        y2 = np.random.normal(2, 1.2, size=(s, 1))

        index = pd.MultiIndex.from_product([[0.0, 0.4], np.arange(s), [0]], names=["timeStep", "run_id", "id"])
        df = pd.DataFrame(data=np.vstack([y, y2]), index=index)
        qoi_time_dep = QuantityOfInterest(values=df,
                                          name="time_dependent_qoi")

        assert (qoi_time_dep.is_static()==False)

    def test_df_no_time_step_index(self):
        s=100
        y = np.random.normal(0, 2, size=(s, 1))

        index = pd.MultiIndex.from_product([np.arange(s), [0]], names=["run_id", "id"])
        df = pd.DataFrame(data=y, index=index)
        qoi_time_dep = QuantityOfInterest(values=df,
                                          name="time_dependent_qoi")

        assert(qoi_time_dep.values.index.nlevels==3)
        assert (qoi_time_dep.is_static())



