import numpy


def _create_a_random_numpy_weight_matrix(n_neurons, type=None):
    if type == 'complex':
        re = numpy.random.random((n_neurons, n_neurons))
        im = numpy.random.random((n_neurons, n_neurons))
        return re + im * 1j
    else:
        return numpy.random.random((n_neurons, n_neurons))
