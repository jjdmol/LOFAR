#!/usr/bin/env python

import math
import time
import datetime
import os
import sys
import copy

class CS1_Parset(object):

    def __init__(self):
        self.stationList = list()
	self.parameters = dict()

    def readFromFile(self, fileName):
        lastline = ''
        for line in open(fileName, 'r').readlines():
            lastline = lastline + line.split('#')[0]
            lastline = lastline.rstrip()
            if len(lastline) > 0 and lastline[-1] == '\\':
                lastline = lastline[:-1]
            elif '=' in lastline:
                key, value = lastline.split('=')
                self.parameters[key.strip()] = value.strip()
                lastline = ''

    def writeToFile(self, fileName):
        outf = open(fileName, 'w')
	items = self.parameters.items()
	items.sort()
	for key, value in items:
            outf.write(key + ' = ' + str(value) + '\n')
        outf.close()

    def __contains__(self, key):
        return key in self.parameters
        
    def __setitem__(self, key, value):
        self.parameters[key] = value

    def __getitem__(self, key):
        return self.parameters[key]

    def getInt32Vector(self, key):
        ln = self.parameters[key]
	ln_tmp = ln.split('[')
        line = 	ln_tmp[1].split(']')
        return [int(lp) for lp in line[0].split(',')]

    def getInt32(self, key):
        return int(self.parameters[key])

    def getStringVector(self, key):
        line = self.parameters[key]
        line.strip('[').strip(']')
        return line.split(',')

    def getString(self, key):
        return self.parameters[key]

    def getFloat(self, key):
        return float(self.parameters[key])

    def getBool(self, key):
        return self.parameters[key] == 'true'

class ClusterSlave(object):
    def __init__(self, intName, extIP):
        self.intName = intName
        self.extIP = extIP
    def getIntName(self):
        return self.intName
    def getExtIP(self):
        return self.extIP

class ClusterFEN(object):
    def __init__(self, name, address, slaves = list()):
        self.slaves = slaves
        #Host.__init__(self, name, address)
    def getSlaves(self, number = None):
        return self.slaves[0:number]
    def setSlaves(self, slaves):
        self.slaves = slaves
    def setSlavesByPattern(self, intNamePattern, extIPPattern, numberRange):
        self.slaves = list()
        for number in numberRange:
            self.slaves.append(ClusterSlave(intNamePattern % number, extIPPattern % number))

def parseStationList():
    """
    pattern = '^CS010_dipole0|CS010_dipole4|CS010_dipole8|CS010_dipole12| \
	        CS008_dipole0|CS008_dipole4|CS008_dipole8|CS008_dipole12| \
		CS001_dipole0|CS001_dipole4|CS001_dipole8|CS001_dipole12| \
		CS016_dipole0|CS016_dipole4|CS016_dipole8|CS016_dipole12$'
    print 'pattern = ' + str(re.search(pattern, 'CS010_dipole8'))
    """

def getInputNodes(stationList, parset):
    inputNodelist = list()
	
    for s in stationList:
        s = s.strip(" ")
	s = s.strip("[ ]")
	s = s.strip("'")
	name = parset.getString('PIC.Core.' + s + '.port')
	name=name.split(":")
	name=name[0].strip("lii")
	inputNodelist.append(int(name))
    
    return inputNodelist

