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

    def getNCells(self):
        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        psetspercell = parset.getInt32('OLAP.BGLProc.psetsPerCell')
        subbandsperpset = parset.getInt32('OLAP.subbandsPerPset')
        return nSubbands / (psetspercell * subbandsperpset)

    def writeToFile(self, fileName):
        outf = open(fileName, 'w')
        for key, value in sorted(self.parameters.iteritems()):
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


	
if __name__ == '__main__':

    # create the parset
    parset = CS1_Parset() 
    stationList = list()
    
    if os.path.exists("../share/StorageProg.parset"):
       
        #read keys from parset file: Transpose.parset
        parset.readFromFile('../share/StorageProg.parset')
        
	#get the stations
	stationList = parset.getStringVector('OLAP.storageStationNames')
        '''
	#set start/stop time
	if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'NULL':
	    # Read from memory!
	    parset['Observation.startTime'] = datetime.datetime.fromtimestamp(1)
	else:    
	    start=int(time.time() + 90)
	    parset['Observation.startTime'] = datetime.datetime.fromtimestamp(start)

	duration = 300
	
	parset['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration) 
        '''
	if parset.getString('OLAP.OLAP_Conn.input_DelayComp_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.input_DelayComp_Transport']	 = 'NULL'
	    
	if parset.getString('OLAP.OLAP_Conn.input_BGLProc_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.input_BGLProc_Transport']	 = 'NULL'
	    
	if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.station_Input_Transport']	 = 'NULL'

	if parset.getString('OLAP.OLAP_Conn.BGLProc_Storage_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.BGLProc_Storage_Transport']	 = 'NULL'
	
	
        if not parset.getBool('OLAP.BGLProc.useZoid'): # override CS1.parset
	    parset['OLAP.IONProc.useScatter']	 = 'false'
	    parset['OLAP.IONProc.useGather']	 = 'false'
	    parset['OLAP.BGLProc.nodesPerPset']	 = 8
	    parset['OLAP.IONProc.maxConcurrentComm'] = 2
	
	if parset.getBool('OLAP.IONProc.useGather'):
	    #parset['OLAP.IONProc.integrationSteps']     = integrationTime
	    parset['OLAP.StorageProc.integrationSteps'] = 1
	else:
	    parset['OLAP.IONProc.integrationSteps']     = 1
	    #parset['OLAP.StorageProc.integrationSteps'] = integrationTime
        
	#create output cluster objects
        listfen    = ClusterFEN(name = 'listfen', address = '129.125.99.50')
        listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [1,2])

        #set key 'Connections.BGLProc_Storage.ServerHosts'
        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        nSubbandsPerCell = parset.getInt32('OLAP.subbandsPerPset')
        nPsetsPerStorage = parset.getInt32('OLAP.psetsPerStorage')
        if not nSubbands % (nSubbandsPerCell * nPsetsPerStorage) == 0:
            raise Exception('Not a integer number of subbands per storage node!')
        
        noProcesses = nSubbands / (nSubbandsPerCell * nPsetsPerStorage)
	host = copy.deepcopy(listfen)
	
        storageIPs = [s.getExtIP() for s in host.getSlaves(noProcesses)]
        parset['OLAP.OLAP_Conn.BGLProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'
       
        # if the msname wasn't given, read the next number from the file
        MSdatabaseFile = '/log/MSList'
        
        try:
	    dbfile = open(MSdatabaseFile, 'a')
	    nodesStr = str([1] * parset.getNCells() + [0] * (12 - parset.getNCells()))[1:-1]
	    dateStr = time.strftime('%Y %0m %0d %H %M %S', time.gmtime()).split()

	    MS = parset.getString('Observation.MSNameMask')
	    MS = MS.replace('${YEAR}', dateStr[0])
	    MS = MS.replace('${MONTH}', dateStr[1])
	    MS = MS.replace('${DAY}', dateStr[2])
	    MS = MS.replace('${HOURS}', dateStr[3])
	    MS = MS.replace('${MINUTES}', dateStr[4])
	    MS = MS.replace('${SECONDS}', dateStr[5])
	    MS = MS.replace('${MSNUMBER}', '%05d' % parset.getInt32('Observation.ObsID'))
	    MS = MS.replace('${SUBBAND}', '*')
	
	    dbfile.write(MS + '\t' + ' '.join(dateStr[0:3]) + '\t' + nodesStr + '\n')
	    dbfile.close()
        except:
	    sys.exit(1)
		

	commandstr ='cexec mkdir ' + '/data/' + 'L' + dateStr[0] + '_' + '%05d' % parset.getInt32('Observation.ObsID')
	if os.system(commandstr) != 0:
	    print 'Failed to create directory: /data/' + parset.getInt32('Observation.ObsID')
        
	parset.writeToFile('../share/StorageProg.parset')
	
	#createMachinefile
	lmf = '/tmp/CS1_tmpfile'
	slaves = host.getSlaves(noProcesses)
	outf = open(lmf, 'w')
        for slave in slaves:
            outf.write(slave.getIntName() + '\n')
        outf.close()
	os.system('mv /tmp/CS1_tmpfile /opt/lofar/bin/StorageProg.machinefile 2>&1 >> /dev/null')

	sys.exit(noProcesses)
    else:
        print '../share/StorageProg.parset does not exist!'
        sys.exit(0) 	
	
