from typing import List

import chaospy

from uq.PreProcessing.Parameter import Parameter, ParameterMapper
from uq.PreProcessing.sampling.Base import SampleBase


class SampleBaseQuadraturePoints(SampleBase):

    def args(self, sampling):
        sampling['quad_order'] = self.quad_order
        return sampling

    def __init__(self, parameters=List[Parameter], quad_order=2):
        self.quad_order = quad_order
        super().__init__(parameters=parameters)

    def get_samples(self):
        self.assign_parameter_index()
        self.assign_parameter_name_if_missing()

        if self.samples is None:
            parameters = ParameterMapper.map(self.parameters)
            joint = chaospy.J(*parameters)
            sample_points, __ = chaospy.generate_quadrature(self.quad_order, joint, rule="gaussian")
            self.samples = sample_points.transpose()
        return self.samples