if __name__ == '__main__':

    # create the parset
    parset = CS1_Parset() 
    stationList = list()
    
    if os.path.exists("../share/Correlator.parset"):
       
        parset.readFromFile('../share/Correlator.parset')

	#read keys from parset file: OLAP.parset
	if os.path.exists("OLAP.parset"):
	    parset.readFromFile('OLAP.parset')
	else:
	    print 'file OLAP.parset does not exist!'
	    sys.exit(0)

        '''
	#set start/stop time
	if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'NULL':
	    # Read from memory!
	    parset['Observation.startTime'] = datetime.datetime.fromtimestamp(1)
	else: 
	    #start=int(time.asctime(time.gmtime()))+ 90
	    start=int(time.time() + 90)
	    #parset['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
	    parset['Observation.startTime'] = datetime.datetime.utcfromtimestamp(start)

	duration = 300
	
	parset['Observation.stopTime'] = datetime.datetime.utcfromtimestamp(start + duration) 
        '''

	if parset.getString('OLAP.OLAP_Conn.input_BGLProc_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.input_BGLProc_Transport']	 = 'NULL'
	    
	if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.station_Input_Transport']	 = 'NULL'

	if parset.getString('OLAP.OLAP_Conn.BGLProc_Storage_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.BGLProc_Storage_Transport']	 = 'NULL'
	
        if not parset.getBool('OLAP.BGLProc.useZoid'): # override CS1.parset
            print 'ZOID!!!!'
	    parset['OLAP.IONProc.useScatter']	 = 'false'
	    parset['OLAP.IONProc.useGather']	 = 'false'
	    parset['OLAP.BGLProc.nodesPerPset']	 = 8
	    parset['OLAP.IONProc.maxConcurrentComm'] = 2
	
	BGLPartition = ('R000_128_0', 'R000_128_0Z')[parset.getBool('OLAP.BGLProc.useZoid')]
	parset['CorrAppl.Correlator.partition'] = BGLPartition
	
	if parset.getBool('OLAP.IONProc.useGather'):
	    print 'useGather!!!!'
	    #parset['OLAP.IONProc.integrationSteps']     = integrationTime
	    parset['OLAP.StorageProc.integrationSteps'] = 1
	else:
	    parset['OLAP.IONProc.integrationSteps']     = 1
	    #parset['OLAP.StorageProc.integrationSteps'] = integrationTime

        if parset.getInt32('Observation.sampleClock') == 160:
            parset['OLAP.BGLProc.integrationSteps'] = 608
        elif parset.getInt32('Observation.sampleClock') == 200:
            parset['OLAP.BGLProc.integrationSteps'] = 768
	
	#get the stations
	stationList = parset.getStringVector('OLAP.storageStationNames')
	parset['OLAP.nrRSPboards'] = len(stationList)
	
	#create input cluster objects
        liifen    = ClusterFEN(name = 'liifen', address = '129.125.99.51')
        liifen.setSlavesByPattern('lii%03d', '10.162.0.%d', [1,2,3,4,5,6,7,8,9,10,11,12])

        #set keys 'Input.InputNodes' and 'Input.OutputNodes'
        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        nSubbandsPerCell = parset.getInt32('OLAP.subbandsPerPset') * parset.getInt32('OLAP.BGLProc.psetsPerCell')
	nCells = float(nSubbands) / float(nSubbandsPerCell)
        if not nSubbands % nSubbandsPerCell == 0:
            raise Exception('Not a integer number of compute cells (nSubbands = %d and nSubbandsPerCell = %d)' % (nSubbands, nSubbandsPerCell))
        nCells = int(nCells)
        host = copy.deepcopy(liifen)
        slaves = host.getSlaves()
	
	inputNodes = getInputNodes(stationList, parset)
	outputNodes = range(1, nCells + 1)
	allNodes = inputNodes + [node for node in outputNodes if not node in inputNodes]

	inputIndices = range(len(inputNodes))
	outputIndices = [allNodes.index(node) for node in outputNodes]

	newslaves = [slaves[ind - 1] for ind in allNodes]
	host.setSlaves(newslaves)
	noProcesses = len(newslaves)
	
	parset['Input.InputNodes'] = inputIndices
	parset['Input.OutputNodes'] = outputIndices
	
	bglprocIPs = [newslaves[j].getExtIP() for j in outputIndices]
	parset['OLAP.OLAP_Conn.input_BGLProc_ServerHosts'] = '[' + ','.join(bglprocIPs) + ']'

	#create output cluster objects
        listfen    = ClusterFEN(name = 'listfen', address = '129.125.99.50')
        listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [1,2])

	#set key 'Connections.BGLProc_Storage.ServerHosts'
        nSubbandsPerCellStorage = parset.getInt32('OLAP.subbandsPerPset')
	nPsetsPerStorage = parset.getInt32('OLAP.psetsPerStorage')
        if not nSubbands % (nSubbandsPerCellStorage * nPsetsPerStorage) == 0:
            raise Exception('Not a integer number of subbands per storage node!')	

        noProcessesStorage = nSubbands / (nSubbandsPerCellStorage * nPsetsPerStorage)
	host = copy.deepcopy(listfen)

        storageIPs = [s.getExtIP() for s in host.getSlaves(noProcessesStorage)]
        parset['OLAP.OLAP_Conn.BGLProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'
	
	parset.writeToFile('./CS1_BGL_Processing.parset')
	
    else:
        print 'file ../share/Correlator.parset does not exist!'
        sys.exit(0) 	
	
