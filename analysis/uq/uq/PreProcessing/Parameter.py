import abc
import json

from scipy.stats import uniform, norm, expon
import chaospy

from typing import  Union, List

class Parameter(abc.ABC):

    def __init__(self, index=0, name=""):
        self.name = name
        self.index = index

    def get_name(self):
        return self.name

    def set_name(self, name):
        self.name = name

    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)

    def set_index(self, index):
        self.index = index

    def get_index(self):
        return self.index

    @abc.abstractmethod
    def get_mean(self):
        raise NotImplementedError

    @abc.abstractmethod
    def get_std(self):
        raise NotImplementedError

    @abc.abstractmethod
    def transform(self, samples):
        # transformation types, see scikit-learn for background reading
        # https://scikit-learn.org/stable/modules/preprocessing.html#non-linear-transformation
        raise NotImplementedError

    def __eq__(self, other):
        return self.toJSON() == other.toJSON()


class UniformParameter(Parameter):
    def __init__(self, lower_bound=0.0, upper_bound=1.0, index=0, name=""):
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        super().__init__(index=index, name=name)

    def get_lower_bound(self):
        return self.lower_bound

    def get_upper_bound(self):
        return self.upper_bound

    def get_mean(self):
        return 0.5 *(self.lower_bound +self.upper_bound)

    def get_std(self):
        return (self.upper_bound -self.lower_bound ) /12**0.5

    def transform(self, samples):
        return uniform.ppf(samples,loc=self.lower_bound, scale=self.upper_bound-self.lower_bound)


class GaussianParameter(Parameter):
    def __init__(self, mean=0., std= 1., index=0,  name=""):
        self.mean = mean
        self.std = std
        super().__init__(index=index, name=name)

    def get_mean(self):
        return self.mean

    def get_std(self):
        return self.std

    def transform(self, samples):
        return norm.ppf(samples, loc=self.mean, scale=self.std)


class ExponentialParameter2(Parameter):
    def __init__(self, mean=2.0, std=0.1, index=0, name=""):
        self.std = std
        self.mean = mean
        super().__init__(index=index, name=name)

    def get_mean(self):
        if self.mean != expon.mean(loc=self.get_shift(), scale=self.get_scale()):
            raise ValueError("Wrong mean value.")
        return self.mean

    def get_std(self):
        if self.std != expon.std(loc=self.get_shift(), scale=self.get_scale()):
            raise ValueError(f"Wrong std value.")
        return self.std

    def transform(self, samples):
        return expon.ppf(samples, loc=self.get_shift(), scale=self.get_scale())

    def get_scale(self):
        return self.std

    def get_shift(self):
        return self.mean - self.std


class ExponentialParameter(Parameter):
    def __init__(self, lamda=1.0, x_shift=0.0, index=0, name=""):
        self.lamda = lamda
        self.x_shift = x_shift
        super().__init__(index=index, name=name)

    def get_mean(self):
        mean = self.x_shift + self.get_std()
        if mean != expon.mean(loc=self.get_shift(), scale=self.get_scale()):
            raise ValueError("Wrong mean value.")
        return mean

    def get_std(self):
        std = 1 / self.lamda
        if std!= expon.std(loc=self.get_shift(), scale=self.get_scale()):
            raise ValueError("Wrong std value.")
        return std

    def get_shift(self):
        return self.x_shift

    def get_lamda(self):
        return self.lamda

    def get_scale(self):
        return 1/self.lamda

    def transform(self, samples):
        return expon.ppf(samples, loc=self.get_shift(), scale=self.get_scale())



class ParameterMapper:

    @classmethod
    def map(cls, parameter = Union[Parameter, List[Parameter]]):
        if isinstance(parameter, Parameter):
            return cls._map(parameter)
        else:
            return [ParameterMapper._map(p) for p in parameter]


    @classmethod
    def _map(self, parameter):

        if isinstance(parameter, UniformParameter):
            return chaospy.Uniform(lower=parameter.get_lower_bound(), upper=parameter.get_upper_bound())
        elif isinstance(parameter, GaussianParameter):
            return chaospy.Normal(mu=parameter.get_mean(), sigma=parameter.get_std())
        elif isinstance(parameter, ExponentialParameter):
            return chaospy.Exponential(shift=parameter.get_shift(), scale=parameter.get_scale())
        elif isinstance(parameter, ExponentialParameter2):
            return chaospy.Exponential(shift=parameter.get_shift(), scale=parameter.get_scale())
        else:
            raise NotImplementedError(f"Parameter of type {type(parameter)} not supported.")
