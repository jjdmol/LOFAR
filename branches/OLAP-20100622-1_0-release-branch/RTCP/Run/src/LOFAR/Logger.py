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

  logging.raiseExceptions = False                 

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
  logger = logging.getLogger( appname )

  handler = logging.handlers.TimedRotatingFileHandler( filename, when = "midnight", interval = 1, backupCount = 0 )

  logger.propagate = False
  logger.addHandler( handler )

  return logger
