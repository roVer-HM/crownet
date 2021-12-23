import abc

from uq.PreProcessing.sampling.Base import SampleBase
import matplotlib.pyplot as plt
from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest

import os


class Analysis(abc.ABC):

    def __init__(self, sample_base: SampleBase, quantity_of_interest_values: QuantityOfInterest):
        self.sample_base = sample_base
        self.qoi_values = quantity_of_interest_values

    def is_qoi_time_static(self):
        return self.qoi_values.is_static()

    def print_results(self):
        print(f"\n")
        print(f"Output of {self.__class__.__name__} for QoI = {self.qoi_values.get_name()}:")
        print(self.get_results())
        print(f"\n")

    @abc.abstractmethod
    def get_results(self):
        raise NotImplementedError()

    @abc.abstractmethod
    def plot_results(self):
        raise NotImplementedError()

    def save_figure_to_png(self, output_folder, save_fig, file_name):
        # file_name -> *.png
        if save_fig:
            if output_folder is None:
                output_folder = os.getcwd()

            output_folder = os.path.abspath(output_folder)
            if os.path.isdir(output_folder) == False:
                os.makedirs(output_folder)

            file_path = os.path.join(output_folder, file_name)
            plt.savefig(file_path)


if __name__ == "__main__":
    pass