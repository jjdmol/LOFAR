import numpy
import time
import random
from numpy.lib.scimath import sqrt
import matplotlib.pyplot as plt
import pylab
from helpers.learn_weights import *
from helpers.visualize import *

def create_data_objects(n_neurons):
    activation = numpy.ones((n_neurons), dtype=complex)[numpy.newaxis]
    weights = numpy.ones((n_neurons, n_neurons), dtype=complex)
    output = numpy.ones((n_neurons), dtype=complex)[numpy.newaxis]
    return activation, weights, output

def calculate_output_from_activation(activation, output, parameter_v,
                                    steepness=1.2):
    numpy.tanh(steepness *
               parameter_v *
               numpy.ma.conjugate(activation), output)
    return output

def calculate_delta(weights, output, activation, external_input, parameter_v):
    network_output = numpy.dot(weights, output.T).T

    delta = network_output - parameter_v * activation + external_input

    return delta

def generate_random_input(n_patterns, n_neurons):
    patterns = []
    for idx in range(n_patterns):
        pattern = numpy.ones((n_neurons), dtype=complex)
        for idx_neuron in range(n_neurons):
            if random.random() > 0.5:
                pattern[idx_neuron] = numpy.complex(-1)
        patterns.append(pattern[numpy.newaxis])

    return patterns




if __name__ == "__main__":

    n_neurons = 40 * 40
    n_side = int(round(sqrt(n_neurons)))
    n_patterns = 3
    parameter_v = numpy.complex(1, 0)
    activation, weights, output = create_data_objects(n_neurons)
    patterns = generate_random_input(n_patterns, n_neurons)
    weights = learn_patterns(patterns, n_neurons)
    weights = numpy.ones((n_neurons, n_neurons), dtype=complex)
    apply_weight_mutator_to_weight_matrix(n_neurons, weights, scaling=2.0, type="simple_mexican")
    display_weight_matrix(weights)

    #external_input = numpy.zeros((n_neurons), dtype=complex)[numpy.newaxis] #patterns[0]
    external_input = patterns[0] # ones
    steepness = 1.2
    delta = None
    outputs = []
    outputs_slices = []
    for idx in range(n_neurons):
        outputs.append([])

    t1 = time.time()
    for idx in range(2000):
        print idx
        output = calculate_output_from_activation(activation, output, parameter_v, steepness)
        slice = []
        for idx_n in range(n_neurons):
            outputs[idx_n].append(output[0][idx_n])
            slice.append(output[0][idx_n])
        outputs_slices.append(numpy.real(slice))

        delta = calculate_delta(weights, output, activation, external_input, parameter_v)

        activation += 0.05 * delta
        external_input = numpy.zeros((n_neurons), dtype=complex)[numpy.newaxis]
        #external_input = ones times noisy input
        # noise moet altijd in de vorm vorm echte input??


    t = numpy.arange(0, len(outputs[0]))

    for idx in range(n_neurons):
        pylab.plot(t, outputs[idx])

    pylab.show()
    slide_show_of_output(outputs_slices, n_side)
