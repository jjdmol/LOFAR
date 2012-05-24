from __future__ import with_statement
import os
import unittest
import tempfile
import sys
import shutil
import numpy

from argparse import ArgumentTypeError

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.support.lofarexceptions import PipelineRecipeFailed
from lofarpipe.recipes.nodes.parmexportcal import ParmExportCal
from lofarpipe.recipes.helpers.ComplexArray import ComplexArray, RealImagArray, AmplPhaseArray
from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB
#import from fixtures:
from logger import logger

class ParmExportCalWrapper(ParmExportCal):
    """
    The test wrapper allows overwriting of function with muck functionality
    """
    def __init__(self):
        """
        """
        self.logger = logger()

class ParmExportCalTest(unittest.TestCase):
    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ParmExportCalTest, self).__init__(arg)

    def setUp(self):
        self.tempDir = tempfile.mkdtemp()


    def tearDown(self):
        shutil.rmtree(self.tempDir)

    def test_convert_data_to_ComplexArray_real_imag(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["Imag", "Real"]  # Order is alphabetical
        parmExportCal = ParmExportCalWrapper()
        complex_array = parmExportCal._convert_data_to_ComplexArray(data, type_pair)

        goal_array = RealImagArray([1], [1])
        self.assertTrue(complex_array.real == goal_array.real)
        self.assertTrue(complex_array.imag == goal_array.imag)

    def test_convert_data_to_ComplexArray_amp_phase(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["Ampl", "Phase"]  # Order is alphabetical
        parmExportCal = ParmExportCalWrapper()
        complex_array = parmExportCal._convert_data_to_ComplexArray(data, type_pair)

        goal_array = AmplPhaseArray([1], [1])
        self.assertTrue(complex_array.amp == goal_array.amp)
        self.assertTrue(complex_array.phase == goal_array.phase)

    def test_convert_data_to_ComplexArray_incorrect_pair(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["spam", "spam"]  # Order is alphabetical
        parmExportCal = ParmExportCalWrapper()

        self.assertRaises(PipelineRecipeFailed,
                          parmExportCal._convert_data_to_ComplexArray,
                          data, type_pair)

    def test_write_corrected_data(self):
        # define input data
        name = "test"
        station = "station"
        parmExportCal = ParmExportCalWrapper()
        input_polarization_data = {"pol1":[{'freqs':[11],
                                           'freqwidths':[12],
                                           'times':[13],
                                           'timewidths':[14]}]}

        input_corected_data = {"pol1":RealImagArray([[1], [1]], [[2], [2]]),
                 "pol22":RealImagArray([[3], [3]], [[4], [4]])}
        # This object will be taken from the fixture: it is a recorder muck
        parmdb = WritableParmDB("parmdb")

        # call function
        parmExportCal = ParmExportCalWrapper()
        parmExportCal._write_corrected_data(parmdb, station,
                            input_polarization_data, input_corected_data)

        # test output: (the calls to parmdb)
        # there is one polarization, containing a single complex array
        # when writing this should result in, 1 times 2 function calls
        # first delete the REAL entry
        expected = ['deleteValues', ['Gain:pol1:Real:station']]
        self.assertTrue(parmdb.called_functions_and_parameters[0] == expected,
                        "result({0}) != expected({1})".format(
                        parmdb.called_functions_and_parameters[0], expected))
        # then the new values should be added, with the correct values
        expected = ['addValues', ['Gain:pol1:Real:station',
                                  numpy.array([[1.], [1.]],),
                        11, 11 + 12, 13, 13 + 2 * 14, False]] #stat + steps*size 

        # Now scan the argument array: for numpy use special compare function
        for left, right in zip(parmdb.called_functions_and_parameters[1][1],
                                expected[1]):
            error_message = "\nresult({0}) != \nexpected({1}) \n"\
                "-> {2} !=  {3}".format(
                        parmdb.called_functions_and_parameters[1], expected,
                        left, right)
            try:
                if not left == right:
                    self.assertTrue(False, error_message)
            except ValueError:
                if not numpy.array_equal(left, right):
                    self.assertTrue(False, error_message)

        # now delete the imag entry: Rememder these are on the 2nd and 3th array
        # position
        expected = ['deleteValues', ['Gain:pol1:Imag:station']]
        self.assertTrue(parmdb.called_functions_and_parameters[2] == expected,
                        "result({0}) != expected({1})".format(
                        parmdb.called_functions_and_parameters[2], expected))
        # then the new values should be added, with the correct values
        expected = ['addValues', ['Gain:pol1:Imag:station',
                                  numpy.array([[2.], [2.]],),
                        11, 11 + 12, 13, 13 + 2 * 14, False]] #stat + steps*size 

        # Now scan the argument array: for numpy use special compare function
        for left, right in zip(parmdb.called_functions_and_parameters[3][1],
                                expected[1]):
            error_message = "\nresult({0}) != \nexpected({1}) \n"\
                "-> {2} !=  {3}".format(
                        parmdb.called_functions_and_parameters[3], expected,
                        left, right)
            try:
                if not left == right:
                    self.assertTrue(False, error_message)
            except ValueError:
                if not numpy.array_equal(left, right):
                    self.assertTrue(False, error_message)

    def test_write_corrected_data_does_not_contain_pol(self):
        name = "test"
        station = "station"
        parmExportCal = ParmExportCalWrapper()
        input_polarization_data = {"unknownPolarisation":[{'freqs':[11],
                                           'freqwidths':[12],
                                           'times':[13],
                                           'timewidths':[14]}]}

        input_corected_data = {"pol1":RealImagArray([[1], [1]], [[2], [2]]),
                 "pol2":RealImagArray([[3], [3]], [[4], [4]])}
        # This object will be taken from the fixture: it is a recorder muck
        parmdb = WritableParmDB("parmdb")

        # call function
        parmExportCal = ParmExportCalWrapper()
        self.assertRaises(PipelineRecipeFailed,
                          parmExportCal._write_corrected_data,
                          parmdb, station,
                          input_polarization_data, input_corected_data)


    def test_swap_outliers_with_median(self):
        data = {"pol1":[{"values": [1., 1., 1., 1., 100., 100.]},
                        {"values": [1., 1., 1., 1., 100., 100.]}]
                 }
        type_pair = ["Imag", "Real"]  # Order is alphabetical

        # omit the last entry do swap the 5th entry with the median (1)
        goal_filtered_array = numpy.array([1., 1., 1., 1., 1., 100.])
        parmExportCal = ParmExportCalWrapper()
        corrected_polarisation = \
            parmExportCal._swap_outliers_with_median(data, type_pair, 2.0)

        #incredibly rough and incorrect float comparison of the values in the 
        for left, right in zip(corrected_polarisation['pol1'].real, goal_filtered_array):
            message = "Comparison of float values in the array did not" \
                    "result in about the same value: {0}"
            if not int(left) == int(right):
                self.assertTrue(False, message.format(
                    "int value not the same: "
                    "{0} !=  {1}".format(int(left), int(right))))
            precision = 1000
            if not int(left * precision) == int(right * precision):
                self.assertTrue(False, message.format(
                    "value not the same within current precision: "
                    "{0} !=  {1}".format(int(left * precision), int(right * precision))))

