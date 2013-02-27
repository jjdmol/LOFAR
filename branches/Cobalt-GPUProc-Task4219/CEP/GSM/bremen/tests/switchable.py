#!/usr/bin/python
import unittest
from testconfig import config
from src.gsmconnectionmanager import GSMConnectionManager

class SwitchableTest(unittest.TestCase):
    """
    Test with MonetDB/PostgreSQL switch using 'testconfig'.
    """
    def setUp(self):
        """
        """
        if ('monetdb' in config):
            self.cm = GSMConnectionManager(
                                use_monet=bool(config['monetdb'] == 'True'),
                                use_console=False)
            self.is_monet = bool(config['monetdb'] == 'True')
        else:
            self.cm = GSMConnectionManager(use_console=False, use_monet=True)
            self.is_monet = True
