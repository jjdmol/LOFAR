from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile

#imports from fixture:
import pyrap.tables as tb                                                       #@UnresolvedImport
import monetdb.sql as db
import gsmutils as gsm
from logger import logger


from support.utilities import create_directory                        #@UnresolvedImport
from nodes.imager_create_dbs import imager_create_dbs         #@UnresolvedImport



class ImagerCreateDBsTestWrapper(imager_create_dbs):
    """
    Wrapper for the imager_create_dbs allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()
        self.db = db
        self.gsm = gsm

        self.outputs = {}
        self.outputs['parmdbs'] = "default value, uncalled"
        self.outputs['sky'] = "default value, uncalled"

class ImagerCreateDBsTest(unittest.TestCase):
    """
    Tests for ImagerCreateDBs class 
    """
    test_path = "/data/scratch/python_unittest"

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ImagerCreateDBsTest, self).__init__(arg)

    def setUp(self):
        self.imager_create_dbs = ImagerCreateDBsTestWrapper()
        create_directory(self.test_path)

    def tearDown(self):
        shutil.rmtree(self.test_path)


    def test_field_of_view_HBA_120_CS(self):
        """
        Test the calcultaion of the FOV for lowest freq on a hba core station 
        """

        variable_dictionary = {'NAME':["CS--HBA--"],
                               'REF_FREQUENCY':["120E6"]}
        tb.table.variable_dictionary = variable_dictionary
        fov = self.imager_create_dbs._field_of_view("MS_name")
        self.assertAlmostEqual(fov, 3.02, 2, "Incorrect FOV Value")


    def test_field_of_view_HBA_240_RS(self):
        """
        Test the calcultaion of the FOV for lowest freq on a hba core station 
        """
        variable_dictionary = {'NAME':["RS--HBA--"],
                               'REF_FREQUENCY':["240E6"]}
        tb.table.variable_dictionary = variable_dictionary
        fov = self.imager_create_dbs._field_of_view("MS_name")
        self.assertAlmostEqual(fov, 1.13, 2, "Incorrect FOV Value")

    def test_field_of_view_LBA_15_INNER(self):
        """
        Test the calcultaion of the FOV for lowest freq on a hba core station 
        """
        variable_dictionary = {'NAME':["--LBA--"],
                               'REF_FREQUENCY':["15E6"],
                               'LOFAR_ANTENNA_SET':["--INNER--"]}
        tb.table.variable_dictionary = variable_dictionary
        fov = self.imager_create_dbs._field_of_view("MS_name")
        self.assertAlmostEqual(fov, 23.04, 2, "Incorrect FOV Value")


    def test_field_of_view_LBA_75_OUTER(self):
        """
        Test the calcultaion of the FOV for lowest freq on a hba core station 
        """
        variable_dictionary = {'NAME':["--LBA--"],
                               'REF_FREQUENCY':["75E6"],
                               'LOFAR_ANTENNA_SET':["--OUTER--"]}
        tb.table.variable_dictionary = variable_dictionary
        fov = self.imager_create_dbs._field_of_view("MS_name")
        self.assertAlmostEqual(fov, 1.83, 2, "Incorrect FOV Value")


    def test_field_of_view_incorrect_antenna_name(self):
        """
        When the measurement set is from an antenna with a name NOT
        containing either LBA or HBA en Exception should be trown
        """
        variable_dictionary = {'NAME':["--error--"]}
        tb.table.variable_dictionary = variable_dictionary
        self.assertRaises(Exception, self.imager_create_dbs._field_of_view, "MS_name")

#    def test__create_parmdb(self):
#        """
#        Test the correct functioning of the create parmdbs function
#        1. test if dir is created
#        2. test if dir contains files (content tests omitted: thats a parmdbs 
#            unit test.
#        3. correct return value
#         
#        """
#        path_to_create = os.path.join(self.test_path, "testParmdb")
#        create_directory(path_to_create)
#
#        parmdb_output = os.path.join(path_to_create, "parmdbs")
#        parmdb_executable = "/opt/cep/LofIm/daily/lofar/bin/parmdbm" #TODO: static
#        self.assertTrue(0 == self.imager_create_dbs._create_parmdb(parmdb_executable,
#                                                            parmdb_output),
#                        self.imager_create_dbs.logger._log[-1])
#
#        self.assertTrue(os.path.exists(parmdb_output), "targer dir to be"
#                        "created by parmdb does not exist")
#        table_data_file_path = os.path.join(parmdb_output, "table.dat")
#        self.assertTrue(os.path.exists(table_data_file_path),
#                        "Creation of table.dat failed")
#
#
#        shutil.rmtree(path_to_create)
#
#    def test__create_parmdb_missing_exec(self):
#        """
#        Test the correct functioning of the create parmdbs function
#        
#        """
#        path_to_create = os.path.join(self.test_path, "testParmdb")
#        create_directory(path_to_create)
#
#        parmdb_output = os.path.join(path_to_create, "parmdbs")
#        parmdb_executable = "/opt/cep/LofIm/daily/lofar/bin/incorrectExecutable"
#        self.assertTrue(1 == self.imager_create_dbs._create_parmdb(parmdb_executable,
#                                                            parmdb_output),
#                        self.imager_create_dbs.logger.last())
#
#
#        self.assertFalse(os.path.exists(parmdb_output), "target dir to be"
#                        "created by parmdb does exist, while it should not")
#
#        shutil.rmtree(path_to_create)
#
#    def test__create_parmdb_for_timeslices(self):
#        """
#        Test the correct functioning of the _create_parmdb_for_timeslices
#        Creating paramdbs for multiple measurement sets         
#        """
#        path_to_create = os.path.join(self.test_path, "testParmdb")
#        parmdb_ms_output = os.path.join(path_to_create, "parmdbs")
#        create_directory(parmdb_ms_output)
#        parmdb_executable = "/opt/cep/LofIm/daily/lofar/bin/parmdbm"
#
#        #Create a number of paths to supply to the create function
#        ms_paths = []
#        for idx in range(5):
#            ms_paths.append(os.path.join(parmdb_ms_output, str(idx)))
#
#
#        #test output
#        self.assertTrue(
#            0 == self.imager_create_dbs._create_parmdb_for_timeslices(parmdb_executable,
#                 ms_paths, ".parmdb"),
#            self.imager_create_dbs.logger.last())
#
#        #test creation of parmdb
#        final_ms_path = os.path.join(parmdb_ms_output, "4.parmdb")
#        self.assertTrue(os.path.exists(final_ms_path))
#        final_ms_table = os.path.join(final_ms_path, "table.dat")
#        self.assertTrue(os.path.exists(final_ms_table))
#
#    def test__create_parmdb_for_timeslices_except(self):
#        """
#        Test the errorous functioning of the _create_parmdb_for_timeslices
#        with missing executable should return 1 and no created directories         
#        """
#        path_to_create = os.path.join(self.test_path, "testParmdb")
#        parmdb_ms_output = os.path.join(path_to_create, "parmdbs")
#        create_directory(parmdb_ms_output)
#        parmdb_executable = "/opt/cep/LofIm/daily/lofar/bin/missingExcecutable"
#
#        #Create a number of paths to supply to the create function
#        ms_paths = []
#        for idx in range(5):
#            ms_paths.append(os.path.join(parmdb_ms_output, str(idx)))
#
#
#        self.assertTrue(
#            1 == self.imager_create_dbs._create_parmdb_for_timeslices(parmdb_executable,
#                 ms_paths, ".parmdb"),
#            self.imager_create_dbs.logger.last())
#        final_ms_path = os.path.join(parmdb_ms_output, "time_slice_8.dppp.ms.parmdb")
#        self.assertFalse(os.path.exists(final_ms_path))

    def test__create_monet_db_connection(self):
        """
        Tests the correct creation of a monetdb connection
        Monat db is mucked!! and returns: "connection"
        """
        db_host = "hostname"
        db_dbase = "spam"
        db_user = "spam"
        db_passwd = "spam"
        db_port = 1
        self.assertTrue("connection" ==
                        self.imager_create_dbs._create_monet_db_connection(db_host,
                            db_dbase, db_user, db_passwd, db_port),
                        "_create_monat_db_connection() did not return the"
                        " string 'connection'")


    def test__create_monet_db_connection_fail(self):
        """
        Tests the monetdb connection that failse: Altough internally db.Error
        exceptions could be trown these should be caught the function return None
        When no conenction could be established
        
        """
        db_host = "except"
        db_dbase = "spam"
        db_user = "spam"
        db_passwd = "spam"
        db_port = 1
        self.assertRaises(Exception, self.imager_create_dbs._create_monet_db_connection, [db_host,
                            db_dbase, db_user, db_passwd, db_port])


    def test__get_ra_and_decl_from_ms(self):
        """
        Test the extraction of the beam direction from the measurement set
        1. insert temp values is muck db
        2. call extract function 
        """
        ra = 123
        decl = 456
        variable_dictionary = {'PHASE_DIR':[[numpy.array([ra, decl])]]}
        tb.table.variable_dictionary = variable_dictionary

        ret_ra, ret_decl = \
            self.imager_create_dbs._get_ra_and_decl_from_ms("measurementset")

        self.assertTrue((ra == ret_ra) and (decl == ret_decl) ,
                        "_get_ra_and_decl_from_ms dir not return the expected"
                        " values for the re and decl")


    def test__get_ra_and_decl_from_ms_pyrap_raised_except(self):
        """
        Test correct raising of exceptions
        """
        error_message = "test__get_ra_and_decl_from_ms_pyrap_raised_except"
        variable_dictionary = {'FIELD': error_message}
        tb.table.variable_dictionary = variable_dictionary

        self.assertRaises(Exception, self.imager_create_dbs._get_ra_and_decl_from_ms, 'except')
        self.assertTrue(self.imager_create_dbs.logger.last()[1].count(error_message) > 0,
                        "The last logged message is incorrect")


    def test__get_ra_and_decl_from_ms_pyrap_incorrect_data(self):
        """
        Test correct return value on non correct values (but none exceptionaly)
        """
        error_message = "returned PHASE_DIR data did not contain two values"
        variable_dictionary = {'PHASE_DIR': [[numpy.array([1])]]}
        tb.table.variable_dictionary = variable_dictionary

        self.assertTrue(
            None == self.imager_create_dbs._get_ra_and_decl_from_ms('ms'),
                  "_get_ra_and_decl_from_ms should return None when retreived"
                  "data has not 2 entries")

        self.assertTrue(self.imager_create_dbs.logger.last()[1].count(error_message) > 0,
                        "The last logged message is incorrect")


    def test__create_bbs_sky_model_no_theta(self):
        """
        Test correct functioning of _create_bbs_sky_model.
        The inner workings of nested funtions is not tested 
        """

        # create the muck db with location
        ra = 123
        decl = 456

        variable_dictionary = {'NAME':["--LBA--"],
                               'REF_FREQUENCY':["75E6"],
                               'LOFAR_ANTENNA_SET':["--OUTER--"],
                               'PHASE_DIR':[[numpy.array([ra, decl])]]}
        tb.table.variable_dictionary = variable_dictionary

        #Create temp location to save the output!!
        tempdir = tempfile.mkdtemp()
        output_skymodel_name = "bbs.skymodel.test"
        test_skymodel_path = os.path.join(tempdir, output_skymodel_name)


        #test correct return value
        self.assertTrue(0 == self.imager_create_dbs._get_sky_model(
            "measurement_set", test_skymodel_path, "host", "db_port", "db_name",
                "db_user", "db_password"))

        #assert creation of output file
        self.assertTrue(os.path.exists(test_skymodel_path), "output file, not created")

        #assert correct creation of theta 
        fp = open(test_skymodel_path)
        theta = fp.readline()
        self.assertTrue(theta == "0.025\n")
        #clean up the created file and dir
        try:
            os.remove(test_skymodel_path)
        except:
            pass

        #os.rmdir(tempdir)

    def test__create_bbs_sky_model_theta(self):
        """
        Test correct functioning of _create_bbs_sky_model.
        The inner workings of nested funtions is not tested 
        """
        theta = "20.0"

        # create the muck db with location
        ra = 123
        decl = 456
        variable_dictionary = {'NAME':["--LBA--"],
                               'REF_FREQUENCY':["75E6"],
                               'LOFAR_ANTENNA_SET':["--OUTER--"],
                               'PHASE_DIR':[[numpy.array([ra, decl])]]}
        tb.table.variable_dictionary = variable_dictionary

        #Create temp location to save the output!!
        tempdir = tempfile.mkdtemp()
        output_skymodel_name = "bbs.skymodel.test"
        test_skymodel_path = os.path.join(tempdir, output_skymodel_name)


        #test correct return value
        self.assertTrue(0 == self.imager_create_dbs._get_sky_model(
            "measurement_set", test_skymodel_path, "host", "db_port",
             "db_name", "db_user", "db_password", theta))


        #assert creation of output file
        self.assertTrue(os.path.exists(test_skymodel_path))

        #assert correct creation of theta 
        fp = open(test_skymodel_path)
        theta_red = fp.readline()

        self.assertTrue(theta_red == theta + "\n")
        #clean up the created file and dir
        try:
            os.remove(test_skymodel_path)
        except:
            pass

        #os.rmdir(tempdir)


if __name__ == "__main__":
    unittest.main()
