import time
import datetime
import LOFAR_Parset
import math
import sys
import copy
import string
import struct
from Numeric import zeros
from Host_Names import *
from sets import Set

class StationRSPpair(object):
    def __init__(self, station, rsp):
        self.station	= station
        self.rsp	= rsp

    def getStation(self):
        return self.station

    def getRsp(self):
	return self.rsp
	
    def setStation(self, station):
        self.station = station

    def setRsp(self, rsp):
	self.rsp = rsp
	
class Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
        self.inputFromRaw = False
        self.stationList = list()

    def setClock(self, clock):
        self.clock = clock
        
        if self.clock == '160MHz':
	    self['Observation.sampleClock'] = 160
        elif self.clock == '200MHz':
	    self['Observation.sampleClock'] = 200
       
	self['OLAP.CNProc.integrationSteps'] = int(round(self['Observation.sampleClock'] * 1e6 / 1024 / int(self['Observation.channelsPerSubband']) / 16)) * 16

    def getClockString(self):
        return self.clock

    def subbandsPerPset(self):
	if not self.isDefined('OLAP.subbandsPerPset'):
	    if self.getNrSubbands() == 1 : self['OLAP.subbandsPerPset'] = 1
	    elif self.getNrSubbands() % 2 != 0:
	        print 'Number of subbands(%d) is not even.' %(nrSubbands)
		sys.exit(0)
	    else:
		for pSets in range(len(IONodes.get(self.getPartition())), 0 , -1):
		    if self.getNrSubbands() % pSets == 0:
			self['OLAP.subbandsPerPset'] = self.getNrSubbands() / pSets
			break
	return self.getInt32('OLAP.subbandsPerPset')		
    
    def nrUsedStorageNodes(self):
        nrStorageNodes = len(listfen.getSlaves())
	nrPSets = self.getNrSubbands()/self.subbandsPerPset()
	
	for nrNodes in range(nrStorageNodes, 0, -1):
	    if nrPSets % nrNodes == 0:
	        return nrNodes
	
    def psetsPerStorage(self):
        if not self.isDefined('OLAP.psetsPerStorage'):
	    if self.getNrSubbands() == 1 : self['OLAP.psetsPerStorage'] = 1
	    else:
	        nrPSets = self.getNrSubbands()/self.subbandsPerPset()
		nrStorageNodes = self.nrUsedStorageNodes()
		self['OLAP.psetsPerStorage'] = nrPSets/nrStorageNodes

    def storageNodeList(self):
	nrSubbands = self.getNrSubbands()
	nrStorageNodes = self.nrUsedStorageNodes()
	storageNodes = list()
	
	if nrSubbands == 1 : self['OLAP.storageNodeList'] = '[0]'
	else:
	    for i in range(0,nrSubbands):
 		storageNodes.append(i/(nrSubbands/nrStorageNodes))
	
	    self['OLAP.storageNodeList'] = [s for s in storageNodes]

    def setStations(self, stationList):
        self.stationList = stationList
        self['OLAP.nrRSPboards'] = len(stationList)
        self['OLAP.storageStationNames'] = [s.getName() for s in stationList]
	
	for station in stationList:
	  self['PIC.Core.Station.' + station.name + '.RSP.ports'] = station.inputs
	for pset in range(len(IONodes.get(self.partition))):
	  self['PIC.Core.IONProc.' + self.partition + '[' + str(pset) + '].inputs'] = [station.name + '/RSP' + str(rsp) for station in stationList if station.getPset(self.partition) == pset for rsp in range(len(station.inputs))]

    def setTiedArrayStations(self):
        tiedArrayStationList = list()
 	stationList = copy.deepcopy(self.stationList)
	beamform = False
	index=0
	
	while self.isDefined('Observation.Beamformer[' + str(index) + '].stationList'):
	    beamform = True
	    tiedArrayStationList.append(string.replace(self.getString('Observation.Beamformer[' + str(index) + '].stationList').strip('"').rstrip('"'),',','+'))
	    superStations = self.getString('Observation.Beamformer[' + str(index) + '].stationList').strip('"').rstrip('"').split(',')
	    for sp in superStations:
		for s in stationList:
		    if s.getName() == sp:
		       stationList.remove(s)
		       break
	    index +=1
	
	if (beamform):
	    for p in stationList:
	        tiedArrayStationList.append(p.getName())
	
	self['OLAP.tiedArrayStationNames'] = [sp for sp in tiedArrayStationList]

    def tabMapping(self):
	tabList = list()
	
	if len(self.getStringVector('OLAP.tiedArrayStationNames')) > 0:
	    for s in self.stationList:
	        tabStations = self.getStringVector('OLAP.tiedArrayStationNames')
	        for tab in tabStations:
		    if tab.find('+') != -1 :
		       stations = tab.split('+')
		       for st in stations:
		           if s.getName() == st:
		               tabList.append(tabStations.index(tab))
		    else:
		       if s.getName() == tab:
		         tabList.append(tabStations.index(tab))  
	    
	    if len(tabList) != len(self.stationList):
	        print 'tabList contains wrong number (' + str(len(tabList)) + ') of elements (expected '+ str(len(self.stationList)) + ')'
		sys.exit(0)
		
	self['OLAP.CNProc.tabList'] = [tab for tab in tabList]
	
    def getStations(self):
	return self.stationList
    
    def setPartition(self, partition):
	if partition not in IONodes:
		print 'Partition',partition,'not defined in IONodes (Host_Names.py)'
		sys.exit(0)

	self.partition = partition
	self['OLAP.CNProc.partition'] = partition	
	
    def getPartition(self):
        return self.partition
	
    def setInputToMem(self):
        self.inputFromMemory = True
	self['OLAP.OLAP_Conn.station_Input_Transport'] = 'NULL'

    def getNStations(self):
        return int(len(self.stationList))
        
    def setInterval(self, start, duration):
        self['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
        self['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration)

    def setIntegrationTime(self, integrationTime):
	self['OLAP.IONProc.integrationSteps']     = integrationTime

    def setMSName(self, msName):
        self['Observation.MSNameMask'] = msName

    def getMSName(self):
        return self['Observation.MSNameMask']

    def getNPsets(self):
        subbands = self.getNrSubbands()
        subbandsperpset = self.getInt32('OLAP.subbandsPerPset')
        return subbands / subbandsperpset
    
    def nrSlotsInFrame(self):
        return self.getInt32('Observation.nrSlotsInFrame')
    
    def subbandToRSPboardMapping(self):
        return self.getExpandedInt32Vector('Observation.rspBoardList')
    
    def subbandToRSPslotMapping(self):
        return self.getExpandedInt32Vector('Observation.rspSlotList')
    
    def getNrSubbands(self):
        if self.isDefined('Observation.subbandList'):
	    return len(self.getExpandedInt32Vector('Observation.subbandList'))
	else:
	    index=0
	    subbands = Set() 
	    while self.isDefined('Observation.Beam[' + str(index) + '].subbandList'):
	        subbandList = self.getExpandedInt32Vector('Observation.Beam[' + str(index) + '].subbandList')
		for s in subbandList:
		    subbands.add(s)
		index +=1
		
	    if not self.isDefined('Observation.nrBeams'):
	      	self['Observation.nrBeams'] = index
	
	    return len(subbands)

    def getStationNamesAndRSPboardNumbers(self, psetNumber):
	inputs = self.getStringVector('PIC.Core.IONProc.' + self.getPartition() + '[' + str(psetNumber) + '].inputs')
	stationsAndRSPs = list()   
	for i in range(0, len(inputs)):
            split = inputs[i].split('/')
	    if len(split) != 2 or split[1][0:3] != 'RSP':
	        print 'expected stationname/RSPn pair in "%s"' % inputs[i]
	        sys.exit(0)
            
	    stationsAndRSPs.append(StationRSPpair(split[0],split[1][3]))
	
	return stationsAndRSPs
    
    def checkBeamformList(self):
	index = 0

	while self.isDefined('Observation.Beamformer[' + str(index) + '].stationList'):
	    superStations = self.getString('Observation.Beamformer[' + str(index) + '].stationList').strip('"').rstrip('"').split(',')    	
            for sp in superStations:
	        found = False
		for s in self.stationList:
		    if sp == s.getName():
		        found = True
		        break
		if not (found):
		    print 'invalid value: Observation.Beamformer[%d].stationList = ' % index + sp + ', or the station isn\'t found in the stationList.'
		    sys.exit(0)
	    index +=1
	    
    def checkSubbandCount(self, key):
        if len(self.getExpandedInt32Vector(key)) != self.getNrSubbands():
	    print key + ' contains wrong number (' + str(len(self.getExpandedInt32Vector(key))) + ') of subbands (expected ' + str(self.getNrSubbands()) + ')'
	    sys.exit(0)
	    
    def check(self):
        if self.isDefined('Observation.subbandList'):
            self.checkSubbandCount('Observation.beamList');
            self.checkSubbandCount('Observation.rspBoardList');
            self.checkSubbandCount('Observation.rspSlotList');
	
	    slotsPerFrame        = self.nrSlotsInFrame()
	    boards		 = self.subbandToRSPboardMapping()
	    slots		 = self.subbandToRSPslotMapping()
	
	    for subband in range(0, len(slots)):
	        if slots[subband] >= slotsPerFrame:
	            print 'Observation.rspSlotList contains slot numbers >= Observation.nrSlotsInFrame'
		    sys.exit(0)

    def inputPsets(self):
        return self.getInt32Vector('OLAP.CNProc.inputPsets')
	    
    def checkRspBoardList(self):
        if self.isDefined('Observation.subbandList'):
	    inputs		 = self.inputPsets()
	    boards		 = self.subbandToRSPboardMapping()
	
	    for pset in inputs:
	        nrRSPboards = len(self.getStationNamesAndRSPboardNumbers(pset))
	        for subband in range(0, len(boards)):
		    if boards[subband] >= nrRSPboards:
		        print 'Observation.rspBoardList contains rsp board numbers that do not exist'
		        sys.exit(0)
		
	
	
	
	
	
    
    
