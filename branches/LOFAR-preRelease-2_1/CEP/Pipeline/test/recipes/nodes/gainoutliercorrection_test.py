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
from lofarpipe.recipes.nodes.gainoutliercorrection import gainoutliercorrection
from lofarpipe.recipes.helpers.ComplexArray import ComplexArray, RealImagArray, AmplPhaseArray
from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB
#import from fixtures:
from logger import logger

class GainOutlierCorrectionWrapper(gainoutliercorrection):
    """
    The test wrapper allows overwriting of function with muck functionality
    """
    def __init__(self):
        """
        """
        self.logger = logger()

class GainOutlierDetectionTest(unittest.TestCase):
    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(GainOutlierDetectionTest, self).__init__(arg)

    def setUp(self):
        self.tempDir = tempfile.mkdtemp()


    def tearDown(self):
        shutil.rmtree(self.tempDir)

    def test_convert_data_to_ComplexArray_real_imag(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["Imag", "Real"]  # Order is alphabetical
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        complex_array = GainOutlierDetection._convert_data_to_ComplexArray(data, type_pair)

        goal_array = RealImagArray([1], [1])
        self.assertTrue(complex_array.real == goal_array.real)
        self.assertTrue(complex_array.imag == goal_array.imag)

    def test_convert_data_to_ComplexArray_amp_phase(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["Ampl", "Phase"]  # Order is alphabetical
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        complex_array = GainOutlierDetection._convert_data_to_ComplexArray(data, type_pair)

        goal_array = AmplPhaseArray([1], [1])
        self.assertTrue(complex_array.amp == goal_array.amp)
        self.assertTrue(complex_array.phase == goal_array.phase)

    def test_convert_data_to_ComplexArray_incorrect_pair(self):
        data = [{"values": [1]}, {"values": [1]}]
        type_pair = ["spam", "spam"]  # Order is alphabetical
        GainOutlierDetection = GainOutlierCorrectionWrapper()

        self.assertRaises(PipelineRecipeFailed,
                          GainOutlierDetection._convert_data_to_ComplexArray,
                          data, type_pair)

    def test_write_corrected_data(self):
        # define input data
        name = "test"
        station = "station"
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        input_polarization_data = {"pol1":[{'freqs':[11],
                                           'freqwidths':[12],
                                           'times':[13],
                                           'timewidths':[14]}]}

        input_corected_data = {"pol1":RealImagArray([[1], [1]], [[2], [2]]),
                 "pol22":RealImagArray([[3], [3]], [[4], [4]])}
        # This object will be taken from the fixture: it is a recorder muck
        parmdb = WritableParmDB("parmdb")

        # call function
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        GainOutlierDetection._write_corrected_data(parmdb, station,
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
                        11, 11 + 12, 13, 13 + 2 * 14, True]] #stat + steps*size 

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
                        11, 11 + 12, 13, 13 + 2 * 14, True]] #stat + steps*size 

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
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        input_polarization_data = {"unknownPolarisation":[{'freqs':[11],
                                           'freqwidths':[12],
                                           'times':[13],
                                           'timewidths':[14]}]}

        input_corected_data = {"pol1":RealImagArray([[1], [1]], [[2], [2]]),
                 "pol2":RealImagArray([[3], [3]], [[4], [4]])}
        # This object will be taken from the fixture: it is a recorder muck
        parmdb = WritableParmDB("parmdb")

        # call function
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        self.assertRaises(PipelineRecipeFailed,
                          GainOutlierDetection._write_corrected_data,
                          parmdb, station,
                          input_polarization_data, input_corected_data)


    def test_swap_outliers_with_median(self):
        data = {"pol1":[{"values": [1., 1., 1., 1., 100., 100.]},
                        {"values": [1., 1., 1., 1., 100., 100.]}]
                 }
        type_pair = ["Imag", "Real"]  # Order is alphabetical

        # omit the last entry do swap the 5th entry with the median (1)
        goal_filtered_array = numpy.array([1., 1., 1., 1., 1., 100.])
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        corrected_polarisation = \
            GainOutlierDetection._swap_outliers_with_median(data, type_pair, 2.0)

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


    def test_swap_outliers_with_median_within_3_sigma(self):
        data = {"pol1":[{"values": [1., 1., 1., 1., 100., 100.]},
                        {"values": [1., 1., 1., 1., 100., 100.]}]
                 }
        type_pair = ["Imag", "Real"]  # Order is alphabetical

        # omit the last entry do swap the 5th entry with the median (1)
        goal_filtered_array = numpy.array([1., 1., 1., 1., 100., 100.])
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        corrected_polarisation = \
            GainOutlierDetection._swap_outliers_with_median(data, type_pair, 3.0) # Sigma three!!

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


    def test_read_polarisation_data_and_type_from_db(self):
        #create a muck parmdb
        parmdb = WritableParmDB("parmdb")
        parmdb.names = ["1:1:Real:name1",
                      "1:1:Real:name2",
                      "1:1:Real:name3",
                      "1:1:Real:name4",
                      "Gain:1:1:Real:test",
                      "Gain:1:1:Imag:test",
                      "Gain:0:0:Real:test",
                      "Gain:0:0:Imag:test"]

        station = "test"

        #create sut
        GainOutlierDetection = GainOutlierCorrectionWrapper()
        (retrieved_data, type_pair) = GainOutlierDetection._read_polarisation_data_and_type_from_db(parmdb,
                                        station)

        #validate output!!
        value_dict = {"values":[[1., 1., 1., 1., 100., 100.], [1., 1., 1., 1., 100., 100.]],
                          'freqs':[2],
                          'freqwidths':[2],
                          'times':[2],
                          'timewidths':[2]}
        goal_retrieved_data = {'1:1': [value_dict, value_dict],
                                '0:0': [value_dict, value_dict]}

        self.assertTrue(retrieved_data == goal_retrieved_data,
                         "Incorrect data retrieved from the parmdb: {0}".format(
                                      retrieved_data))
        goal_type_pair = ['Imag', 'Real']
        self.assertTrue(type_pair == goal_type_pair,
                         "Incorrect data retrieved from the parmdb: {0}".format(
                                      retrieved_data))


    def test_read_polarisation_data_and_type_from_db_invalid_type_pair(self):
        #create a muck parmdb
        parmdb = WritableParmDB("parmdb")
        parmdb.names = ["1:1:Real:name1",
                      "1:1:Real:name2",
                      "1:1:Real:name3",
                      "1:1:Real:name4",
                      "Gain:1:1:Real:test",
                      "Gain:1:1:Imag:test",
                      "Gain:0:0:incorrect_type:test",
                      "Gain:0:0:Imag:test"]

        station = "test"

        GainOutlierDetection = GainOutlierCorrectionWrapper()

        # unknown datatype should throw an exception
        self.assertRaises(PipelineRecipeFailed, GainOutlierDetection._read_polarisation_data_and_type_from_db,
                          parmdb, station)


    def test_filter_stations_parmdb_unexisting_infile(self):
        unexisting_file = os.path.join(self.tempDir, "name")
        unexisting_file2 = os.path.join(self.tempDir, "name2")

        GainOutlierDetection = GainOutlierCorrectionWrapper()
        self.assertRaises(PipelineRecipeFailed,
                          GainOutlierDetection._filter_stations_parmdb,
                           unexisting_file, unexisting_file2, "1.0")

    def test_filter_stations_parmdb(self):
        file_path_in = os.path.join(self.tempDir, "input")
        create_directory(file_path_in)

        file_path_out = os.path.join(self.tempDir, "fullName")

        GainOutlierDetection = GainOutlierCorrectionWrapper()

        # Call the major  function
        # No errors should be thrown...
        parmdb = GainOutlierDetection._filter_stations_parmdb(file_path_in,
                                    file_path_out, 2)

