#!/usr/bin/python
import sys,os

# allow ../util to be found, a bit of a hack
sys.path += [(os.path.dirname(__file__) or ".")+"/.."]

import datetime
import time
import socket
import util.Parset
import getpass
import os
from itertools import count
from Partitions import PartitionPsets
from Locations import Hosts,Locations
from Stations import Stations
from util.dateutil import parse,format,parseDuration,timestamp
from logging import error
import math
from sets import Set

class Parset(util.Parset.Parset):
    def __init__(self):
        util.Parset.Parset.__init__(self)

	self.stations = []
	self.storagenodes = []
	self.partition = ""
	self.psets = []

        self.filename = ""

    def setFilename( self, filename ):
        self.filename = filename

    def getFilename( self ):
        return self.filename

    def save(self):
        self.writeFile( self.filename )

    def distillStations(self, key="Observation.VirtualInstrument.stationList"):
        """ Distill station names to use from the parset file and return them. """

        if key not in self:
          return "+".join(self.get('OLAP.storageStationNames',""))

        # translate station name + antenna set to CEP comprehensable names
        antennaset = self["Observation.antennaSet"]

        def applyAntennaSet( station, antennaset ):
          if antennaset == "":
            # useful for manually entered complete station names like CS302HBA1
            suffix = [""]
          elif antennaset in ["LBA_INNER","LBA_OUTER","LBA_X","LBA_Y","LBA_SPARSE_EVEN","LBA_SPARSE_ODD"]:
            suffix = ["LBA"]
          elif station.startswith("CS"):
            if antennaset == "HBA_ZERO":
              suffix = ["HBA0"]
            elif antennaset == "HBA_ONE":  
              suffix = ["HBA1"]
            elif antennaset == "HBA_JOINED":  
              suffix = ["HBA"]
            else: 
              assert antennaset == "HBA_DUAL", "Unknown antennaSet: %s" % (antennaset,)
              suffix = ["HBA0","HBA1"]
          else:  
            suffix = ["HBA"]

          return "+".join(["%s%s" % (station,s) for s in suffix])

        return "+".join( [applyAntennaSet(s, self["Observation.antennaSet"]) for s in self[key]] )

    def distillPartition(self, key="OLAP.CNProc.partition"):
        """ Distill partition to use from the parset file and return it. """

        if key not in self:
          return ""

        return self[key]

    def distillStorageNodes(self, key="Observation.VirtualInstrument.storageNodeList"):
        """ Distill storage nodes to use from the parset file and return it. """

        if key not in self:
          return []

        return self.getStringVector(key)

    def postRead(self):
        """ Distill values for our internal variables from the parset. """

        # partition
        partition = self.distillPartition()
        if partition:
          self.setPartition( partition )

        # storage nodes
        storagenodes = self.distillStorageNodes() or []
        self.setStorageNodes( storagenodes )

        # stations
        stationStr = self.distillStations()
        if stationStr:
          stationList = Stations.parse( stationStr )
          self.setStations( stationList )

    def addMissingKeys(self):
        """ Sets some default values which SAS does not yet contain. """

        # meta data
        self.setdefault("Observation.ObserverName","unknown")
        self.setdefault("Observation.ProjectName","unknown")

        self.setdefault("OLAP.Correlator.integrationTime",1);

    def convertDepricatedKeys(self):
        """ Converts some new keys to old ones to help old CEP code cope with new SAS code. """

        pass


    def convertSASkeys(self):
        """ Convert keys generated by SAS to those used by OLAP. """

        def delIfEmpty( k ):
          if k in self and not self[k]:
            del self[k]

        # SAS cannot omit keys, so assume that empty keys means 'use default'
        delIfEmpty( "OLAP.CNProc.phaseOnePsets" )
        delIfEmpty( "OLAP.CNProc.phaseTwoPsets" )
        delIfEmpty( "OLAP.CNProc.phaseThreePsets" )

        # SAS uses the incorrect names for these keys
        if "OLAP.CNProc.usedCores" in self:
          self.setdefault("OLAP.CNProc.usedCoresInPset",self.getInt32Vector("OLAP.CNProc.usedCores"))

        # SAS specifies beams differently
        if "Observation.subbandList" not in self:
          # convert beam configuration
          allSubbands = {}

	  for b in count():
            if "Observation.Beam[%s].angle1" % (b,) not in self:
              break

	    beamSubbands = self.getInt32Vector("Observation.Beam[%s].subbandList" % (b,)) # the actual subband number (0..511)
            beamBeamlets = self.getInt32Vector("Observation.Beam[%s].beamletList" % (b,)) # the bebamlet index (0..247)

            assert len(beamSubbands) == len(beamBeamlets), "Beam %d has %d subbands but %d beamlets defined" % (b,len(beamSubbands),len(beamBeamlets))

            for subband,beamlet in zip(beamSubbands,beamBeamlets):
              assert beamlet not in allSubbands, "Beam %d and %d both use beamlet %d" % (allSubbands[beamlet]["beam"],b,beamlet)

              allSubbands[beamlet] = {
                "beam":     b,
                "subband":  subband,
                "beamlet":  beamlet,

                # assign rsp board and slot according to beamlet id
                "rspboard": beamlet // 62,
                "rspslot":  beamlet % 62,
              }


          # order subbands according to beamlet id, for more human-friendly reading
          sortedSubbands = [allSubbands[x] for x in sorted( allSubbands )]

          # populate OLAP lists
	  self["Observation.subbandList"]  = [s["subband"] for s in sortedSubbands]
	  self["Observation.beamList"]     = [s["beam"] for s in sortedSubbands]
	  self["Observation.rspBoardList"] = [s["rspboard"] for s in sortedSubbands]
	  self["Observation.rspSlotList"]  = [s["rspslot"] for s in sortedSubbands]

    def addStorageKeys(self):
	self["OLAP.Storage.userName"] = getpass.getuser()
	self["OLAP.Storage.sshIdentityFile"]  = "%s/.ssh/id_rsa" % (os.environ["HOME"],)
	self["OLAP.Storage.msWriter"] = Locations.resolvePath( Locations.files["storage"], self )

    def preWrite(self):
        """ Derive some final keys and finalise any parameters necessary
	    before writing the parset to disk. """

        self.convertSASkeys();
        self.convertDepricatedKeys();
        self.addMissingKeys();
	self.addStorageKeys();

        # Versioning info
        self["OLAP.BeamsAreTransposed"] = True

	# TODO: we use self.setdefault, but this can create inconsistencies if we
	# set one value but not the other in a pair of interdependent parameters.
	# This should possibly be detected in the check routine, but it seems
	# sloppy to let it pass through here unnoticed.

	# tied-array beam forming
        tiedArrayStationList = []
	tabList = []
	beamFormedStations = []

        for index in count():
          if "Observation.Beamformer[%s].stationList" % (index,) not in self:
            break

	  curlist = self.getString('Observation.Beamformer[%s].stationList' % (index,))

          # remove any initial or trailing "
	  curlist = curlist.strip('"').rstrip('"')

          # transform , to +
	  curlist = curlist.replace(',','+')

	  tiedArrayStationList.append(curlist)

          # extract the individual stations
	  beamFormedStations += curlist.split('+')
	
	if index > 0:
	  # tied-array beamforming will occur

	  # add remaining stations to the list
	  allStationNames = [st.getName() for st in self.stations]
	  tiedArrayStationList += filter( lambda st: st not in beamFormedStations, allStationNames )

          # create a list of indices for all the stations, which by definition occur in
	  # the tiedArrayStationList
	  def findTabStation( s ):
	    for nr,tab in enumerate(tiedArrayStationList):
	      for st in tab.split('+'):
	        if s.getName() == st:
	          return nr

	  tabList = map( findTabStation, self.stations )
	   
	self.setdefault('OLAP.tiedArrayStationNames', tiedArrayStationList)
	self.setdefault('OLAP.CNProc.tabList', tabList)

	# input flow configuration
	for station in self.stations:
	  self.setdefault('PIC.Core.Station.%s.RSP.ports' % (station.name,), station.inputs)
	  
          stationName = station.name.split("_")[0] # remove specific antenna or array name (_hba0 etc) if present
          self.setdefault("PIC.Core.%s.position" % (stationName,), self["PIC.Core.%s.phaseCenter" % (stationName,)])

	for pset in xrange(len(self.psets)):
	  self.setdefault('PIC.Core.IONProc.%s[%s].inputs' % (self.partition,pset), [
	    "%s/RSP%s" % (station.name,rsp) for station in self.stations
	                                      if station.getPsetIndex(self.partition) == pset
	                                    for rsp in xrange(len(station.inputs))] )


	# output flow configuration
        self['OLAP.storageStationNames'] = [s.name for s in self.stations]

        self.setdefault('OLAP.OLAP_Conn.IONProc_Storage_Transport','TCP');
        self.setdefault('OLAP.OLAP_Conn.IONProc_CNProc_Transport','FCNP');

	# subband configuration
	if "Observation.subbandList" in self:
	  nrSubbands = len(self.getInt32Vector("Observation.subbandList"))

        for nrBeams in count():
          if "Observation.Beam[%s].angle1" % (nrBeams,) not in self:
            break

        self.setdefault('Observation.nrBeams', nrBeams)

	# Pset configuration
	self['OLAP.CNProc.partition'] = self.partition
        self['OLAP.IONProc.psetList'] = self.psets

	nrPsets = len(self.psets)
	nrStorageNodes = self.getNrUsedStorageNodes()
        nrBeamFiles = self.getNrBeamFiles()
        cores = self.getInt32Vector("OLAP.CNProc.usedCoresInPset")

        # set and resolve storage hostnames
        # sort them since mpirun will as well, messing with our indexing schemes!
        self["OLAP.OLAP_Conn.IONProc_Storage_ServerHosts"] = [Hosts.resolve( s, "back") for s in self.storagenodes]

	self.setdefault('OLAP.nrPsets', nrPsets)
	self.setdefault('OLAP.CNProc.phaseOnePsets', [s.getPsetIndex(self.partition) for s in self.stations])
	self.setdefault('OLAP.CNProc.phaseTwoPsets', range(nrPsets))
        if self.phaseThreeExists():
	  self.setdefault('OLAP.CNProc.phaseThreePsets', self['OLAP.CNProc.phaseTwoPsets'])
        else:  
	  self.setdefault('OLAP.CNProc.phaseThreePsets', [])

        self.setdefault('OLAP.CNProc.phaseOneTwoCores',cores)  
        self.setdefault('OLAP.CNProc.phaseThreeCores',cores)

        self["OLAP.CNProc.phaseThreeDisjunct"] = self.phaseThreePsetDisjunct() or self.phaseThreeCoreDisjunct()

        # what will be stored where?
        # outputSubbandPsets may well be set before finalize()
	self.setdefault('OLAP.subbandsPerPset', int( math.ceil(1.0 * nrSubbands / max( 1, len(self["OLAP.CNProc.phaseTwoPsets"]) ) )  ) )
	if nrStorageNodes == 0:
	  self.setdefault('OLAP.storageNodeList',[0] * nrSubbands)
	else:  
	  self.setdefault('OLAP.storageNodeList',[i//int(math.ceil(1.0 * nrSubbands/nrStorageNodes)) for i in xrange(nrSubbands)])

	self.setdefault('OLAP.PencilInfo.beamsPerPset', int( math.ceil(1.0 * nrBeamFiles / max( 1, len(self["OLAP.CNProc.phaseThreePsets"]) ) ) ) )
	if nrStorageNodes == 0:
	  self.setdefault('OLAP.PencilInfo.storageNodeList',[0] * nrBeamFiles)
	else:  
	  self.setdefault('OLAP.PencilInfo.storageNodeList',[i//int(math.ceil(1.0 * nrBeamFiles/nrStorageNodes)) for i in xrange(nrBeamFiles)])

	# calculation configuration

        # integration times of CNProc and IONProc, based on self.integrationtime
        # maximum amount of time CNProc can integrate
        maxCnIntegrationTime = 1.2 # seconds

        # (minimal) number of times the IONProc will have to integrate
        integrationtime = float( self["OLAP.Correlator.integrationTime"] )
        ionIntegrationSteps = int(math.ceil(integrationtime / maxCnIntegrationTime))
        self.setdefault('OLAP.IONProc.integrationSteps', ionIntegrationSteps)

        # the amount of time CNProc will integrate, translated into samples
        cnIntegrationTime = integrationtime / int(self["OLAP.IONProc.integrationSteps"])
        nrSamplesPerSecond = int(self['Observation.sampleClock']) * 1e6 / 1024 / int(self['Observation.channelsPerSubband'])

        cnIntegrationSteps = int(round(nrSamplesPerSecond * cnIntegrationTime / 16)) * 16
        self.setdefault('OLAP.CNProc.integrationSteps', cnIntegrationSteps)

    def setStations(self,stations):
	""" Set the array of stations to use (used internally). """

        def name( s ):
          try:
            return s.name
          except:
            return s
	
	self.stations = sorted( stations, cmp=lambda x,y: cmp(name(x),name(y)) )

    def forceStations(self,stations):
	""" Override the set of stations to use (from the command line). """

	self.setStations(stations)

        def name( s ):
          try:
            return s.name
          except:
            return s

        self['OLAP.storageStationNames'] = map( name, self.stations )
        del self['Observation.VirtualInstrument.stationList']
        del self['Observation.antennaSet']
        
    def setPartition(self,partition):
	""" Define the partition to use. """

	assert partition in PartitionPsets, "Partition %s unknown. Run LOFAR/Partitions.py to get a list of valid partitions." % (partition,)
	
	self.partition = partition
	self.psets = PartitionPsets[partition]

    def setStorageNodes(self,storagenodes):
	""" Define the list of storage nodes to use. """

        # do not resolve host names, since the resolve depends on the need (i.e. NIC needed)
	self.storagenodes = sorted(storagenodes)

        # OLAP needs IP addresses from the backend
        self["OLAP.OLAP_Conn.IONProc_Storage_ServerHosts"] = self.storagenodes

    def setObsID(self,obsid):
        self.setdefault("Observation.ObsID", obsid)

    def getObsID(self):
        if "Observation.ObsID" not in self:
          return None

        return int(self["Observation.ObsID"])

    def getNrUsedStorageNodes(self):
        return len(self.storagenodes)

    def disableCNProc(self):
        self["OLAP.OLAP_Conn.IONProc_CNProc_Transport"] = "NULL"

    def disableIONProc(self):
        self["OLAP.OLAP_Conn.IONProc_Storage_Transport"] = "NULL"
        self["OLAP.OLAP_Conn.IONProc_CNProc_Transport"] = "NULL"

    def disableStorage(self):
        self["OLAP.OLAP_Conn.IONProc_Storage_Transport"] = "NULL"
        self.setStorageNodes([])

    def parseMask( self, mask = None ):
      """ Fills a mask, by default the Observation.MSNameMask. """

      assert "Observation.ObsID" in self, "Observation ID not generated yet."
      if mask is None:
        assert "Observation.MSNameMask" in self, "Observation.MSNameMask not defined in parset."

      mask = mask or self["Observation.MSNameMask"]

      # obtain settings
      date = parse( self["Observation.startTime"] ).timetuple()

      # fill in the mask
      datenames = [ "YEAR", "MONTH", "DAY", "HOURS", "MINUTES", "SECONDS" ] # same order as in time tuple
      for index,d in enumerate(datenames):
        mask = mask.replace( "${%s}" % d, "%02d" % (date[index],) )

      mask = mask.replace( "${MSNUMBER}", "%05d" % (self.getObsID(),) )
      mask = mask.replace( "${SUBBAND}", "*" )
      mask = mask.replace( "${RAID}", "*" )

      return mask

    def setStartStopTime( self, starttime, stoptime ):
      start = timestamp( parse( starttime ) )
      stop  = timestamp( parse( stoptime ) )

      self["Observation.startTime"] = format( start )
      self["Observation.stopTime"] = format( stop )

    def setStartRunTime( self, starttime, duration ):
      start = timestamp( parse( starttime ) )
      stop  = start + parseDuration( duration ) 

      self["Observation.startTime"] = format( start )
      self["Observation.stopTime"] = format( stop )

    def setClock( self, mhz ):
      self['Observation.sampleClock'] = int( mhz )

    def setIntegrationTime( self, integrationTime ):
      self["OLAP.Correlator.integrationTime"] = integrationTime

      # make sure these values will be recalculated in finalise()
      del self['OLAP.IONProc.integrationSteps']
      del self['OLAP.CNProc.integrationSteps']

    def getNrBeams( self ):
      rings =  int( self["OLAP.PencilInfo.nrRings"] )
      manual = int( self["OLAP.nrPencils"] )

      return 3 * rings * (rings + 1) + manual

    def getNrMergedStations( self ):
      tabList = self["OLAP.CNProc.tabList"]

      if not tabList:
        return len(self.stations)

      return max(tabList) + 1  

    def getNrBeamFiles( self ):
      if self.getBool("OLAP.outputBeamFormedData"):
        files = 2
      elif self.getBool("OLAP.outputCoherentStokes"):
        files = len(self["OLAP.Stokes.which"])
      else:
        files = 0

      if self.getBool("OLAP.PencilInfo.flysEye"):
        return self.getNrMergedStations() * files

      return self.getNrBeams() * files

    def phaseThreeExists( self ):  
      # NO support for mixing with Observation.mode and Observation.outputIncoherentStokesI
      output_keys = [
        "OLAP.outputBeamFormedData",
        "OLAP.outputCoherentStokes",
      ]

      for k in output_keys:
        if k in self and self.getBool(k):
          return True

      return False

    def phaseThreePsetDisjunct( self ):
      phase1 = set(self.getInt32Vector("OLAP.CNProc.phaseOnePsets"))
      phase2 = set(self.getInt32Vector("OLAP.CNProc.phaseTwoPsets"))
      phase3 = set(self.getInt32Vector("OLAP.CNProc.phaseThreePsets"))

      return len(phase1.intersection(phase3)) == 0 and len(phase2.intersection(phase3)) == 0

    def phaseThreeCoreDisjunct( self ):
      phase12 = set(self.getInt32Vector("OLAP.CNProc.phaseOneTwoCores"))
      phase3 = set(self.getInt32Vector("OLAP.CNProc.phaseThreeCores"))

      return len(phase12.intersection(phase3)) == 0

    def phaseTwoThreePsetEqual( self ):
      phase2 = set(self.getInt32Vector("OLAP.CNProc.phaseTwoPsets"))
      phase3 = set(self.getInt32Vector("OLAP.CNProc.phaseThreePsets"))

      return phase2 == phase3

    def phaseOneTwoThreeCoreEqual( self ):
      phase12 = set(self.getInt32Vector("OLAP.CNProc.phaseOneTwoCores"))
      phase3 = set(self.getInt32Vector("OLAP.CNProc.phaseThreeCores"))

      return phase12 == phase3

    def getNrOutputs( self ):
      # NO support for mixing with Observation.mode and Observation.outputIncoherentStokesI
      output_keys = [
        "OLAP.outputFilteredData",
        "OLAP.outputCorrelatedData",
        "OLAP.outputBeamFormedData",
        "OLAP.outputCoherentStokes",
        "OLAP.outputIncoherentStokes",
      ]

      return sum( (1 for k in output_keys if k in self and self.getBool(k)) )

    def check( self ):
      """ Check the Parset configuration for inconsistencies. """

      def getBool(k):
        """ A getBool() routine with False as a default value. """
        return k in self and self.getBool(k)

      assert self.getNrOutputs() > 0, "No data output selected."
      assert len(self.stations) > 0, "No stations selected."
      assert len(self.getInt32Vector("Observation.subbandList")) > 0, "No subbands selected."

      # phase 2 and 3 are either disjunct or equal
      assert self.phaseThreePsetDisjunct() or self.phaseTwoThreePsetEqual(), "Phase 2 and 3 should use either disjunct or the same psets."
      assert self.phaseThreeCoreDisjunct() or self.phaseOneTwoThreeCoreEqual(), "Phase 1+2 and 3 should use either disjunct or the same cores."
      assert not (self.phaseThreePsetDisjunct() and self.phaseThreeCoreDisjunct()), "Phase 3 should use either disjunct psets or cores."

      # no both bf complex voltages and stokes
      assert not (getBool("OLAP.outputBeamFormedData") and getBool("OLAP.outputCoherentStokes")), "Cannot output both complex voltages and coherent stokes."

      # restrictions on #samples and integration in beam forming modes
      if self.getBool("OLAP.outputBeamFormedData") or self.getBool("OLAP.outputCoherentStokes"):
        # beamforming needs a multiple of 16 samples
        assert int(self["OLAP.CNProc.integrationSteps"]) % 16 == 0, "OLAP.CNProc.integrationSteps should be dividable by 16"

        assert int(self["OLAP.CNProc.integrationSteps"]) % int(self["OLAP.Stokes.integrationSteps"]) == 0, "OLAP.CNProc.integrationSteps should be dividable by OLAP.Stokes.integrationSteps"

        # create at least 1 beam
        assert self.getNrBeams() > 0, "Beam forming requested, but no beams defined."

      if self.getBool("OLAP.outputCoherentStokes"):
        assert int(self["OLAP.CNProc.integrationSteps"]) >= 4, "OLAP.CNProc.integrationSteps should be at least 4 if coherent stokes are requested"

      # verify start/stop times
      assert self["Observation.startTime"] < self["Observation.stopTime"], "Start time (%s) must be before stop time (%s)" % (self["Observation.startTime"],self["Observation.stopTime"])

      if self.getBool( "OLAP.realTime" ):
        assert timestamp(parse(self["Observation.startTime"])) > time.time(), "Observation.realTime is set, so start time (%s) should be later than now (%s)" % (self["Observation.startTime"],format(time.time()))

      # verify stations
      for s in self.stations:
        stationName = s.name.split("_")[0] # remove specific antenna or array name (_hba0 etc) if present
        assert "PIC.Core.%s.phaseCenter" % (stationName,) in self, "Phase center of station '%s' not present in parset." % (stationName,)

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import sys

  # parse the command line
  parser = OptionParser( "usage: %prog [options] parset [parset ...]" )

  # parse arguments
  (options, args) = parser.parse_args()
  
  if not args:
    parser.print_help()
    sys.exit(0)

  parset = Parset()

  for files in args:
    parset.readFile( files )

  parset.postRead()
  parset.preWrite()
  parset.writeFile( "-" )  

  sys.exit(0)
