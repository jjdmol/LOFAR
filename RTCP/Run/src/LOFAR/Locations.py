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
    self.isproduction = isProduction()

    self.files = {}

    self.files.update( {
        # the parset that will be written by us and read by the sections
        # the observation ID is included to allow parallel observations

        # where configuration files are kept
        "configdir": "%s/etc" % (lofarRoot(),),

        # location of the observation id counter
	"nextmsnumber": "/globalhome/lofarsystem/log/nextMSNumber",
    } )

    if self.isproduction:
      self.files.update( {
        # the location of the Storage executable
	"storage": "/data/home/lofarsys/production/lofar/bin/Storage_main",

	# where to save the parset
        "parset":      "/globalhome/lofarsystem/parsets/L${OBSID}.parset", # for communication with Storage and offline pipelines
	"parset-ion":  "/bghome0/lofarsys/parsets/L${OBSID}.parset", # for communication to the IO nodes 
      } )
    else:
      self.files.update( {
        # the base directory most paths will be related to
	"basedir":    "%s/projects/LOFAR" % (homeDir(),),

        # the location of the Storage executable
	"storage":    "${BASEDIR}/installed/gnu_opt/bin/Storage_main",

	# where to save the parset
        "parset":     "${BASEDIR}/parsets/L${OBSID}.parset",     # for communication with Storage and offline pipelines
        "parset-ion": "${BASEDIR}/parsets/L${OBSID}.parset", # for communication with the I/O nodes
      } )

  def setFilename(self,name,path):
    self.files[name] = path 

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

  def resolveAllPaths(self, parset = None):
    for name,path in self.files.iteritems():
      self.files[name] = self.resolvePath( path, parset )

Locations = Locations()

