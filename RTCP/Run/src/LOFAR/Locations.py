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

    if self.isproduction:
      self.files.update( {
        # the base directory most paths will be related to
	"basedir": "/cephome/lofar-prod/lofar",

        # the locations of the main executables
	"cnproc":  "${BASEDIR}/bin/CN_Processing.cnk",
	"ionproc": "${BASEDIR}/bin/IONProc.ppc",
	"storage": "${BASEDIR}/bin/Storage.x86_64",

      } )
      
      self.nodes.update( {
        # which storage nodes to use
	"storage": ["list001","list002"],
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
      } )

    self.files.update( {
        # allows ${HOME} to be resolved in other paths
        "home":    homeDir(),

        # the parset that will be written by us and read by the sections
	"parset":  "${RUNDIR}/RTCP.parset", 

        # where to store logs and start the executables
	"logdir":  "${BASEDIR}/log",
	"rundir":  "${LOGDIR}",

        # locations of the observation id counter and tables
	"mslist":       "listfen:/log/MSList",
	"nextmsnumber": "listfen:/log/nextMSNumber",
    } )

    self.nodes.update( {
        # on which node to start the mpirun for Storage
        "storagemaster": "listfen",
    } )

  def setFilename(self,name,path):
    self.files[name] = path 

  def resolvePath(self,path,parset=None):
    """ Resolve a path by replacing ${BASEDIR} by self["basedir"], etc. """

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

