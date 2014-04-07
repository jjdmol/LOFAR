#!/usr/bin/python
import unittest
from testconfig import config
from src.gsmconnectionmanager import GSMConnectionManager

class SwitchableTest(unittest.TestCase):
    """
    """
    def setUp(self):
        """
        """
        if ('monetdb' in config):
            self.cm = GSMConnectionManager(
                                use_monet=bool(config['monetdb'] == 'True'),
                                use_console=False)
        else:
            self.cm = GSMConnectionManager(use_console=False, use_monet=True)

