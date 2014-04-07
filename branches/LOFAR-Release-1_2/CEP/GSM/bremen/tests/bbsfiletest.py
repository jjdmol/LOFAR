#!/usr/bin/python
import unittest
from src.bbsfilesource import GSMBBSFileSource
from src.gsmconnectionmanager import GSMConnectionManager
from tests.switchable import SwitchableTest


class BBSFileTest(SwitchableTest):
    """
    Test for BBS-format file input.
    """
    def test_first_file(self):
        self.cm = GSMConnectionManager()
        xfile = GSMBBSFileSource('test', 'tests/data/new_field.dat', 'test')
        self.assertTrue(xfile.read_and_store_data(self.cm.get_connection()))

    def tearDown(self):
        conn = self.cm.get_connection()
        conn.execute("delete from detections;")
        conn.close()

