from unittest import TestCase

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from ..PostProcessing.SurrogateModels import KrigingModel
from ..PostProcessing.UQMethods.EnsembleSensitivityAnalysis import EnsembleSensitivityAnalysis
from ..PreProcessing.Parameter import UniformParameter


class TestSensitivityAnalysisSurrogate(TestCase):

    def setUp(self) -> None:
        s = 256
        self.parameters = [UniformParameter(), UniformParameter(), UniformParameter()]
        self.sampling = SampleBaseSaltelli(parameters=self.parameters, sample_size=s)
        samples = self.sampling.get_samples()
        y = 2 * samples[:, 0] ** 2 + samples[:, 1] + samples[:, 2]
        self.qoi_static = QuantityOfInterest(values=y, name="static_qoi")

    def test_indices(self):
        nr = 5
        t = 0.6
        ensemble_sa = EnsembleSensitivityAnalysis(sample_base=self.sampling,
                                                  quantity_of_interest_values=self.qoi_static,
                                                  surrogate_model_sample_base=self.sampling,
                                                  model=KrigingModel(),
                                                  nr_ensembles=nr,
                                                  train_size=t
                                                  )

        indices = ensemble_sa.get_train_data_indices(nr_ensembles=nr, train_size=t)
        assert (len(indices) == nr)
        assert (indices.shape[1] == int(t * self.sampling.get_sample_size()))

    def test_results(self):
        nr = 3
        ensemble_sa = EnsembleSensitivityAnalysis(sample_base=self.sampling,
                                                  quantity_of_interest_values=self.qoi_static,
                                                  surrogate_model_sample_base=self.sampling,
                                                  model=KrigingModel(),
                                                  nr_ensembles=nr
                                                  )

        df = ensemble_sa.get_results()
        assert (df.index.nlevels == 4)
        assert (len(df) == nr * 9)  # 9 result indices in case of three parameters
