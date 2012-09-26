#!/usr/bin/python
from os import path
from src.gsmlogger import get_gsm_logger
from src.gsmparset import GSMParset

class TempParset(GSMParset):
    """
    """
    def __init__(self, datafilename, freq):
        """
        """
        self.filename = datafilename
        self.path = path.dirname(__file__)
        self.data = {'image_id': datafilename,
                     'source_lists': datafilename,
                     'frequency': freq}
        self.parset_id = self.data.get('image_id')
        self.image_id = None # Not yet known.
        self.source_count = None
        self.log = get_gsm_logger('parsets', 'test.log')
