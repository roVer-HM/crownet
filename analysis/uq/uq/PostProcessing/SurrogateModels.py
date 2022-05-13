import abc

import numpy
import numpy as np
from pykrige import UniversalKriging3D, UniversalKriging
from scipy.stats import normaltest
from sklearn.metrics import r2_score

import chaospy, numpy
import matplotlib.pyplot as plt
from scipy import stats
from chaospy.distributions.operators import joint





class Model(abc.ABC):
    # this is just a wrapper for arbitrary surrogate models

    def __init__(self, train_set_indices = None):
        self.model = None
        self.x_learn = None
        self.y_learn = None
        self.x_pred = None
        self.y_pred = None
        self.train_set_indices = train_set_indices

    def get_y_pred(self):
        return self.y_pred

    def set_y_pred(self, y_pred):
        self.y_pred = y_pred

    def get_y_learn(self):
        return self.y_learn

    def set_y_learn(self, y_learn):
        self.y_learn = y_learn

    def r2_score(self):
        return r2_score(y_true=self.get_y_learn(),y_pred=self.get_y_pred())

    def set_train_size_indices(self, indices):
        self.train_set_indices = indices

    def construct(self, x, y):

        if self.train_set_indices is not None:
            # use subset of data to construct surrogate
            x = x[self.train_set_indices, :]
            y = y[self.train_set_indices, :]

        return self._construct(x, y)


    @abc.abstractmethod
    def evaluate(self, x):
        pass

    @abc.abstractmethod
    def _construct(self, x: np.array, y: np.array):
        pass


class LinearModel(Model):

    def __init__(self):
        super().__init__()


class KrigingModel(Model):

    #  2D and 3D ordinary and universal kriging.

    def __init__(self, exact=True, variogram_model="linear"):
        self.exact = exact
        self.variogram_model = variogram_model
        super().__init__()

    def _construct(self, x: np.array, y: np.array):

        nr_parameters = x.shape[1]
        if nr_parameters == 2:
            model = UniversalKriging(
                x=x[:, 0].ravel(),
                y=x[:, 1].ravel(),
                z=y.ravel(),
                variogram_model=self.variogram_model,
                exact_values=self.exact
            )
        elif nr_parameters == 3:
            model = UniversalKriging3D(
                x=x[:, 0].ravel(),
                y=x[:, 1].ravel(),
                z=x[:, 2].ravel(),
                val=y.ravel(),
                variogram_model=self.variogram_model,
                exact_values=self.exact
            )
        else:
            raise NotImplementedError("Kriging model only available for 2 or 3 parameters.")

        self.model = model
        self.set_y_learn(y.ravel())
        return self

    def check_Q1_measure(self):
        Q1, Q2, cR = self.model.get_statistics()
        nr_train_samples = self.model.VALUES.size
        if Q1 > (2 / numpy.sqrt(nr_train_samples - 1)):
            raise UserWarning(f"Q1 value over threshold.")

    def check_residuals(self):
        residuals = self.model.get_epsilon_residuals()
        k2, p = normaltest(residuals)

        alpha = 0.05
        null_hypothesis = "x comes from a normal distribution"
        if p < alpha:
            # not normally distributed
            UserWarning(f"The null hypothesis, {null_hypothesis}, can be rejected")
        else:
            print(f"The null hypothesis, {null_hypothesis}, cannot be rejected")


    def _evaluate(self, x):

        if isinstance(self.model, UniversalKriging):
            qoi_mean, qoi_std = self.model.execute(
                style="points",
                xpoints=x[:, 0].ravel(),
                ypoints=x[:, 1].ravel(),
            )
        elif isinstance(self.model, UniversalKriging3D):
            qoi_mean, qoi_std = self.model.execute(
                style="points",
                xpoints=x[:, 0].ravel(),
                ypoints=x[:, 1].ravel(),
                zpoints=x[:, 2].ravel(),
            )
        else:
            raise ValueError("Model must be of type UniversalKriging or UniversalKriging3D.")

        self.set_y_pred(qoi_mean)
        return qoi_mean, qoi_std

    def evaluate(self, x):
        mean, std = self._evaluate(x)
        return mean

if __name__ == "__main__":

    #TODO add class QuadratureModel
    def model_solver(q):
        return 5 + q[0] ** 1 + +0.1 * q[0] ** 2 + 0.4 * numpy.sin(q[0])


    container = list()

    # Generate parameter distribution
    sample_nr = 1000
    alpha = chaospy.Normal(1.5, 0.2)
    beta = chaospy.Uniform(0.1, 0.2)
    distribution = chaospy.J(alpha, beta)

    for order in [4]:
        print(f"Order PCE: {order}")
        abscissas, weights = chaospy.generate_quadrature(order, distribution, rule="gaussian")
        expansion = chaospy.orth_ttr(order, distribution)
        print(expansion)
        solves = numpy.array([model_solver(ab) for ab in abscissas.T])
        approx = chaospy.fit_quadrature(expansion, abscissas, weights, solves)
        qoi_dist = chaospy.QoI_Dist(approx, distribution, 1000)

        #
        mean = chaospy.E(approx, distribution)
        std = chaospy.Var(approx, distribution)**0.5

        xx = numpy.linspace(qoi_dist.lower, qoi_dist.upper, 20)
        kdepdf = qoi_dist.kernel.evaluate(xx.T)
        plt.plot(xx, kdepdf, '--', label=f"PC - KDE, order ={order}")

