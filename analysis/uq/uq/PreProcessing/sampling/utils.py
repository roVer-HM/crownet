import copy
import json

import numpy as np

from uq.PreProcessing.sampling.Base import SampleBase
from uq.PreProcessing.sampling.MonteCarlo import *
from uq.PreProcessing.sampling.GaussianQuadrature import *


def read_json(file_path) -> SampleBase:
    with open(file_path) as f:
        sampling = json.load(f)

    parameters = copy.deepcopy(sampling['parameter'])
    parameters_ = list()
    for p, p_ in parameters.items():
        dist_type = p_["dist_type"]
        p_.pop("dist_type")
        p_["name"] = p
        parameters_.append(globals()[dist_type](**p_))

    sampling_ = None
    if sampling['sampling_method'] in ["SampleBaseSaltelli","SampleBaseHalton", "SampleBaseSobol", "SampleBaseLatinHypercube"]:
        sampling_ = globals()[sampling['sampling_method']](sample_size=sampling['sample_size'], parameters=parameters_)
    elif sampling['sampling_method'] in ["SampleBaseQuadraturePoints"]:
        sampling_ = globals()[sampling['sampling_method']](quad_order=sampling['quad_order'], parameters=parameters_)
    else:
        ValueError(f"Sampling method {sampling['sampling_method']} not found. Please check the spelling.")

    if np.allclose(sampling_.get_samples(), np.array(sampling['sample_values']), atol=10**-6) is False:
        raise ValueError("Sample values could not be re-produced.")

    return sampling_