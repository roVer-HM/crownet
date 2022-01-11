import abc
import copy
import json
import os
from typing import List

from uq.PreProcessing.Parameter import Parameter


class SampleBase(abc.ABC):

    # https://docs.scipy.org/doc/scipy/reference/stats.qmc.html

    def __init__(self, parameters=List[Parameter]):
        self.parameters = parameters
        self.samples = None

    @abc.abstractmethod
    def get_samples(self):
        raise NotImplementedError("Implement in child class.")

    def write(self, file_path):

        extension = os.path.splitext(file_path)[1]
        if extension == "":
            file_path = file_path + ".json"
            extension = os.path.splitext(file_path)[1]
        if extension != ".json":
            raise ValueError("File must be *.json")


        with open(file_path, 'w+') as fp:
            json.dump(self.to_dict(), fp, indent=3)

        print(f"Saved sampling to {os.path.abspath(file_path)}")

    def to_dict(self):
        samples = self.get_samples()
        params = dict()
        for s in self.parameters:
            attributes_ = copy.deepcopy(s.__dict__)
            attributes_.pop("name")
            attributes_['dist_type'] = s.__class__.__name__
            params[s.get_name()] = attributes_

        sampling = {"parameter": params}
        sampling['sampling_method'] = self.__class__.__name__
        sampling = self.args(sampling)
        sampling['sample_values'] = samples.tolist()
        return sampling

    def args(self, sampling):
        # do nothing
        return sampling

    def set_samples(self, samples):
        self.samples = samples

    def get_parameter_names(self):
        return [p.get_name() for p in self.parameters]


    def get_interaction_name_from_parameter_index(self, first_index=0, second_index=0):
        return f"{self.get_parameter_names()[first_index]}_{self.get_parameter_names()[second_index]}"

    def assign_parameter_name_if_missing(self):
        for index in range(len(self.parameters)):
            parameter = self.parameters[index]
            if parameter.get_name() == "":
                parameter.set_name(f"Parameter{parameter.get_index()}")

    def assign_parameter_index(self):
        for index in range(len(self.parameters)):
            parameter = self.parameters[index]
            parameter.set_index(index)

    def get_parameters(self):
        return self.parameters