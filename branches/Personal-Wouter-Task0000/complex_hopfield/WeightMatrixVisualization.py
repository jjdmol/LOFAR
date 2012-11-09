import WeightMatrixVisualizationUtils as wm_utils
import numpy


class WeightMatrixVisualization(object):
    """
    Collects a number of functions to display weight matrixes
    """
    def __init__(self, weightMatrix):
        """
        Initializes the class with a weight matrix, creates a private copy
        """
        self._weightMatrix = weightMatrix.copy()
        self._order = range(len(self._weightMatrix))

    def _perform_horizontal_swap(self, first, second):
        """
        Performs a swap of first and second weight matrix lines, rowwise
        Should always be followed or preceded by a vertical swap!!!
        """
        self._weightMatrix[first, :], self._weightMatrix[second, :] = \
            self._weightMatrix[second, :].copy(), \
                self._weightMatrix[first, :].copy()

    def _perform_vertical_swap(self, first, second):
        """
        Performs a swap of first and second weight matrix lines, columnwise
        Should always be followed or preceded by a horizontal swap!!!
        """
        self._weightMatrix[:, first], self._weightMatrix[:, second] = \
            self._weightMatrix[:, second].copy(), \
                self._weightMatrix[:, first].copy()

    def _perform_matrix_swap(self, first, second):
        """
        Swap two entrie in a matrix both horizontal as vertical.
        Update the _order to reflect the changed order

        Validate inputs indexes
        """
        if first > len(self._weightMatrix) or second > len(self._weightMatrix):
            raise Exception("one of the swap indexes is larger than the "
                            "data matrix dimensions" + str())
        self._perform_horizontal_swap(first, second)
        self._perform_vertical_swap(first, second)
        # now swap the entries in the _order list
        self._order[first], self._order[second] = self._order[second], \
            self._order[first]

    def sort_matrix_on_direction(self):
        """
        Sort matrix in such a way that feedforward/backwards
        is pronounced
        """
        n_neurons = len(self._weightMatrix)
        # loop all neurons not last (cant swap this with any other neuron)
        for idx in range(n_neurons - 1):
            weights = numpy.sum(numpy.abs(self._weightMatrix[:, idx:]), axis=1)

            # add the start index of the sub step
            max_index = numpy.argmax(weights[idx:]) + idx
            self._perform_matrix_swap(max_index, idx)

    def sort_matrix_on_weight_equality(self):
        """
        sort matrix in such a way that equality between neuron connectity is
        pronounced
        """
        n_neurons = len(self._weightMatrix)
        # Skip the first neuron: We are comparing with him
        # We are switching the most like and the least like matches with the
        # seed neuron: We need to skip the seed and two two skips in the loop
        for idx in range(1, n_neurons - 1):
            # Calculate equality/correlation in two step:
            # first the rows
            horizontal_distance_to_seed = numpy.sum(
                # get the absoluut of the difference_total
                numpy.abs(
                # get the difference_total between each row
                    numpy.subtract(
                        self._weightMatrix[idx:, idx:],
                        # and  the seed neuron
                        self._weightMatrix[idx - 1, idx:])), axis=1)

            # Now the column difference_total
            transposed_weights = self._weightMatrix.T
            vertical_distance_to_seed = numpy.sum(
                # get the absoluut of the difference_total
                numpy.abs(
                    # get the difference_total between each column
                    numpy.subtract(
                        transposed_weights[idx:, idx:],
                        # and the seed neuron
                        transposed_weights[idx - 1, idx:])), axis=1)

            difference_total = vertical_distance_to_seed + \
                horizontal_distance_to_seed

            # use argsort to get the sorted indexes,
            #  sorted_indexes[0] == smallest, sorted_indexes[-1] == largest
            sorted_indexes = numpy.argsort(difference_total)
            # swap the neuron that looks the most like the seed to next to the
            # current neuron. Remember to add the idx!!!
            self._perform_matrix_swap(sorted_indexes[0] + idx, idx)

    def get_matrix(self):
        """
        Return the internal weightmatrix in its current form
        """
        return self._weightMatrix

    def apply_sorting_to_matrix(self, target_order):
        """
        Applies the order supplied in target_order to the weight matrix by
        applying consecutive swaps.
        The matrix is ordered based on the indexes of the original input matrix
        Allowing chained sorting!
        """
        if len(target_order) != len(self._weightMatrix):
            raise Exception("supplied ordering is of a different dimension"
                            "then the internal weight matrix" +
                            str(target_order) + " " + str(self._weightMatrix))

        for idx in range(len(target_order)):
            # find the index in the current weight matrix order
            target_index = None
            for idy in range(idx, len(target_order)):
                if  self._order[idy] == target_order[idx]:
                    target_index = idy
                    break

            self._perform_matrix_swap(idx, target_index)



if __name__ == "__main__":
    # perform a self test of the functionality
#
#    n_neurons = 6
#    wmatrix = wm_utils._create_a_random_numpy_weight_matrix(n_neurons)
#    for idx in range(1, n_neurons):
#        wmatrix[idx, idx:] = 0
#    print wmatrix
#    weights_ret = WeightMatrixVisualization(wmatrix)
#    weights_ret.sort_matrix_on_direction()
#    print weights_ret.get_matrix()

    pass
