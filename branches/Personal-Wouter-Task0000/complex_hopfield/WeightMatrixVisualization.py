import WeightMatrixVisualizationUtils as wm_utils
import numpy

class WeightMatrixVisualization(object):
    def __init__(self, weightMatrix):
        """
        Initializes the class with a weight matrix, creates a private copy
        """
        self._weightMatrix = weightMatrix.copy()


    def _perform_horizontal_swap(self, first, second):
        """
        Performs a swap of first and second weight matrix lines,
        """
        if first > len(self._weightMatrix) or second > len(self._weightMatrix):
            raise Exception("one of the swap indexes is larger than the "
                            "data matrix dimensions")
        line = self._weightMatrix[first, :].copy()
        self._weightMatrix[first, :] = self._weightMatrix[second, :]
        self._weightMatrix[second, :] = line

    def _perform_vertical_swap(self, first, second):
        if first > len(self._weightMatrix) or second > len(self._weightMatrix):
            raise Exception("one of the swap indexes is larger than the "
                            "data matrix dimensions")
        line = self._weightMatrix[:, first].copy()
        self._weightMatrix[:, first] = self._weightMatrix[:, second]
        self._weightMatrix[:, second] = line

    def _perform_matrix_swap(self, first, second):
        if first > len(self._weightMatrix) or second > len(self._weightMatrix):
            raise Exception("one of the swap indexes is larger than the "
                            "data matrix dimensions")
        self._perform_horizontal_swap(first, second)
        self._perform_vertical_swap(first, second)

    def sort_matrix_on_direction(self):
        """
        Most simple sorter implementation
        """
        n_neurons = len(self._weightMatrix)
        for idx in range(n_neurons - 1):
            weights = numpy.sum(numpy.abs(self._weightMatrix[:, idx:]), axis=1)
            max_index = numpy.argmax(weights[idx:]) + idx # add the start index
                        #of the sub step
            self._perform_matrix_swap(max_index, idx)



    def get_matrix(self):
        """
        Return the internal weightmatrix in its current form
        """
        return self._weightMatrix



if __name__ == "__main__":
    # perform a self test of the functionality

    n_neurons = 6
    wmatrix = wm_utils._create_a_random_numpy_weight_matrix(n_neurons)
    for idx in range(1, n_neurons):
        wmatrix[idx, idx:] = 0
    print wmatrix
    weights = WeightMatrixVisualization(wmatrix)
    weights.sort_matrix_on_direction()
    print weights.get_matrix()

