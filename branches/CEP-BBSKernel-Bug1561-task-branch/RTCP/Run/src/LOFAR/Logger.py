import sys
from math import modf
from time import time,strftime
import logging
import logging.handlers
from traceback import format_exception

DEBUG=False

def my_excepthook( etype, value, tb ):
  """ Replacement for default exception handler, which uses the logger instead of stderr. """

  lines = format_exception( etype, value, tb )

  for l in lines:
    for m in l.split("\n")[:-1]:
      logging.critical( m )

def initLogger():
  if DEBUG:
    minloglevel = logging.DEBUG
  else:
    minloglevel = logging.INFO

  logging.basicConfig( level = minloglevel,
                       format = "OLAP %(asctime)s.%(msecs)03d %(levelname)-5s %(message)s",
                       datefmt = "%d-%m-%y %H:%M:%S",
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

  sys.excepthook = my_excepthook  

def rotatingLogger( appname, filename ):
  logger = logging.getLogger( appname )

  handler = logging.handlers.TimedRotatingFileHandler( filename, when = "midnight", interval = 1, backupCount = 0 )

  logger.propagate = False
  logger.addHandler( handler )

  return logger
