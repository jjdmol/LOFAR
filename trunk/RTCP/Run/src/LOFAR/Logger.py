import sys
from math import modf
from time import time,strftime
import logging
import logging.handlers

DEBUG=False

def initLogger():
  if DEBUG:
    minloglevel = logging.DEBUG
  else:
    minloglevel = logging.INFO

  logging.basicConfig( level = minloglevel,
                     format = "%(levelname)-5s|%(asctime)s|OLAP|%(message)s",
                   )

  loglevels = {
   "DEBUG":  logging.DEBUG,
   "INFO":   logging.INFO,
   "WARN":   logging.WARNING,
   "ERROR":  logging.ERROR,
   "FATAL":  logging.CRITICAL
  }

  for name,level in loglevels.iteritems():
    logging.addLevelName( level, name )

def rotatingLogger( appname, filename ):
  handler = logging.handlers.TimedRotatingFileHandler( filename, when = 1, interval = 'D', backupCount = 0, utc = True )
  logger = logging.getLogger( appname )
  logger.addHandler( handler )

  return logger
