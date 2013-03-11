import numpy
import time
import random
from numpy.lib.scimath import sqrt
import matplotlib.pyplot as plt
import pylab


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

    delta = network_output - parameter_v * activation + external_input * 0
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


def learn_patterns(patterns, n_neurons):
    weights = numpy.zeros((n_neurons, n_neurons), dtype=complex)
    patern_normalize = 1.0 / (2 * n_neurons)
    patern_normalize = 1.0
    for pattern in patterns:
        weights += numpy.dot(pattern.T, pattern)
    weights *= patern_normalize

#    for idx in range(len(weights)):
#        weights[idx][idx] = 0
    return weights


def get_distance_2d(loc_1, loc_2, dimension, type="euclidian"):
    """
    Return a distance between two nodes
    """
    if type == "euclidian":
        x_dis = get_distance_1d(loc_1[0], loc_2[0], dimension)
        y_dis = get_distance_1d(loc_1[1], loc_2[1], dimension)
        return numpy.sqrt(x_dis ** 2 + y_dis ** 2)

    raise NotImplementedError("Unknown distance metric supplied")

def get_distance_1d(x1, x2, dimension):
    """
    return distance in single dimension
    THIS FUNCTION INTRODUCES THE TORUS ASPECT OF THE GRID
    """
    return min(abs(x1 - x2), dimension - abs(x1 - x2))

def convert_distance_to_weigth_mutator(distance, max_distance,
                                       scaling=1.0, type="linear"):
    """
    converts a distance to a weighting factor
    type specifies the type of weighting used:
    linear
    gaussian 
    Needs the max_distance to calculate a correct factor. 
    the scaling allows scaling of the influence from full (1) to off(0) 
    """
    if distance > max_distance:
        Exception("received distance is larger then the max distance")
    if scaling > 1 or scaling < 0:
        Exception("got invalid value for the scaling factor, should be between 0 and one")

    if type == "linear":
        factor = float(distance) / max_distance

        #influence_distance = scaling * (1 - factor)
        correction = max(0, 1 - scaling * factor)
        return correction

    if type == "simple_mexican":
        fract_distance = (distance * 1.0) / max_distance
        correction = 0

        low_limit = 0.2
        high_limit = 0.4
        if fract_distance >= high_limit:
            correction = 0.0
        elif (fract_distance >= low_limit) and (fract_distance < high_limit):
            correction = -0.1
        else:
            correction = 0.45
        return correction

    raise NotImplementedError("Unknown weight mutator type retrieved")

def convert_neuron_index_to_position(index, n_neurons_in_side):
    """
    The neuron array does not have any notion of dimensions:
    The index need to be converted to a location.   
    THIS FUNCTION INTRODUCES THE GRID  
    """
    idx_row = index / n_neurons_in_side
    idx_column = index % n_neurons_in_side
    return (idx_row, idx_column)

def apply_weight_mutator_to_weight_matrix(n_neurons, weight_matrix, type="linear", scaling=1):
    """
    Aplies the weight mutator to all the value in the weight matrix.
    square torus world (no borders) assumed in the network.   
    mogelijk nog een bug: dus controleren
    """
    if not (numpy.sqrt(n_neurons) * numpy.sqrt(n_neurons) == n_neurons):
        raise Exception("Incorrect number of neurons retrieved: square network"
                        "is assumed: sqrt(n_neurons) is not a whole number")

    neuron_in_side = int(sqrt(n_neurons))
    max_distance = 0
    # Calculate the max distance ( brute force)
    for idx_second in range(1, n_neurons):
        neuron_1_position = convert_neuron_index_to_position(1, neuron_in_side)
        neuron_2_position = convert_neuron_index_to_position(idx_second, neuron_in_side)
        distance = get_distance_2d(neuron_1_position, neuron_2_position, neuron_in_side)
        if distance > max_distance:
            max_distance = distance

    for idx_first in range(n_neurons):
        if idx_first % 50:
            print "mutator:" , idx_first
        for idx_second in range(idx_first, n_neurons):
            neuron_1_position = convert_neuron_index_to_position(idx_first, neuron_in_side)
            neuron_2_position = convert_neuron_index_to_position(idx_second, neuron_in_side)
            distance = get_distance_2d(neuron_1_position, neuron_2_position, neuron_in_side)
            weight_mutator = convert_distance_to_weigth_mutator(distance, max_distance, scaling, type)
            weight_matrix[idx_first][idx_second] *= weight_mutator
            weight_matrix[idx_second][idx_first] *= weight_mutator


def slide_show_of_output(outputs_slices, n_side):
    x = numpy.arange(6)
    y = numpy.arange(5)
    z = x * y[:, numpy.newaxis]

    for idx in range(len(outputs_slices)):
    #for idx in range(20):
        slice = numpy.array(outputs_slices[idx])
        q_slice = numpy.reshape(slice, (n_side, n_side))

        q_slice += 2
        q_slice *= 10
        q_slice = numpy.array(q_slice, dtype=int)
        if idx == 0:
            p = plt.imshow(z)
            fig = plt.gcf()
            plt.clim()   # clamp the color limits
#            plt.title("Boring slide show")
            plt.title("Boring slide show")
        else:
            p.set_data(q_slice)

            plt.pause(0.0001)

def display_weight_matrix(weights):
    print weights
    print len(weights)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    cax = ax.imshow(numpy.abs(weights))#, interpolation='nearest')
    cbar = fig.colorbar(cax,)

    plt.show()


if __name__ == "__main__":
    n_neurons = 20 * 20
    n_side = int(round(sqrt(n_neurons)))
    n_patterns = 25
    parameter_v = numpy.complex(1.0, 0.0)
    #parameter_v = numpy.complex(0.5, 0.5)
    activation, weights, output = create_data_objects(n_neurons)
    patterns = generate_random_input(n_patterns, n_neurons)
    #weights = learn_patterns(patterns, n_neurons)
    weights = numpy.ones((n_neurons, n_neurons), dtype=complex)
    apply_weight_mutator_to_weight_matrix(n_neurons, weights, scaling=1.0, type="simple_mexican")
    display_weight_matrix(weights)

    #external_input = numpy.zeros((n_neurons), dtype=complex)[numpy.newaxis] #patterns[0]
    external_input = patterns[0]
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

        activation += 0.5 * delta
        #external_input = numpy.zeros((n_neurons), dtype=complex)[numpy.newaxis]
        external_input = numpy.random.random((n_neurons)) * 0.5


    t = numpy.arange(0, len(outputs[0]))
#    p = pylab.imshow(outputs_slices[0])
#    fig = pylab.gcf()
    for idx in range(n_neurons):
        pylab.plot(t, outputs[idx])
#
    pylab.show()
    slide_show_of_output(outputs_slices, n_side)

        #ax.imshow(pylab.real(q_slice))

