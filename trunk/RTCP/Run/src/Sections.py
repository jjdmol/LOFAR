import Hosts
from LOFAR_Jobs import *
import time
import os
import copy
import sys
from Host_Names import *

class Section(object):
    """
    Represents a part of the RTCP application
    """

    def __init__(self, parset, package, host, workingDir, parsetfile, buildvar = 'gnu_opt'):
        self.workingDir = workingDir
        self.parset = parset
        self.package = package
        self.host = host
        self.buildvar = buildvar
        self.executable = self.package.split('/')[-1]
	self.partition = parset.getPartition()
	self.parsetfile = parsetfile
        
    def getName(self):
        return self.package.split('/')[-1]

    def run(self, runlog, timeOut, runCmd = None):
        if 'bgp_cn' in self.buildvar:
            self.runJob = CNJob(self.package.split('/')[-1], \
                                 self.host, \
                                 executable = self.workingDir + 'LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                                 noProcesses = self.noProcesses, \
                                 partition = self.partition, \
                                 workingDir = self.workingDir)
        elif 'mpi' in self.buildvar:
            self.runJob = MPIJob(self.package.split('/')[-1], \
                                 self.host, \
                                 executable = self.workingDir + 'LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                                 noProcesses = self.noProcesses, \
                                 workingDir = self.workingDir,
				 partition = self.partition)
        else:   
            self.runJob = Job(self.package.split('/')[-1], \
                              self.host, \
                              executable = self.workingDir + 'LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                              workingDir = self.workingDir, \
			      partition = self.partition)

        if self.parset.getBool('OLAP.log2SasMac'):  
	    self.runJob.log2SasMac(self.parset.getString('OLAP.OLAP_Conn.log2SasMacOutputs'))
 
        # For now set the timeout on 100 times the number of seconds to process
        if 'IONProc' in self.executable:
	    self.runJob.runIONProc(runlog, 
                                   self.parsetfile,
                                   100*timeOut,
                                   runCmd)
	else:
	    self.runJob.run(runlog, 
                            self.parsetfile,
                            100*timeOut,
                            runCmd)

    def isRunDone(self):
        return self.runJob.isDone()
    
    def abortRun(self):
        self.runJob.abort()
    
    def isRunSuccess(self):
        return self.runJob.isSuccess()
	
    def inOutPsets(self, parset):
        nSubbands = parset.getNrSubbands()
        nSubbandsPerPset = parset.getSubbandsPerPset()
	nPsets = parset.getNrPsets()

        self.noProcesses = int(nPsets) * parset.getInt32('OLAP.CNProc.coresPerPset')
        self.noProcesses = 256 # The calculation above is not correct, because some ranks aren't used

	stations = parset.getStations()

	interfaces = IONodes.get(self.partition)

	#inputPsets = [interfaces.index(i) for i in inputNodes]
	inputPsets = [station.getPset(self.partition) for station in stations]
	outputPsets = range(nPsets)

	if nPsets > len(interfaces):
	  raise Exception('need too many output psets --- increase nrSubbandsPerPset')
	
	if not parset.isDefined('OLAP.CNProc.inputPsets') and not parset.isDefined('OLAP.CNProc.outputPsets'):
	    print 'interfaces = ', interfaces
	    print 'inputPsets = ', inputPsets
	    print 'outputPsets = ', outputPsets

	parset['OLAP.CNProc.inputPsets']  = inputPsets
	parset['OLAP.CNProc.outputPsets'] = outputPsets
	parset.checkRspBoardList()
	
    def getNoProcesses(self):
	return self.noProcesses


class StorageSection(Section):
    def __init__(self, parset, host, workingDir, parsetfile):

        nSubbands = parset.getNrSubbands()
        nSubbandsPerPset = parset.getSubbandsPerPset()
        nPsetsPerStorage = parset.getPsetsPerStorage()
	nPsets = parset.getNrPsets()
	self.noProcesses = parset.getNrUsedStorageNodes()

	print 'subbands = ' + str(nSubbands) + ' subbandsPerPset = ' + str(nSubbandsPerPset) + ' psetsPerStorage = ' + str(nPsetsPerStorage) + ' nrProcesses = ' + str(self.noProcesses)

        Section.__init__(self, parset, \
                         'RTCP/Storage', \
                         host = host, \
 			 workingDir = workingDir, \
			 parsetfile = parsetfile, \
			 buildvar = 'gnu_openmpi-opt')

        storageIPs = [s.getExtIP() for s in self.host.getSlaves(self.noProcesses)]
        self.parset['OLAP.OLAP_Conn.IONProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'

    def run(self, runlog, timeOut, runCmd = None):
        Section.run(self, runlog, timeOut, runCmd)


class IONProcSection(Section):
    def __init__(self, parset, host, partition, workingDir, parsetfile):
        self.partition = partition

	Section.__init__(self, parset, \
                         'RTCP/IONProc', \
                         host = host, \
			 workingDir = workingDir, \
			 parsetfile = parsetfile, \
			 buildvar = 'gnubgp_ion')
	
        self.inOutPsets(parset)
	
    def run(self, runlog, timeOut, runCmd = None):
        Section.run(self, runlog, timeOut, runCmd)        
        

class CNProcSection(Section):
    def __init__(self, parset, host, partition, workingDir, parsetfile):
        self.partition = partition

        self.inOutPsets(parset)

        Section.__init__(self, parset, \
                         'RTCP/CNProc', \
                         host = host, \
			 workingDir = workingDir, \
			 parsetfile = parsetfile, \
			 buildvar = 'gnubgp_cn')

        nstations = parset.getNStations()
        clock = parset.getClockString()
	self.executable = 'CN_Processing'
        
    def run(self, runlog, timeOut, runCmd = None):
        Section.run(self, runlog, timeOut, runCmd)
