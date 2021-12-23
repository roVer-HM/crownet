import pandas as pd
from matplotlib import pyplot as plt

from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PostProcessing.UQMethods.Analysis import Analysis
from uq.PreProcessing.sampling.Base import SampleBase


class ForwardPropagationAnalysis(Analysis):

    def __init__(self, sample_base: SampleBase, quantity_of_interest_values: QuantityOfInterest):
        super().__init__(sample_base, quantity_of_interest_values)

    def get_results(self):

        df = pd.DataFrame()
        for time_key, qoi in self.qoi_values.get_values().groupby(by=[QuantityOfInterest.TIME_INDEX_NAME],
                                                                  as_index=False):
            qoi = qoi.droplevel(QuantityOfInterest.TIME_INDEX_NAME)
            qoi_independent = qoi.to_numpy()[self.sample_base.get_index_independent_samples()]
            df_ = self.get_statistical_data_for_time(qoi=qoi_independent, time_key=time_key)
            df = pd.concat([df, df_])
        return df

    def get_independent_qoi_vals(self):

        df = pd.DataFrame()
        for time_key, qoi in self.qoi_values.get_values().groupby(by=[QuantityOfInterest.TIME_INDEX_NAME],
                                                                  as_index=False):
            qoi = qoi.droplevel(QuantityOfInterest.TIME_INDEX_NAME)
            qoi_independent = qoi.to_numpy()[self.sample_base.get_index_independent_samples()]
            df_ = pd.DataFrame([qoi_independent.ravel()],
                               index=pd.Index(name="time", data=[time_key]),
                               )
            df = pd.concat([df, df_])
        return df


    def plot_results(self, save_fig = False, output_folder = None ):

        self.plot_stats(output_folder, save_fig)
        self.boxplot(output_folder, save_fig)

    def plot_stats(self, output_folder, save_fig):
        values = self.get_results()
        values.drop(columns=["count"], inplace=True)
        values.plot(marker="o")
        plt.title(f"Stats {self.qoi_values.get_name()}")
        if self.is_qoi_time_static():
            plt.xlabel("(no time-dependency)")
        else:
            plt.xlabel("time")
        f = f"{self.qoi_values.get_name()}.png"
        self.save_figure_to_png(output_folder, save_fig, file_name=f)
        plt.show()

    def get_statistical_data_for_time(self, qoi, time_key):
        stats = pd.DataFrame(qoi).describe().transpose()
        stats.index = pd.Index(data=[time_key],name='time')
        return stats

    def boxplot(self, output_folder, save_fig):
        values = self.get_independent_qoi_vals()
        values.transpose().boxplot()
        plt.title(f"Boxplot {self.qoi_values.get_name()}")
        if self.is_qoi_time_static():
            plt.xlabel("(no time-dependency)")
        else:
            plt.xlabel("time")
        f = f"{self.qoi_values.get_name()}Boxplot.png"
        self.save_figure_to_png(output_folder, save_fig, file_name=f)
        plt.show()
        print()

    def hist(self, save_fig = False, output_folder = None, time_key="static"):
        values = self.get_independent_qoi_vals().transpose()
        if time_key not in values.columns:
            raise ValueError(f"Allowed time_keys: {values.columns}. Got: >{time_key}<.")

        values = values[time_key]
        values.hist()
        plt.xlabel(self.qoi_values.get_name())
        title = f"Hist {self.qoi_values.get_name()}"
        if time_key != "static":
            title = f"{title} (time={time_key})"
        plt.title(title)
        f = f"{title}Histogram.png"
        self.save_figure_to_png(output_folder, save_fig, file_name=f)
        plt.show()