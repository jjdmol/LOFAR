#!/usr/bin/python
import unittest
from src.queries import get_svn_version, makelistable

class UtilsTest(unittest.TestCase):
    def test_svn(self):
        self.assertIsInstance(get_svn_version(), int)

    def test_makelistable(self):
        @makelistable
        def try_listable(par1, par2):
            return '%s-%s' % (par1, par2)
        self.assertEquals(try_listable('1', '2'), '1-2')
        self.assertEquals(try_listable(['1', 'a'], '2'), '1-2,a-2')
