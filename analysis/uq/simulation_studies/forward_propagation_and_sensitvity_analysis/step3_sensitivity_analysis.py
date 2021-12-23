from uq.PreProcessing.sampling.utils import read_json
from uq.PreProcessing.sampling.MonteCarlo import SampleBaseSaltelli
from uq.PostProcessing.UQMethods.SensitivityAnalysis import SensitivityAnalysis
from uq.PostProcessing.UQMethods.SensitivityAnalysis import SurrogateModelBasedSensitivityAnalysis
from uq.PostProcessing.QuantityOfInterest import QuantityOfInterest
from uq.PostProcessing.SurrogateModels import KrigingModel

if __name__ == "__main__":

    # Read sample values
    samplingMC = read_json("saltellisExtensionSamples.json")  # If not done yet, run step1_generate_parameter_combinations.py
    samples = samplingMC.get_samples()

    #TODO replace this by simulation run values
    y =  2*samples[:, 0] ** 2 +  samples[:, 1] +  samples[:, 2]
    y = y.reshape( [len(y),-1])

    qoi = QuantityOfInterest(values=y, name="qoi")

    # Conduct sensitivity analysis
    sa = SensitivityAnalysis(sample_base=samplingMC, quantity_of_interest_values=qoi)
    results = sa.get_results()
    #sa.plot_results()


    #TODO Conduct sensitivity analysis based on surrogate model
    # sample_base_surrogate: SampleBaseSaltelli = SampleBaseSaltelli(parameters=samplingMC.get_parameters(),
    #                                            sample_size=2**12)
    # sa2 = SurrogateModelBasedSensitivityAnalysis(sample_base=samplingMC,
    #                                              quantity_of_interest_values=qoi,
    #                                              surrogate_model_sample_base=sample_base_surrogate,
    #                                              model=KrigingModel()
    #                                              )
    # results2 = sa2.get_results()
















