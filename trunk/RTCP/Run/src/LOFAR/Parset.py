import datetime
import time
import socket
import util.Parset
from Partitions import PartitionPsets
from util.dateutil import parse,format,parseDuration,timestamp
import math
import sys
from sets import Set

class Parset(util.Parset.Parset):
    def __init__(self):
        util.Parset.Parset.__init__(self)

	self.stations = []
	self.storagenodes = []
	self.partition = ""
        self.integrationtime = 1.0
	self.psets = []

        self.filename = ""

    def setFilename( self, filename ):
       self.filename = filename

    def getFilename( self ):
       return self.filename

    def save(self):
       self.writeFile( self.filename )

    def distillStations(self):
        """ Distill station names to use from the parset file and return them. """

        def stripQuotes(s):
          return s.strip("'").rstrip("'")

        if "OLAP.storageStationNames" not in self:
          return ""

        return map( stripQuotes, self.getStringVector( "OLAP.storageStationNames" ) )

    def distillPartition(self):
        """ Distill partition to use from the parset file and return it. """

        if "OLAP.CNProc.partition" not in self:
          return ""

        return self["OLAP.CNProc.partition"]

    def finalise(self):
        """ Derive some final keys and finalise any parameters necessary
	    before writing the parset to disk. """

	# override some erroneous values
	self["OLAP.DelayComp.positionType"] = "ITRF"    

	# TODO: we use self.setdefault, but this can create inconsistencies if we
	# set one value but not the other in a pair of interdependent parameters.
	# This should possibly be detected in the check routine, but it seems
	# sloppy to let it pass through here unnoticed.

	# input flow configuration
        self.setdefault('OLAP.nrRSPboards', len(self.stations))
	
	for station in self.stations:
	  self.setdefault('PIC.Core.Station.%s.RSP.ports' % (station.name,), station.inputs)

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
	else:  
	  index = 0
	  subbands = Set()

	  while "Observation.Beam[%s].subbandList" % (index,) in self:
	    subbandList = self.getInt32Vector("Observation.Beam[%s].subbandList" % (index,))
	    subbands.update( subbandList )

	    index += 1

	  nrSubbands = len(subbands)
		
	  self.setdefault('Observation.nrBeams', index)
	
	# Pset configuration
	self['OLAP.CNProc.partition'] = self.partition

	nrPsets = len(self.psets)
	nrStorageNodes = self.getNrUsedStorageNodes()

	self.setdefault('OLAP.nrPsets', nrPsets)
	self.setdefault('OLAP.CNProc.inputPsets', [s.getPsetIndex(self.partition) for s in self.stations])
	self.setdefault('OLAP.CNProc.outputPsets', range(nrPsets))

	self.setdefault('OLAP.subbandsPerPset', int( math.ceil(1.0 * nrSubbands / nrPsets) ) )
	if nrSubbands == 1 or nrStorageNodes == 0:
	  self.setdefault('OLAP.psetsPerStorage', 1 )
	  self.setdefault('OLAP.storageNodeList',[0] * nrSubbands)
	else:  
	  self.setdefault('OLAP.psetsPerStorage', int( math.ceil(1.0 * nrPsets / nrStorageNodes) ) )
	  self.setdefault('OLAP.storageNodeList',[i//(nrSubbands//nrStorageNodes) for i in xrange(0,nrSubbands)])
	
	#print 'nrSubbands = ' + str(nrSubbands) + ', nrStorageNodes = ' + str(nrStorageNodes) + ', subbandsPerPset = ' + str(self.getSubbandsPerPset())

	#print 'storageNodes: ' + str(self['OLAP.storageNodeList'])

	# calculation configuration

        # integration times of CNProc and IONProc, based on self.integrationtime
        # maximum amount of time CNProc can integrate
        maxCnIntegrationTime = 1.2 # seconds

        # (minimal) number of times the IONProc will have to integrate
        ionIntegrationSteps = int(math.ceil(self.integrationtime / maxCnIntegrationTime))
        self.setdefault('OLAP.IONProc.integrationSteps', ionIntegrationSteps)

        # the amount of time CNProc will integrate, translated into samples
        cnIntegrationTime = self.integrationtime / int(self["OLAP.IONProc.integrationSteps"])
        nrSamplesPerSecond = int(self['Observation.sampleClock']) * 1e6 / 1024 / int(self['Observation.channelsPerSubband'])

        cnIntegrationSteps = int(round(nrSamplesPerSecond * cnIntegrationTime / 16)) * 16
        self.setdefault('OLAP.CNProc.integrationSteps', cnIntegrationSteps)

        # observation mode
	self.setdefault('Observation.pulsar.mode',0)

	# tied-array beam forming
        tiedArrayStationList = []
	tabList = []
	beamFormedStations = []
	index = 0
	
	while 'Observation.Beamformer[%s].stationList' % (index,) in self:
	    curlist = self.getString('Observation.Beamformer[%s].stationList' % (index,))

            # remove any initial or trailing "
	    curlist = curlist.strip('"').rstrip('"')

            # transform , to +
	    curlist = curlist.replace(',','+')

	    tiedArrayStationList.append(curlist)

            # extract the individual stations and remove them from the global station list
	    beamFormedStations += curlist.split('+')
	    remainingStationList
	    for sp in curlist.split('+'):
		for s in remainingStationList:
		    if s.getName() == sp:
		       remainingStationList.remove(s)
		       break

	    index += 1
	
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

    def setStations(self,stations):
	""" Define the set of stations to use. """
	
	self.stations = stations
	
    def setPartition(self,partition):
	""" Define the partition to use. """

	assert partition in PartitionPsets, "Partition %s unknown. Run LOFAR/Partitions.py to get a list of valid partitions." % (partition,)
	
	self.partition = partition
	self.psets = PartitionPsets[partition]

    def setStorageNodes(self,storagenodes):
	""" Define the list of storage nodes to use. """

        # resolve host names, as OLAP needs IP addresses
	self.storagenodes = map(socket.gethostbyname, storagenodes)

        self["OLAP.OLAP_Conn.IONProc_Storage_ServerHosts"] = self.storagenodes

    def setObsID(self,obsid):
        self["Observation.ObsID"] = obsid	

    def getObsID(self):
        assert "Observation.ObsID" in self, "Observation ID not generated yet."

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
        mask = mask.replace( "${%s}" % d, str(date[index]) )

      mask = mask.replace( "${MSNUMBER}", "%05d" % (self.getObsID(),) )
      mask = mask.replace( "${SUBBAND}", "*" )

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
      self.integrationtime = float( integrationTime )

      # make sure these values will be recalculated in finalise()
      del self['OLAP.IONProc.integrationSteps']
      del self['OLAP.CNProc.integrationSteps']

    def getStoragePorts( self ):
      """ Returns a dictionary of the ports (value) required by each storage node (key). """

      globalPorts = self.getInt32Vector("OLAP.OLAP_Conn.IONProc_Storage_Ports")
      storageNodes = self.getStringVector("OLAP.OLAP_Conn.IONProc_Storage_ServerHosts")
      subbandMapping = self.getInt32Vector("OLAP.storageNodeList")

      localPorts = {}

      for s in storageNodes:
        localPorts[s] = []

      for i,s in enumerate(subbandMapping):
        node = storageNodes[s]
        portnr = globalPorts[i]

        localPorts[node].append(portnr)

      return localPorts

    def disableStoragePorts( self, reservedPorts ):
      """ Prevents the use of a certain subset of ports for Storage. """

      portkey = "OLAP.OLAP_Conn.IONProc_Storage_Ports"
      myports = filter( lambda x: x not in reservedPorts, self.getInt32Vector(portkey) )
      self[portkey] = myports

    def check( self ):
      """ Check the Parset configuration for inconsistencies. """

      # verify start/stop times
      assert self["Observation.startTime"] < self["Observation.stopTime"], "Start time (%s) must be before stop time (%s)" % (self["Observation.startTime"],self["Observation.stopTime"])

      if self.getBool( "OLAP.realTime" ):
        assert timestamp(parse(self["Observation.startTime"])) > time.time(), "Observation.realTime is set, so start time (%s) should be later than now (%s)" % (self["Observation.startTime"],format(time.time()))

      # verify stations
      for s in self.stations:
        stationName = s.name.split("_")[0] # remove specific antenna or array name (_hba0 etc) if present
        assert "PIC.Core.%s.phaseCenter" % (stationName,) in self, "Phase center of station '%s' not present in parset." % (stationName,)
