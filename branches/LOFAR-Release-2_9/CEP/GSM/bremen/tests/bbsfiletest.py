#!/usr/bin/python
import unittest
from src.errors import SourceException
from src.bbsfilesource import GSMBBSFileSource
from src.gsmconnectionmanager import GSMConnectionManager
from tests.switchable import SwitchableTest


class BBSFileTest(SwitchableTest):
    """
    Test for BBS-format file input.
    """
    def setUp(self):
        super(BBSFileTest, self).setUp()
        self.conn = self.cm.get_connection()

    def test_first_file(self):
        xfile = GSMBBSFileSource('test', 0, 'tests/data/new_field.dat', 'test')
        self.assertTrue(xfile.read_and_store_data(self.conn))

    def test_full(self):
        xfile = GSMBBSFileSource('test', 0, 'tests/data/full_file.dat')
        self.assertTrue(xfile.read_and_store_data(self.conn))
        g_major =self.conn.exec_return('select g_major ' \
                                         'from detections where lra < 123.0;')
        self.assertEqual(g_major, 0.0)
        g_major =self.conn.exec_return('select g_major ' \
                                         'from detections where lra > 125.6;')
        self.assertEqual(g_major, 0.101701)

    def test_full2(self):
        xfile = GSMBBSFileSource('test', 0, 'tests/data_extended/field_ext.dat')
        self.assertTrue(xfile.read_and_store_data(self.conn))

    def test_wrong_default(self):
        xfile = GSMBBSFileSource('test', 0, 'tests/data/bad_file.dat')
        self.assertRaises(SourceException, xfile.read_and_store_data, self.conn)

    def test_wrong_test(self):
        xfile = GSMBBSFileSource('test', 0, 'tests/data/bbs_field.dat', 'test')
        self.assertRaises(SourceException, xfile.read_and_store_data, self.conn)

    def tearDown(self):
        self.conn.execute("delete from detections;")
        self.conn.close()

