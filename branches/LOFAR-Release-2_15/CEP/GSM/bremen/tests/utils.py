#!/usr/bin/python
import unittest
import sys
from src.queries import get_svn_version, makelistable

class UtilsTest(unittest.TestCase):
    if sys.version_info < (2, 7):
        def assertIsInstance(self, par1, par2):
            self.assertTrue(isinstance(par1, par2))

    def test_svn(self):
        self.assertIsInstance(get_svn_version(), int)

    def test_makelistable(self):
        @makelistable
        def try_listable(par1, par2):
            return '%s-%s' % (par1, par2)
        self.assertEquals(try_listable('1', '2'), '1-2')
        self.assertEquals(try_listable(['1', 'a'], '2'), '1-2,a-2')
