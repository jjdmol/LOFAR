import sys
from math import modf
from time import time,strftime

APPNAME="OLAP"

DEBUG=False
VERBOSE=False

def log( level, str ):
  now =  "%s.%06d" % (strftime("%F %T"),(int(modf(time())[0]*1e6)))
  print >>sys.stderr,"%-5s|%s|%s|%s" % (level,now,APPNAME,str)

def debug( str ):
  if DEBUG:
    log( "DEBUG", str )

def info( str ):
  if VERBOSE:
    log( "INFO", str )

def warning( str ):
  log( "WARN", str )

def error( str ):
  log( "ERROR", str )

def fatal( str ):
  log( "FATAL", str )
  sys.exit(1)
