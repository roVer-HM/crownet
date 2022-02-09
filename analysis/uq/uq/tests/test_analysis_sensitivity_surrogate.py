from unittest import TestCase

from numpy.testing import assert_array_almost_equal

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from ..PostProcessing.SurrogateModels import KrigingModel
from ..PostProcessing.UQMethods.SensitivityAnalysis import SurrogateModelBasedSensitivityAnalysis, SensitivityAnalysis
from ..PreProcessing.Parameter import UniformParameter


class TestSensitivityAnalysisSurrogate(TestCase):

    def setUp(self) -> None:
        s = 256
        self.parameters = [UniformParameter(), UniformParameter(), UniformParameter()]
        self.sampling = SampleBaseSaltelli(parameters=self.parameters, sample_size=s)
        samples = self.sampling.get_samples()
        y = 2*samples[:, 0] ** 2 +  samples[:, 1] +  samples[:, 2]
        self.qoi_static = QuantityOfInterest(values=y, name="static_qoi")




    def test_default_behavior(self):

        self.sa_static = SensitivityAnalysis(self.sampling, self.qoi_static)
        self.sa_static_surrogate = SurrogateModelBasedSensitivityAnalysis(sample_base=self.sampling,
                                                                          quantity_of_interest_values=self.qoi_static,
                                                                          surrogate_model_sample_base=self.sampling,
                                                                          model=KrigingModel(),
                                                                          )
        res0 = self.sa_static.get_results()
        res1 = self.sa_static_surrogate.get_results()
        assert_array_almost_equal(res0["Value"].values, res1["Value"].values, decimal=3)

    def test_increase_sample_size_behavior(self):
        sampling_large = SampleBaseSaltelli(parameters=self.parameters)
        max_sample_size = sampling_large.get_sample_sizes_for_convergence()[-1]
        sampling_large.set_sample_size(max_sample_size)

        self.sa_static_surrogate = SurrogateModelBasedSensitivityAnalysis(sample_base=self.sampling,
                                                                          quantity_of_interest_values=self.qoi_static,
                                                                          surrogate_model_sample_base=sampling_large,
                                                                          model=KrigingModel(),
                                                                          )
        self.sa_static_surrogate.get_results()
        assert(len(self.sa_static_surrogate.model.y_learn) == 256)
        assert (len(self.sa_static_surrogate.model.y_pred) == max_sample_size)



