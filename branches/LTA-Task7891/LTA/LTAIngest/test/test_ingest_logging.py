#!/usr/bin/env python

import unittest
import logging
import os
import os.path
from LTAIngest.ingest_config import logger

class TestIngestLogging(unittest.TestCase):
  """Tests for logging part of Ingest"""

  def testBasicLogging(self):
    """Do some basic testing on writing to the ingest.logger"""
    
    #default log handler should be a FileHandler
    handler = logger.handlers[0] if logger.handlers else logger.root.handlers[0]
    self.assertTrue(isinstance(handler, logging.FileHandler))

    #check if logfile exists
    #should be the case, since ingest_config logged that initialization was done
    logpath = handler.baseFilename
    self.assertTrue(os.path.isfile(logpath))

    #test writing and reading a log line
    testlogline = 'unittest log line'
    logger.debug(testlogline)
    logfile = open(logpath, 'r')
    lines = logfile.readlines()
    self.assertTrue(lines[-1].strip().endswith(testlogline))
    
  def testLogRotate(self):
    """Test what happens if an external program (logrotate) moves a logfile
    The logger should automatically recreate the logfile and write to it if it was moved.
    """
    
    handler = logger.handlers[0] if logger.handlers else logger.root.handlers[0]
    logpath = handler.baseFilename
    
    #move (rename) log file (which is what logrotate does)
    logpath_moved = logpath.replace('.log', '.log.old')
    os.rename(logpath, logpath_moved)
    
    #check if it is gone
    self.assertFalse(os.path.isfile(logpath))

    #check new location 
    self.assertTrue(os.path.isfile(logpath_moved))

    #log a line and check if the log file reappears
    testlogline = 'unittest log line'
    logger.debug(testlogline)
    self.assertTrue(os.path.isfile(logpath), 'log file %s (no longer) exists' % logpath)
    
    #test reading the written log line
    logfile = open(logpath, 'r')
    lines = logfile.readlines()
    self.assertTrue(lines[-1].strip().endswith(testlogline))

#run tests if main
if __name__ == '__main__':
    unittest.main(verbosity=2)
