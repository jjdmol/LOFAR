import sys

APPNAME="OLAP"

DEBUG=False
VERBOSE=False

def log( level, str ):
  print >>sys.stderr,"%-5s|%s|%s" % (level,APPNAME,str)

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
