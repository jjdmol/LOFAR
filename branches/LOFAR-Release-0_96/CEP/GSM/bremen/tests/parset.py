#!/usr/bin/python
import unittest
from src.errors import ParsetContentError, SourceException
#from src.bbsfilesource import GSMBBSFileSource
from src.gsmparset import GSMParset
from src.gsmconnectionmanager import GSMConnectionManager

class ParsetTest(unittest.TestCase):
    def test_sample_parset(self):
        parset = GSMParset('tests/sample.parset')
        cm = GSMConnectionManager(database='test')
        loaded = parset.process(cm.get_connection())
        self.assertEquals(loaded, 13)

    def test_missing_parset(self):
        with self.assertRaises(IOError):
            _ = GSMParset('tests/nonexists.parset')

    def test_wrong_parset(self):
        with self.assertRaises(ParsetContentError):
            parset = GSMParset('tests/wrong1.parset')
            cm = GSMConnectionManager(database='test')
            loaded = parset.process(cm.get_connection())
        with self.assertRaises(SourceException):
            parset = GSMParset('tests/wrong2.parset')
            cm = GSMConnectionManager(database='test')
            _ = parset.process(cm.get_connection())
