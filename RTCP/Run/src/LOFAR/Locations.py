#!/usr/bin/env python

__all__ = ["Locations","isProduction","isDevelopment","homeDir"]

import os

def isProduction():
  """ Decides whether this is a production run, in order to set sane default values. """

  return os.environ["USER"] == "lofarsys"

def isDevelopment():
  return not isProduction()

def homeDir():
  return os.environ["HOME"]
  
class Locations:
  def __init__(self):
    self.isproduction = isProduction()

    self.buildvars = {}
    self.files = {}
    self.nodes = {}

    self.setDefaults()

  def setDefaults(self):
    # default build variants
    self.buildvars.update( {
	"CNProc":  "gnubgp_cn",
	"IONProc": "gnubgp_ion",
	"Storage": "gnu_openmpi-opt",
    } )

    self.files.update( {
        # allows ${HOME} to be resolved in other paths
        "home":    homeDir(),

        # the parset that will be written by us and read by the sections
        # the observation ID is included to allow parallel observations
	"parset":  "${RUNDIR}/RTCP-${MSNUMBER}.parset", 

        # where to store logs
	"logdir":  "/log/L${YEAR}_${MSNUMBER}",

        # where to start the executables. rundir needs to be reachable
        # for all sections.
	"rundir":  "${BASEDIR}",

        # location of the observation id counter
	"nextmsnumber": "/log/nextMSNumber",
    } )

    self.nodes.update( {
        # on which node to start the mpirun for Storage
        "storagemaster": "listfen",
    } )

    if self.isproduction:
      self.files.update( {
        # the base directory most paths will be related to
	"basedir": "/cephome/lofar-prod/lofar",

        # the locations of the main executables
	"cnproc":  "${BASEDIR}/bin/CN_Processing",
	"ionproc": "${BASEDIR}/bin/IONProc",
	"storage": "${BASEDIR}/bin/Storage",

        # where to start the executables. rundir needs to be reachable
        # for all sections.
	"rundir":  "${BASEDIR}/share",
      } )
      
      self.nodes.update( {
        # which storage nodes to use
	"storage": ["list001","list002"],

        # default log server address
        "logserver": "tcp:127.0.0.1:24500"
      } )
    else:
      self.files.update( {
	"basedir": "${HOME}/projects/LOFAR",
	"cnproc":  "${BASEDIR}/installed/%s/bin/CN_Processing" % (self.buildvars["CNProc"],),
	"ionproc": "${BASEDIR}/installed/%s/bin/IONProc" % (self.buildvars["IONProc"],),
	"storage": "${BASEDIR}/installed/%s/bin/Storage" % (self.buildvars["Storage"],),
      } )

      self.nodes.update( {
	"storage": ["list003","list004"],

        # no external log server
        "logserver": "",
      } )

  def setFilename(self,name,path):
    self.files[name] = path 

  def resolvePath(self,path,parset=None):
    """ Resolve a path by replacing ${BASEDIR} by self["basedir"], etc.

        For replacements, the paths in self.files are used, and optionally
        the masks allowed by parset.parseMask, such as ${OBSID}. """

    allNames = [("${%s}" % (name.upper(),),value) for name,value in self.files.iteritems()]


    # resolve until nothing changes
    changed = True
    while changed:
      changed = False

      for name,value in allNames:
	if name in path:
	  path = path.replace( name, value )
          changed = True

    # resolve parset variables like ${MSNUMBER},
    # we do this after path resolving above to allow
    # redirection
    if parset is not None:
      path = parset.parseMask( path )

    return path	

  def resolveAllPaths(self, parset = None):
    for name,path in self.files.iteritems():
      self.files[name] = self.resolvePath( path, parset )

Locations = Locations()

