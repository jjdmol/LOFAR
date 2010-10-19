import time
import datetime
import LOFAR_Parset
import math
import sys
import copy
import string
from Numeric import zeros
from CS1_Hosts import *

class CS1_Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
        self.inputFromRaw = False
        self.stationList = list()
	self.bl2beams = list()
	self.bl2subbands = list()
	self.sb2index = list()
	self.nrSubbands = 0
	self.doObservation =False

    def setClock(self, clock):
        self.clock = clock
        
        if self.clock == '160MHz':
	    self['Observation.sampleClock'] = 160
	    self['OLAP.BGLProc.integrationSteps'] = 608
        elif self.clock == '200MHz':
	    self['Observation.sampleClock'] = 200
	    self['OLAP.BGLProc.integrationSteps'] = 768
	    #self['OLAP.BGLProc.integrationSteps'] = 384
	    #self['OLAP.BGLProc.integrationSteps'] = 192
        self.updateSBValues()
       
    def getClockString(self):
        return self.clock

    def setStations(self, stationList):
        self.stationList = stationList
        self['OLAP.nrRSPboards'] = len(stationList)
        self['OLAP.storageStationNames'] = [s.getName() for s in stationList]
    
    def setPartition(self, partition):
        self.partition = partition
	self['OLAP.BGLProc.partition'] = partition
	
    def getPartition(self):
        return self.partition
	
    def setInputToMem(self):
        self.inputFromMemory = True
	self['OLAP.OLAP_Conn.station_Input_Transport'] = 'NULL'

    def getInputNodes(self):
        inputNodelist = list()
	
	for s in self.stationList:
	    destPorts = self.getStringVector('PIC.Core.' + string.split(s.getName(), '_')[0] + '_RSP.dest.ports')[self.getInt32('PIC.Core.' + s.getName() + '.RSP')]
	    destPorts =  destPorts.strip().strip('[').rstrip(']')
	    dest = string.split(destPorts, ':')[0]
	    inputNodelist.append(string.split(destPorts, ':')[0])
	return inputNodelist    
	
    def getNStations(self):
        return int(len(self.stationList))
        
    def setInterval(self, start, duration):
        self['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
        self['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration)

    def setIntegrationTime(self, integrationTime):
	self['OLAP.IONProc.integrationSteps']     = integrationTime
	self['OLAP.StorageProc.integrationSteps'] = 1

    def setMSName(self, msName):
        self['Observation.MSNameMask'] = msName

    def getMSName(self):
        return self['Observation.MSNameMask']

    def getNPsets(self):
        subbands = self.getNrSubbands()
        subbandsperpset = self.getInt32('OLAP.subbandsPerPset')
        return subbands / subbandsperpset

    def find_first_of(self, key):
	digit = False
        for s, result in [('0', ['0']), ('1', ['1']),('2', ['2']),('3', ['3']),('4', ['4']),('5', ['5']),('6', ['6']),('7', ['7']),('8', ['8']),('9', ['9'])]:
	    if (key[0] == s):
	        digit = True
	        break;

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
		
		if (lastVal < firstVal):
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

    def getNrSubbands(self):
	return len(self.getInt32Vector('Observation.subbandList'))
        #if not self.doObservation:
	    #self.observation()
	#return self.nrSubbands   
	
    def observation(self):
	return
        self.doObservation = True
        bl2beamsArray = zeros(4*54)
	bl2subbandsArray = zeros(4*54)
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
	        print 'NrBeamlets(%d)' % len(beamlets) + ' and nrSubbands(%d)' % len(subbands) + ' are not equal'
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
	
	# nrSubbands, check the number of subbands of each RSP
	nSubbands = 0
	for i in range(0,4*54):
	    if bl2subbandsArray[i] != -1:
	        nSubbands += 1
	
	self.bl2beams = bl2beamsArray
	self.bl2subbands = bl2subbandsArray
	self.sb2index = sb2indexArray
	self.nrSubbands = nSubbands
    	
    def subbandList(self):
        sbList = list()
        
	b2s = self.getBeamlet2subbands()
	for i in range(0,4*54):
	    if b2s[i] != -1:
	        sbList.append(b2s[i])
	    
	return sbList    

    def nrBeams(self, index):
        MAX_BEAMLETS_PER_RSP = 54
	begin = index * MAX_BEAMLETS_PER_RSP
	end = begin + MAX_BEAMLETS_PER_RSP-1
	nbeams = 0
        
	for i in range(begin, end):
	    if self.bl2beams[i] != 0:
	       nbeams += 1 
	
	return nbeams   
	    
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
    
    def checkCS1Parset(self):
	print 'checkCS1Parset skipped'
        #if not self.isDefined('Observation.Beam[1].beamletList') and self.getInt32('Observation.nrBeams') == 1:
	    #sbString = self.expandedArrayString(self.getString('Observation.Beam[1].subbandList'))
	    #self.__setitem__('x',sbString);
	    #subbands = self.getInt32Vector('x')
	    #self['Observation.Beam[1].beamletList'] = '[0..%s]' % str(len(subbands) - 1)
		    
	#b2b = self.getBeamlet2beams()
        #nBeamlets = 0
	#for i in range(0,53):
	    #if b2b[i] != 0:
	        #nBeamlets += 1

	#if nBeamlets > self.getInt32('OLAP.nrSubbandsPerFrame'):
	    #print 'NrBeamlets(%d)' % nBeamlets + ' > OLAP.nrSubbandsPerFrame(%d)' %  self.getInt32('OLAP.nrSubbandsPerFrame')
	    #sys.exit(0)

    def addkeys_IONodeRSP(self):
	interfaces = IONodes.get(self.partition)
	dplists = [[] for i in range(len(interfaces))]
	
	for s in self.stationList:
	    dest_ports = self.getStringVector_new('PIC.Core.' + s.getName() + '_RSP.dest.ports')
	    for dp in dest_ports:
		if (dp.find(':') == -1) or (dp[0:5] == 'file:') or (dp[0:5] == 'FILE:'):
		    found = False
		    for i in range(0, len(interfaces)):
		        if not found and self.isDefined('PIC.Core.IONode[%d].RSP' % i):
			    for sname in self.getStringVector_new('PIC.Core.IONode[%d].RSP' % i):
			        if sname == s.getName()+ '_RSP%d' % dest_ports.index(dp):
				    found = True
				    break
				    
		    if not found:
			print 'Warning: missing PIC.Core.IONode[x].RSP for station: ' + s.getName() + '_RSP%d("%s")' % (dest_ports.index(dp), dp) + ' in CS1.parset'
		else:		    		    
		    if (dp[0:4] == 'udp:') or (dp[0:4] == 'UDP:'):
			index = interfaces.index(string.split(dp.strip(dp[0:4]), ':')[0])
		    elif (dp[0:4] == 'tcp:') or (dp[0:4] == 'TCP:'):
		        index = interfaces.index(string.split(dp.strip(dp[0:4]), ':')[0])
		    else:
		        index = interfaces.index(string.split(dp, ':')[0])	
		    
		    if self.nrBeams(dest_ports.index(dp)) > 0:
		        dplists[index].append(s.getName() + '_RSP%d' % dest_ports.index(dp))
	
	count = 0
	for dp in dplists:
	    if self.isDefined('PIC.Core.IONode[%d].RSP' % count):
		if len(self.getStringVector_new('PIC.Core.IONode[%d].RSP' % count)) != len(self.stationList):
		    print 'nrStations in PIC.Core.IONode[%d].RSP' % count + ' must be: %d' % len(self.stationList)
	            sys.exit(0)
	    else:
		if (len(dp) > 0) and (len(dp) != len(self.stationList)):
		    print 'Error: manually set PIC.Core.IONode[x].RSP and the station name(s) in CS1.parset'
	            sys.exit(0)
		else:
		    self['PIC.Core.IONode[%d].RSP' % count] = dp
	    count += 1
	
    def updateSBValues(self):
        if self.clock == '160MHz':
            subbandwidth = 156250
        elif self.clock == '200MHz':
            subbandwidth = 195312.5

	#note: this is only true when the number of subbands in the 4 ranges(0..53,54..107,108..161,162..215) are equal!
	#subbandIDs = self.subbandList()
        
	#nyquistZone = self.nyquistzoneFromFilter(self.getString('Observation.bandFilter'))
	
 	#sbs = list()
        #for sb in range(0, self.getNrSubbands()):
            #sbs.append((nyquistZone -1 )*(self.getInt32('Observation.sampleClock')*1000000/2)  + subbandIDs[sb] * subbandwidth)

        # create the frequencies for all subbands
        #self['Observation.RefFreqs'] = '[' + ', '.join(str(sb) for sb in sbs) + ']'
        #self['Observation.NSubbands'] = self.getNrSubbands()

        #the number of subbands should be divisible by the number of subbands per pset
        #if not self.getNrSubbands() % self.getInt32('OLAP.subbandsPerPset') == 0:
            #raise Exception('Number of subbandIDs(%d) in not divisible by the number of subbands per pset (%d).' % self.getNrSubbands(), self['OLAP.subbandsPerPset'])
            
