#!/usr/bin/env python

__all__ = ["Locations","Hosts","isProduction","isDevelopment","homeDir"]

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

class Hosts:
  def __init__(self):
    self.hostnames = {}

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

class Locations:
  def __init__(self):
    self.isproduction = isProduction()

    self.buildvars = {}
    self.executables = {}
    self.files = {}
    self.nodes = {}

    self.setDefaults()

  def setDefaults(self):
    # default build variants
    self.buildvars.update( {
        "Storage": "gnu_opt",
    } )
    self.executables.update( {
        "Storage": "Storage_main",
    } )

    self.files.update( {
        # allows ${HOME} to be resolved in other paths
        "home":    homeDir(),

        # the parset that will be written by us and read by the sections
        # the observation ID is included to allow parallel observations

        # where to store logs
	"logdir":  "${BASEDIR}/log",

        # where configuration files are kept
        "configdir": ".",

        # parset name mas
        "parset": "${LOGDIR}/L${OBSID}.parset",     # for communication with Storage and offline pipelines
        "parset-ion": "${LOGDIR}/L${OBSID}.parset", # for communication with the I/O nodes

        # location of the observation id counter
	"nextmsnumber": "/globalhome/lofarsystem/log/nextMSNumber",
    } )

    if self.isproduction:
      self.files.update( {
        # fix the homedir, for systems which link to /cephome, /localhome, /home. etc
        "home":    "/globalhome/lofarsystem",

        # the base directory most paths will be related to
	"basedir": "${HOME}/production/lofar",

        # the locations of the main executables
	"storage": "/opt/storage/current/bin/%s" % (self.executables["Storage"],),

        # where to store logs
	"logdir":  "${HOME}/log",

	# where to save the parset
	"parset-ion":  "/bghome0/lofarsys/parsets/RTCP-${OBSID}.parset", # for communication to the IO nodes 

        # where configuration files are kept
        "configdir": "${BASEDIR}/bgfen/etc",

        # location of valgrind suppressions file
        "ionsuppfile": "",
        "storagesuppfile": "",
      } )
    else:
      self.files.update( {
        # the base directory most paths will be related to
	"basedir": "${HOME}/projects/LOFAR",

        # where configuration files are kept
        "configdir": "${BASEDIR}/RTCP/Run/src",

	"storage": "${BASEDIR}/installed/%s/bin/%s" % (self.buildvars["Storage"],self.executables["Storage"]),

        # location of valgrind suppressions file
        "ionsuppfile": "${BASEDIR}/RTCP/IONProc/src/IONProc.supp",
        "storagesuppfile": "${BASEDIR}/RTCP/Storage/src/Storage.supp",
      } )

    #if not os.path.isdir( self.files["configdir"] ):
    #  # fall back to default config dir
    #  self.files["configdir"] = os.path.dirname(__file__)+"/.."

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
Hosts = Hosts()

