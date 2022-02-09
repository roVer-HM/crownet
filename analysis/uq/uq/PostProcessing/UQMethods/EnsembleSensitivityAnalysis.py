from sklearn.model_selection import ShuffleSplit

from uq.PostProcessing.UQMethods.SensitivityAnalysis import SurrogateModelBasedSensitivityAnalysis
from ...PostProcessing.QuantityOfInterest import QuantityOfInterest
from ..SurrogateModels import Model, KrigingModel
from ..UQMethods.Analysis import Analysis
from ...PreProcessing.sampling.Base import SampleBase
from ...PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli

import numpy as np
import pandas as pd

seed=42

class EnsembleSensitivityAnalysis(Analysis):

    def __init__(self,
                 sample_base: SampleBase,
                 quantity_of_interest_values: QuantityOfInterest,
                 model: Model,
                 surrogate_model_sample_base: SampleBaseSaltelli,
                 nr_ensembles=100,
                 train_size=0.5
                 ):
        self.model = model
        self.surrogate_model_sample_base = surrogate_model_sample_base
        self.train_size = train_size
        self.nr_ensembles = nr_ensembles
        self._indices = None
        super().__init__(sample_base, quantity_of_interest_values)

    def get_train_data_indices(self, train_size, nr_ensembles):

        if self._indices is None:
            # The ShuffleSplit iterator will generate a user defined number of independent train / test dataset splits. Samples are first shuffled and then split into a pair of train and test sets.
            # https://scikit-learn.org/stable/modules/cross_validation.html#cross-validation
            kf = ShuffleSplit(n_splits=nr_ensembles, train_size=train_size, random_state=seed)

            train_data_indices = list()
            for indices_training, trainig_indices in kf.split(X=self.sample_base.get_samples()):
                indices_training.sort()
                train_data_indices.append(indices_training)
            self._indices = np.array(train_data_indices)

        return self._indices


    def get_results(self):

        training_indices = self.get_train_data_indices(train_size=self.train_size, nr_ensembles=self.nr_ensembles)

        df = pd.DataFrame()
        for model_index in range(self.nr_ensembles):
            self.model.set_train_size_indices(training_indices[model_index])
            sa_surrogate = SurrogateModelBasedSensitivityAnalysis(sample_base=self.sample_base,
                                                                              quantity_of_interest_values=self.qoi_values,
                                                                              surrogate_model_sample_base=self.surrogate_model_sample_base,
                                                                              model=self.model,
                                                                              )
            df_ = sa_surrogate.get_results()
            df_ = self._add_model_index_(df_, model_index)
            df = pd.concat([df, df_])
        return df

    def _add_model_index_(self, df, model_index):
        old_idx = df.index.to_frame()
        old_idx.insert(0, "SurrogateModel", model_index)
        df.index = pd.MultiIndex.from_frame(old_idx)
        return df



    def plot_results(self):
        pass

if __name__ =="__main__":
    pass

