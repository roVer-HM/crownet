import numpy as np
import pandas as pd
from SALib.analyze.sobol import analyze
from matplotlib import pyplot as plt

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PostProcessing.SurrogateModels import Model
from uq.PostProcessing.UQMethods.Analysis import Analysis
from uq.PreProcessing.sampling.Base import SampleBase
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli


class SensitivityAnalysis(Analysis):
    COL_NAMES = ['Value', 'Confidence_level_low', 'Confidence_level_high']
    INDEX_TYPE = 'SensitivityIndexType'
    PARAMETER = 'Parameter'

    def __init__(self, sample_base: SampleBaseSaltelli, quantity_of_interest_values: QuantityOfInterest, threshold_failed_sims=0.1):
        self.threshold_failed_sims = threshold_failed_sims
        if isinstance(sample_base, SampleBaseSaltelli) is False:
            raise ValueError(f"Sample-base must be of type SampleBaseSaltelli. Got {type(sample_base)}.")
        super().__init__(sample_base, quantity_of_interest_values)

    def get_results(self):
        return self._get_sensitivity_indices()

    def _get_sensitivity_indices(self) -> pd.DataFrame:

        df = pd.DataFrame()
        for time_key, qoi in self.qoi_values.get_values().groupby(by=[QuantityOfInterest.TIME_INDEX_NAME], as_index=False):
            qoi = qoi.droplevel(QuantityOfInterest.TIME_INDEX_NAME)
            df_ = self.get_sensitivity_indices_for_time(qoi=qoi.to_numpy(), time_key=time_key)
            df = pd.concat([df,df_])
        return df

    def get_sensitivity_indices_for_time(self, qoi: np.ndarray, time_key = "static"):

        qoi = self.handle_missing_qoi_vals(qoi, time_key)

        sobol_indices = analyze(
            problem=self.sample_base.get_parameters_dict(),
            Y=qoi.ravel()
        )
        first_order_index = self._extract_indices_from_dict('FirstOrderSensitivityIndex', sobol_indices)
        total_order_index = self._extract_indices_from_dict('TotalOrderSensitivityIndex', sobol_indices)
        second_order_index = self._get_second_order_indices_from_dict(sobol_indices)

        df = pd.concat([first_order_index, total_order_index, second_order_index])
        df.index.names = [SensitivityAnalysis.INDEX_TYPE, SensitivityAnalysis.PARAMETER]

        df = self._add_time_index_key(df, time_key)

        return df

    def handle_missing_qoi_vals(self, qoi, time_key):
        qoi = qoi.ravel()
        # https://github.com/simetenn/uncertainpy/blob/master/src/uncertainpy/core/uncertainty_calculations.py
        nan_percentage = np.count_nonzero(np.isnan(qoi)) / len(qoi)
        if nan_percentage > 0:
            print(f"{nan_percentage * 100:.1f}% of the {self.qoi_values.get_name()} are nan for time_key {time_key}.")
            if nan_percentage > self.threshold_failed_sims:
                raise ValueError(f"Threshold value = {self.threshold_failed_sims * 100:.1f}%). Please re-run the simulations.")
            else:
                mean = np.nanmean(qoi)
                print(f"Replace nan values by mean of remaining samples (={mean:.3f}).")
                qoi = np.nan_to_num(x=qoi, nan=mean)
        return qoi

    def _add_time_index_key(self, df, time_key):
        old_idx = df.index.to_frame()
        old_idx.insert(0, QuantityOfInterest.TIME_INDEX_NAME, time_key)
        df.index = pd.MultiIndex.from_frame(old_idx)
        return df

    def _extract_indices_from_dict(self, index_name, sobol_indices):
        # sobol_indices := dict from SALib.sobol.analyze
        if index_name == 'FirstOrderSensitivityIndex':
            index_type = 'S1'
        elif index_name == 'TotalOrderSensitivityIndex':
            index_type = 'ST'
        else:
            raise ValueError(f"Index of type {index_name} not allowed.")

        sensitivity_index = sobol_indices[index_type]
        sensitivity_index_CI_low = sensitivity_index - sobol_indices[f'{index_type}_conf']
        sensitivity_index_CI_high = sensitivity_index + sobol_indices[f'{index_type}_conf']
        df = pd.DataFrame(
            data=np.row_stack([sensitivity_index, sensitivity_index_CI_low, sensitivity_index_CI_high]).transpose(),
            columns=SensitivityAnalysis.COL_NAMES,
            index=pd.MultiIndex.from_product([[index_name], self.sample_base.get_parameter_names()])
        )

        return df

    def _get_second_order_indices_from_dict(self, sobol_indices):
        # sobol_indices := dict from SALib.sobol.analyze

        index_name = 'SecondOrderSensitivityIndex'
        index_type = 'S2'

        df = pd.DataFrame()
        for i1, i2 in np.argwhere(np.isnan(sobol_indices[index_type]) == False):
            second_order_index = sobol_indices[index_type][i1, i2]
            second_order_index_CI_low = second_order_index - sobol_indices[f'{index_type}_conf'][i1, i2]
            second_order_index_CI_high = second_order_index + sobol_indices[f'{index_type}_conf'][i1, i2]

            interaction = self.sample_base.get_interaction_name_from_parameter_index(i1,i2)
            df_ = pd.DataFrame(
                data=[[second_order_index, second_order_index_CI_low, second_order_index_CI_high]],
                columns=SensitivityAnalysis.COL_NAMES,
                index=pd.MultiIndex.from_product([[index_name], [interaction]])
            )
            df = pd.concat([df, df_])

        return df

    def plot_results(self, save_fig = False, output_folder = None ):
        levels = [SensitivityAnalysis.INDEX_TYPE, SensitivityAnalysis.PARAMETER]
        for index_type, values in self.get_results().groupby(level=levels):
            values.reset_index(level=levels, drop=True, inplace=True)
            values.plot(marker="o")
            plt.title(index_type[1])
            plt.ylabel(index_type[0])
            plt.ylim(bottom=0.0)
            if self.is_qoi_time_static():
                plt.xlabel("(no time-dependency)")
            f = f"SensitivityIndex{index_type[0]}{index_type[1]}.png"
            self.save_figure_to_png(output_folder, save_fig, file_name=f)
            plt.show()


class SurrogateModelBasedSensitivityAnalysis(SensitivityAnalysis):
    def __init__(self, sample_base: SampleBase,
                 quantity_of_interest_values: QuantityOfInterest,
                 model : Model,
                 surrogate_model_sample_base: SampleBaseSaltelli,
                 ):
        self.model = model
        self.surrogate_model_sample_base = surrogate_model_sample_base
        super().__init__(sample_base, quantity_of_interest_values)


    def _get_sensitivity_indices(self) -> pd.DataFrame:

        df = pd.DataFrame()
        for time_key, qoi in self.qoi_values.get_values().groupby(by=[QuantityOfInterest.TIME_INDEX_NAME], as_index=False):
            qoi = qoi.droplevel(QuantityOfInterest.TIME_INDEX_NAME)

            #TODO: one surrogate model/time step  -> does not capture dynamics
            #TODO: Log models
            self.model.construct(x=self.sample_base.get_samples(), y=qoi.values)
            qoi_surrogate = self.model.evaluate(x=self.surrogate_model_sample_base.get_samples())

            df_ = self.get_sensitivity_indices_for_time(qoi=qoi_surrogate, time_key=time_key)
            df = pd.concat([df,df_])
        return df



