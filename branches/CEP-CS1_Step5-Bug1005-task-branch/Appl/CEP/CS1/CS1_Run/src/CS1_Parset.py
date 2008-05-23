import time
import datetime
import LOFAR_Parset
import math
import sys
import copy
import string
from Numeric import zeros
import Hosts
from CS1_Hosts import *

MAX_BEAMLETS_PER_RSP = 54

class CS1_Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
        self.inputFromRaw = False
        self.stationList = list()
	self.bl2beams = list()
	self.bl2subbands = list()
	self.sb2index = list()
	self.doObservation =False

    def setClock(self, clock):
        self.clock = clock
        
        if self.clock == '160MHz':
	    self['Observation.sampleClock'] = 160
	    self['OLAP.BGLProc.integrationSteps'] = 608
        elif self.clock == '200MHz':
	    self['Observation.sampleClock'] = 200
	    self['OLAP.BGLProc.integrationSteps'] = 768
	    #self['OLAP.BGLProc.integrationSteps'] = 16
        self.updateSBValues()
       
    def getClockString(self):
        return self.clock

    def subbandsPerPset(self, index):
        nrSubbands = self.getNrSubbands(index)
	if nrSubbands == 1 : return 1
	else:
	    pSets = (6, 5, 4, 3, 2, 1)
	    for p in pSets:
	        if nrSubbands % self.getInt32('OLAP.nrStorageNodes') != 0:
		    begin = index * MAX_BEAMLETS_PER_RSP
	            end = begin + MAX_BEAMLETS_PER_RSP-1
		    raise Exception('Use even numbers of subbands in beamlets %d-%d' %(begin,end) + ', except for nrSubbands = "1"')
		
		if nrSubbands/self.getInt32('OLAP.nrStorageNodes') % p == 0: 
		    if nrSubbands/p > self.maxNPsets(index):
		        begin = index * MAX_BEAMLETS_PER_RSP
	                end = begin + MAX_BEAMLETS_PER_RSP-1
			print 'Needs "%d"' % (nrSubbands/p) + ' psets; only "%d"' % (self.maxNPsets(index)) + ' available in beamlets %d-%d' %(begin,end)
			sys.exit(0)
		    return p	
    
    def psetsPerStorage(self, index):
        nrSubbands = self.getNrSubbands(index)
	if nrSubbands == 1 : return 1
	else:
	    subPerPset = self.subbandsPerPset(index)
	    return nrSubbands / subPerPset / self.getInt32('OLAP.nrStorageNodes')
    
    def partitionName(self, index):
	partition = self.getStringVector('Observation.VirtualInstrument.partitionList')[index]
	return partition.strip().strip('[').rstrip(']')
	
    def setStations(self, stationList):
        self.stationList = stationList
        self['OLAP.storageStationNames'] = [s.getName() for s in stationList]
    
    def setInputToMem(self):
        self.inputFromMemory = True
	self['OLAP.OLAP_Conn.station_Input_Transport'] = 'NULL'

    def getInputNodes(self,index):
        inputNodelist = list()
	
	for s in self.stationNames(index):
	    destPorts = self.getStringVector('PIC.Core.' + string.split(s, '_')[0] + '_RSP.dest.ports')[self.getInt32('PIC.Core.' + s + '.RSP')]
	    destPorts =  destPorts.strip().strip('[').rstrip(']')
	    dest = string.split(destPorts, ':')[0]
	    inputNodelist.append(string.split(destPorts, ':')[0])
	return inputNodelist    
	
    def getNStations(self,index):
        return int(len(self.stationNames(index)))
        
    def setInterval(self, start, duration):
        self['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
        self['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration)

    def setIntegrationTime(self, integrationTime):
	if self.getBool('OLAP.IONProc.useGather'):
	  self['OLAP.IONProc.integrationSteps']     = integrationTime
	  self['OLAP.StorageProc.integrationSteps'] = 1
	else:
	  self['OLAP.IONProc.integrationSteps']     = 1
	  self['OLAP.StorageProc.integrationSteps'] = integrationTime

    def setMSName(self, msName):
        self['Observation.MSNameMask'] = msName
    
    def maxNPsets(self, index):
        return len(IONodes.get(self.getStringVector('Observation.VirtualInstrument.partitionList')[index].strip().strip('[').rstrip(']')))
     
    def getMSName(self):
        return self['Observation.MSNameMask']

    def find_first_of(self, key):
	digit = False
        for s, result in [('0', ['0']), ('1', ['1']),('2', ['2']),('3', ['3']),('4', ['4']),('5', ['5']),('6', ['6']),('7', ['7']),('8', ['8']),('9', ['9'])]:
	    if (key[0] == s):
	        digit = True
	        break

	if not digit:
	        print key + ' is not an short value'
		sys.exit(0)
	
	
    def expandedArrayString(self, orgStr):
 	if (orgStr.find('..') == -1):
	    return orgStr    #no, just return original
	    
	baseString =  copy.deepcopy(orgStr)
	
	baseString = baseString.strip('[').rstrip(']')
	
	strList = string.split(baseString, ',')
	
	self.find_first_of(strList[0]) 
	
	if (orgStr.find('..') != -1):
	    nrDigits = (len(strList[0])-2)/2
	else:
	    nrDigits = len(strList[0])
	
	outMask = '%' + '0%ld' % nrDigits + 'ld'
	
	result = '['
	firstElem  = True;
	nrElems = len(strList);
	
	for idx in range(0, nrElems):
	    firstVal = '%ld' % int(strList[idx].split('..')[0])
	    
	    if (strList[idx].find('..') != -1):
	        rangePos = len(strList[idx].split('..')[1])
	    else:
	        rangePos = -1	
	    
	    if (rangePos == -1):
	        lastVal = firstVal
	    else:                   # yes, try to get second element.
	        lastVal = outMask % int(strList[idx].split('..')[1])
		
		if (int(lastVal) < int(firstVal)):
		    print 'Illegal range specified in ' + strList[idx] + '.'
	            sys.exit(0)
	    	
	    # finally construct one or more elements
	    for val in range(int(firstVal), int(lastVal)+1):
	        if firstElem:
		    result += str(val)
		    firstElem = False
		else:
		    result += ',' + str(val) 
	    
	return  result + ']'  	       

    def getBeamlet2beams(self):
        if not self.doObservation:
	    self.observation()
	return self.bl2beams
        	        
    def getBeamlet2subbands(self):
        if not self.doObservation:
	    self.observation()
	return self.bl2subbands    

    def getSubband2Index(self):
        if not self.doObservation:
	    self.observation()
	return self.sb2index    

    def getNrSubbands(self, index):
        begin = index * MAX_BEAMLETS_PER_RSP
	end = begin + MAX_BEAMLETS_PER_RSP-1
	
	if not self.doObservation:
	    self.observation()
	
	nSubbands = 0
	for i in range(begin,end):
	    if self.bl2beams[i] != 0:
	        nSubbands += 1
	return nSubbands
	
    def observation(self):
        self.doObservation = True
        bl2beamsArray = zeros(4*MAX_BEAMLETS_PER_RSP)
	bl2subbandsArray = zeros(4*MAX_BEAMLETS_PER_RSP)
	sb2indexArray = list()
	
	for i in range(0, len(bl2subbandsArray)):
	    bl2subbandsArray[i] = -1

        for beam in range(1, self.getInt32('Observation.nrBeams')+1):
            beamPrefix = 'Observation.Beam[%d].' % beam
	    
	    sbString = self.expandedArrayString(self.getString(beamPrefix + 'subbandList'))
	    self.__setitem__('x',sbString);
	    subbands = self.getInt32Vector('x')
	    
	    blString = self.expandedArrayString(self.getString(beamPrefix + 'beamletList'))
	    self.__setitem__('x',blString);
	    beamlets = self.getInt32Vector('x')
	    
	    if len(subbands) != len(beamlets):
	        print 'Number of subbands("%d")' % len(subbands) + ' != number of beamlets("%d")' % len(beamlets) + ' in beam ' + str(beam)
	        sys.exit(0)
		
	    for b in range(0, len(beamlets)):
	        # finally update beamlet 2 beam mapping.
		if bl2beamsArray[beamlets[b]] != 0:
		    print 'beamlet ' + str(b) + " of beam " + str(beam) + ' clashes with beamlet of other beam'
		    sys.exit(0)
		bl2beamsArray[beamlets[b]] = beam;
		bl2subbandsArray[beamlets[b]] = subbands[b]
	
	# subband2index	
	for i in range(0, len(bl2subbandsArray)):
	    if bl2subbandsArray[i] != -1:
	        sb2indexArray.append(i)
	
	self.bl2beams = bl2beamsArray
	self.bl2subbands = bl2subbandsArray
	self.sb2index = sb2indexArray
    	
    def nyquistzoneFromFilter(self, filterName):
    
        if filterName == 'LBL_10_80' : return 1
	if filterName == 'LBL_30_80' : return 1
	if filterName == 'LBH_10_80' : return 1
	if filterName == 'LBH_30_80' : return 1
	if filterName == 'HB_100_190': return 2
	if filterName == 'HB_170_230': return 3
	if filterName == 'HB_210_240': return 3
	
	print 'filterselection value "' + filterName + '" not recognized, using LBL_10_80'
	return 1
    
    def stationNames(self,index):
        sNames = list()
 	for s in self.stationList:
	    if self.getInt32('PIC.Core.' +  s.getName() + '.RSP') == index:
		sNames.append(s.getName())
	
	return sNames  
  
    def parseParset(self):
        for i in range(0, len(self.getStringVector('Observation.VirtualInstrument.partitionList'))):
	    if self.getNrSubbands(i) > 0 and len(self.stationNames(i)) == 0:
	        begin = i * MAX_BEAMLETS_PER_RSP
	        end = begin + MAX_BEAMLETS_PER_RSP-1
	        print 'No station name(s) selected in beamlets %d-%d' %(begin,end)
		sys.exit(0)
	    
        b2b = self.getBeamlet2beams()
	for i in range(0, len(self.getStringVector('Observation.VirtualInstrument.partitionList'))):
	    begin = i * MAX_BEAMLETS_PER_RSP
	    end = begin + MAX_BEAMLETS_PER_RSP-1
	    nBeamlets = 0
	    for j in range(begin,end):
	        if b2b[j] != 0:
		    nBeamlets += 1
		    
	    if nBeamlets > self.getInt32('OLAP.nrSubbandsPerFrame'):
	        print 'NrBeamlets("%d")' % nBeamlets + ' > OLAP.nrSubbandsPerFrame("%d")' %  self.getInt32('OLAP.nrSubbandsPerFrame')
	        sys.exit(0)
	
    def updateSBValues(self):
        if self.clock == '160MHz':
            subbandwidth = 156250
        elif self.clock == '200MHz':
            subbandwidth = 195312.5

	subbandIDs = self.getBeamlet2subbands()
	nyquistZone = self.nyquistzoneFromFilter(self.getString('Observation.bandFilter'))
	refFreqList = zeros(4*MAX_BEAMLETS_PER_RSP)
	for i in range(0, len(refFreqList)):
	    refFreqList[i] = -1

	for i in range(0, len(subbandIDs)):
	   if subbandIDs[i] != -1:
	       refFreqList[i] = ((nyquistZone -1 )*(self.getInt32('Observation.sampleClock')*1000000/2)  + subbandIDs[i] * subbandwidth)
	
	# create the frequencies for all subbands
        self['Observation.RefFreqs'] = '[' + ', '.join(str(refFreg) for refFreg in refFreqList) + ']'
	
	nSubband = 0
	for i in range(0, len(self.getStringVector('Observation.VirtualInstrument.partitionList'))):
	    #the number of subbands should be dividable by the number of subbands per pset
	    if not self.getNrSubbands(i) % self.subbandsPerPset(i) == 0:
	        raise Exception('Number of subbandIDs("%d") in not dividable by the number of subbands per pset ("%d").' % (self.getNrSubbands(i), self.subbandsPerPset(i)))
	    
	    nSubband = nSubband + self.getNrSubbands(i)
	
	self['Observation.NSubbands'] = nSubband
