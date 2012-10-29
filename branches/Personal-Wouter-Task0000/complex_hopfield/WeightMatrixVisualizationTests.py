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


    def test_sort_matrix_on_weight_function(self):
        input_data = numpy.array([[1, 2, 3], [4, 0, 0], [7, 8, 0]])

        sut = WeightMatrixVisualization(input_data)
        sut.sort_matrix_on_direction()
        result = sut.get_matrix()
        target = numpy.array([[0, 8, 7], [0, 0, 4], [3, 2, 1]])
        self.assertTrue(numpy.sum(numpy.abs(result - target)) == 0,
                         "matrix sort failed")

if __name__ == "__main__":
    unittest.main()
