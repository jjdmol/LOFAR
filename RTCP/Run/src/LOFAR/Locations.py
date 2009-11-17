#!/usr/bin/env python

__all__ = ["Locations","Hosts","isProduction","isDevelopment","homeDir"]

import os
from socket import gethostbyname

def isProduction():
  """ Decides whether this is a production run, in order to set sane default values. """

  return os.environ["USER"] == "lofarsys"

def isDevelopment():
  return not isProduction()

def homeDir():
  return os.environ["HOME"]

class Hosts:
  def __init__(self):
    self.hostnames = {}

    self.populate()

  def add(self,hostname,ip,interface="external"):
    ips = self.hostnames.get( hostname, {} );
    ips[interface] = ip

    self.hostnames[hostname] = ips

  def resolve(self,hostname,interface="external"):
    if hostname in self.hostnames:
      ips = self.hostnames[hostname]

      if interface in ips:
        return ips[interface]

    # fallback
    return gethostbyname( hostname )

  def populate(self):
    # storage nodes lse001 - lse024
    for i in xrange( 1, 24 ):
      self.add( "lse%03d" % (i,),
                "10.176.1.%d" % (i,),
                "front" )

      self.add( "lse%03d" % (i,),
                "10.174.0.%d" % (i,),
                "back" )
  
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
	"logdir":  "${BASEDIR}/D${YEAR}_${MSNUMBER}",

        # where to start the executables. rundir needs to be reachable
        # for all sections.
	"rundir":  "${BASEDIR}",

        # location of the observation id counter
	"nextmsnumber": "/log/nextMSNumber",

    } )

    self.nodes.update( {
        # on which node to start the mpirun for Storage
        "storagemaster": "10.144.4.1",
    } )

    if self.isproduction:
      self.files.update( {
        # fix the homedir, for systems which link to /cephome, /localhome, /home. etc
        "home":    "/globalhome/lofarsystem",

        # the base directory most paths will be related to
	"basedir": "${HOME}/production/lofar",

        # the locations of the main executables
	"cnproc":  "${BASEDIR}/bgp_cn/bin/CN_Processing",
	"ionproc": "${BASEDIR}/bgp_ion/bin/IONProc",
	"storage": "${BASEDIR}/lfe/bin/Storage",

        # where to start the executables. rundir needs to be reachable
        # for all sections.
	"rundir":  "/globalhome/lofarsystem/log",

        # where to store logs
	"logdir":  "/globalhome/lofarsystem/log/L${YEAR}_${MSNUMBER}",

        # location of valgrind suppressions file
        "ionsuppfile": "",
        "storagesuppfile": "",
      } )
      
      self.nodes.update( {
        # default log server address
        "logserver": "",
      } )
    else:
      self.files.update( {
	"basedir": "${HOME}/projects/LOFAR",
	"cnproc":  "${BASEDIR}/installed/%s/bin/CN_Processing" % (self.buildvars["CNProc"],),
	"ionproc": "${BASEDIR}/installed/%s/bin/IONProc" % (self.buildvars["IONProc"],),
	"storage": "${BASEDIR}/installed/%s/bin/Storage" % (self.buildvars["Storage"],),

        # location of valgrind suppressions file
        "ionsuppfile": "${BASEDIR}/RTCP/IONProc/src/IONProc.supp",
        "storagesuppfile": "${BASEDIR}/RTCP/Storage/src/Storage.supp",
      } )

      self.nodes.update( {
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
Hosts = Hosts()

