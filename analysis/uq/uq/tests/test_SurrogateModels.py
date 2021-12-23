from unittest import TestCase, expectedFailure
from ..PostProcessing.SurrogateModels import KrigingModel
import numpy as np

class TestKrigingModel(TestCase):

    def test_construct_3_parameters(self):
        np.random.seed = 42
        x = np.random.sample((100,3))
        y = np.random.sample((100,1))
        k = KrigingModel()
        k.construct(x=x, y=y)

        x_pred = np.random.sample((100, 3))
        k.evaluate(x_pred)


    def test_construct_2_parameters(self):
        np.random.seed = 42
        x = np.random.sample((100,2))
        y = np.random.sample((100,1))
        k = KrigingModel()
        k.construct(x=x, y=y)

        x_pred = np.random.sample((100, 2))
        k.evaluate(x_pred)

    @expectedFailure
    def test_construct_n_parameters(self):
        np.random.seed = 42
        x = np.random.sample((100,5))
        y = np.random.sample((100,1))
        k = KrigingModel()
        k.construct(x=x, y=y)




