from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile

from WeightMatrixVisualization import WeightMatrixVisualization


class sutWrapper(WeightMatrixVisualization):
    """
    Wrapper for the System Under Test.
    Allows mucking and stubbing of functions and datamembers
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on
        LOFARnodeTCP.
        """
        pass


class WeightMatrixVisualizationTests(unittest.TestCase):
    def __init__(self, arg):
        super(WeightMatrixVisualizationTests, self).__init__(arg)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_hello_world(self):
        self.assertTrue(True, "Hello World")

    def test_perform_horizontal_swap(self):
        input_data = numpy.array([[1, 2, 3], [4, 5, 6], [6, 7, 8]])

        sut = WeightMatrixVisualization(input_data)
        sut._perform_horizontal_swap(1, 2)
        result = sut.get_matrix()

        target = numpy.array([[1, 2, 3], [6, 7, 8], [4, 5, 6]])
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0, "horizontal swap failed")

    def test_perform_vertical_swap(self):
        input_data = numpy.array([[1, 2, 3], [4, 5, 6], [6, 7, 8]])

        sut = WeightMatrixVisualization(input_data)
        sut._perform_vertical_swap(1, 2)
        result = sut.get_matrix()

        target = numpy.array([[1, 3, 2], [4, 6, 5], [6, 8, 7]])
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0, "vertical swap failed")

    def test_perform_matrix_swap(self):
        input_data = numpy.array([[1, 2, 3], [4, 5, 6], [6, 7, 8]])

        sut = WeightMatrixVisualization(input_data)
        sut._perform_matrix_swap(1, 2)
        result = sut.get_matrix()

        target = numpy.array([[1, 3, 2], [6, 8, 7], [4, 6, 5]])
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0, "matrix swap failed")


    def test_sort_matrix_on_feedforward(self):
        input_data = numpy.array([[1, 2, 3],
                                  [4, 0, 0],
                                  [7, 8, 0]])

        target = numpy.array([[0, 8, 7],
                              [0, 0, 4],
                              [3, 2, 1]])

        sut = WeightMatrixVisualization(input_data)
        sut.sort_matrix_on_direction()
        result = sut.get_matrix()

        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "matrix sort failed")

    def test_sort_matrix_on_weight_equality(self):
        input_data = numpy.array([[2, 0, 2, 2, 0, 0],
                                  [0, 2, 0, 0, 2, 2],
                                  [2, 0, 2, 2, 0, 0],
                                  [2, 0, 2, 2, 0, 0],
                                  [0, 2, 0, 0, 2, 2],
                                  [0, 2, 0, 0, 2, 2]])

        target = numpy.array([[2, 2, 2, 0, 0, 0],
                              [2, 2, 2, 0, 0, 0],
                              [2, 2, 2, 0, 0, 0],
                              [0, 0, 0, 2, 2, 2],
                              [0, 0, 0, 2, 2, 2],
                              [0, 0, 0, 2, 2, 2]])


        sut = WeightMatrixVisualization(input_data)
        sut.sort_matrix_on_weight_equality()
        result = sut.get_matrix()

        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "\n" + str(result))
        # compare the equality of the sorting order (the new indexes)
        target = numpy.array([0, 2, 3, 1, 4, 5])
        result = sut._order
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "\n" + str(result))

    def test_sort_matrix_on_weight_2d_symetric(self):
        # currently the best effort for symetry..
        input_data = numpy.array([[2, 0, 0, 0, 0, 1, 1],
                                  [0, 2, 0, 1, 0, 0, 1],
                                  [0, 0, 2, 0, 1, 1, 0],
                                  [0, 1, 0, 2, 1, 0, 0],
                                  [0, 0, 1, 1, 2, 0, 0],
                                  [1, 0, 1, 0, 0, 2, 0],
                                  [1, 1, 0, 0, 0, 0, 2]])

        target = numpy.array([[2, 1, 0, 0, 0, 0, 1],
                              [1, 2, 1, 0, 0, 0, 0],
                              [0, 1, 2, 1, 0, 0, 0],
                              [0, 0, 1, 2, 1, 0, 0],
                              [0, 0, 0, 1, 2, 1, 0],
                              [0, 0, 0, 0, 1, 2, 1],
                              [1, 0, 0, 0, 0, 1, 2]])

        sut = WeightMatrixVisualization(input_data)
        sut.sort_matrix_on_weight_equality()
        result = sut.get_matrix()

        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "\n" + str(result))

    def test_apply_sorting_to_matrix(self):
        input_data = numpy.array([[1, 2, 3, 4],
                                  [5, 6, 7, 8],
                                  [9, 10, 11, 12],
                                  [13, 14, 15, 16]])

        target = numpy.array([[16, 15, 14, 13],
                              [12, 11, 10, 9],
                              [ 8, 7, 6, 5],
                              [ 4, 3, 2, 1]])
        target_sorting = [3, 2, 1, 0]
        sut = WeightMatrixVisualization(input_data)

        sut.apply_sorting_to_matrix(target_sorting)
        result = sut.get_matrix()
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "apply matrix sorting failed")
        self.assertTrue(target_sorting == sut._order, str("neuron _order not"
                                        " correct after applying sort!!"))

if __name__ == "__main__":
    unittest.main()