#        expected_calls_to_parmdb = [['getValuesGrid', ['Gain:1:1:Imag:test']], #get the four entries for a station
#                                    ['getValuesGrid', ['Gain:1:1:Real:test']],
#                                    ['getValuesGrid', ['Gain:0:0:Imag:test']],
#                                    ['getValuesGrid', ['Gain:0:0:Real:test']],
#                                    ['deleteValues', ['Gain:1:1:Real:test']], #delete orig value
#                                    ['addValues', ['Gain:1:1:Real:test', #insert new value
#                                        numpy.array([[   1., 1., 1., 1., 1., 1.], # new filtered values
#                                               [   1., 1., 1., 1., 100., 100.]]), # skip last timestep (also the orig value)
#                                                    2, 14, 2, 6, False]], #Some stats needed for writing should be this value
#                                    ['deleteValues', ['Gain:1:1:Imag:test']],
#                                    ['addValues', ['Gain:1:1:Imag:test',
#                                        numpy.array([[   1., 1., 1., 1., 1., 1.],
#                                               [   1., 1., 1., 2000., 100., 100.]]),
#                                             2, 14, 2, 6, False]],
#                                    ['deleteValues', ['Gain:0:0:Real:test']],
#                                    ['addValues', ['Gain:0:0:Real:test',
#                                        numpy.array([[   1., 1., 1., 1., 1., 1.],
#                                               [   1., 1., 1., 1., 100., 100.]]),
#                                                 2, 14, 2, 6, False]],
#                                    ['deleteValues', ['Gain:0:0:Imag:test']],
#                                    ['addValues', ['Gain:0:0:Imag:test',
#                                        numpy.array([[   1., 1., 1., 1., 1., 1.],
#                                               [   1., 1., 1., 1., 100., 100.]]),
#                                                 2, 14, 2, 6, False]]]
#
#        # there is a bug in this compare mechanism..
#        # Think about building a generalized comparer... Longterm goal
#        for left, right in zip(parmdb.called_functions_and_parameters, expected_calls_to_parmdb):
#            error_message = "\nresult({0}) != \nexpected({1}) \n"\
#                "-> {2} !=  {3}".format(
#                        parmdb.called_functions_and_parameters, expected_calls_to_parmdb,
#                        left, right)
#            try:
#                if not left == right:
#                    error_message = "{0} {1}".format(left, right)
#                    self.assertTrue(False, error_message)
#            except ValueError:
#
#                for left_entry, right_entry in zip(left, right):
#                    if isinstance(left_entry, basestring):
#                        continue
#                    for left_array, right_array in zip(left_entry, right_entry):
#                        message = "Comparison of float values in the array did not" \
#                                    "result in about the same value: {0}"
#                        if isinstance(left_array, basestring):
#                            continue
#                        if isinstance(left_array, int):
#                            continue
#                        #self.assertTrue(False, "{0} {1}".format(left_array[0], right_array[0]))
#                        for left_digit, right_digit in zip(list(left_array[0]), list(right_array[0])):
#                            if not int(left_digit) == int(right_digit):
#                                 self.assertTrue(False, message.format(
#                                        "int value not the same: "
#                                        "{0} !=  {1}".format(int(left_digit), int(right_digit))))
#                            precision = 1000
#                            if not int(left_digit * precision) == int(right_digit * precision):
#                                    self.assertTrue(False, message.format(
#                                        "value not the same within current precision: "
#                                        "{0} !=  {1}".format(int(left_digit * precision), int(right_digit * precision))))
#

