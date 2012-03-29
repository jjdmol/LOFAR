#!/usr/bin/env python

__all__ = ["Locations","isProduction","isDevelopment","homeDir","lofarRoot"]

import os
import time
from socket import gethostbyname

def isProduction():
  """ Decides whether this is a production run, in order to set sane default values. """

  return os.environ["USER"] == "lofarsys"

def isDevelopment():
  return not isProduction()

def homeDir():
  return os.environ["HOME"]

def lofarRoot():
  return os.environ["LOFARROOT"] or "/opt/lofar"

class Locations:
  def __init__(self):
    self.files = {
        # where configuration files are kept
        "configdir": "%s/etc" % (lofarRoot(),),
    }

  def resolvePath(self,path,parset=None):
    """ Resolve a path by replacing ${BASEDIR} by self["basedir"], etc.

        For replacements, the paths in self.files are used, and optionally
        the masks allowed by parset.parseMask, such as ${OBSID}. """

    allNames = [("${%s}" % (name.upper(),),value) for name,value in self.files.iteritems()]
    allNames.extend( [
      ("${TIMESTAMP}", time.strftime("%Y-%m-%d_%H%M%S")),
      ] )

    # resolve until nothing changes
    changed = True
    while changed:
      changed = False

      for name,value in allNames:
	if name in path:
	  path = path.replace( name, value )
          changed = True

    # resolve parset variables like ${OBSID},
    # we do this after path resolving above to allow
    # redirection
    if parset is not None:
      path = parset.parseMask( path )

    return path	

Locations = Locations()

