#!/usr/bin/python
import unittest
from src.errors import ParsetContentError, SourceException, GSMException
from src.gsmparset import GSMParset
from src.gsmconnectionmanager import GSMConnectionManager
from tests.switchable import SwitchableTest


class ParsetTest(SwitchableTest):
    def test_sample_parset(self):
        parset = GSMParset('tests/sample.parset')
        loaded = parset.process(self.cm.get_connection(database='test'))
        self.assertEquals(loaded, 22)

    def test_missing_parset(self):
        self.assertRaises(GSMException, GSMParset, 'tests/nonexists.parset')

    def test_wrong_parset(self):
        parset = GSMParset('tests/wrong1.parset')
        self.assertRaises(ParsetContentError, parset.process,
                                    self.cm.get_connection(database='test'))
        parset = GSMParset('tests/wrong2.parset')
        self.assertRaises(SourceException, parset.process,
                                    self.cm.get_connection(database='test'))
