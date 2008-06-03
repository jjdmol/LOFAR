import Hosts
from LOFAR_Jobs import *
import time
import os
import copy
import sys
from CS1_Hosts import *

class Section(object):
    """
    Represents a part of the CS1 application
    """

    def __init__(self, parset, package, host, buildvar = 'gnu_opt', workingDir = os.getcwd()[:-len('LOFAR/Appl/CEP/CS1/CS1_Run/src')]):
        self.workingDir = workingDir
        self.parset = parset
        self.package = package
        self.host = host
        self.buildvar = buildvar
        self.executable = self.package.split('/')[-1]
        
    def getName(self):
        return self.package.split('/')[-1]

    def build(self, cvsupdate, doBuild):
        self.buildJob = BuildJob(self.package.split('/')[-1], \
                                 self.host, \
                                 self.buildvar, \
                                 self.package)            
        self.buildJob.build(cvsupdate, doBuild)

    def isBuildSuccess(self):
        return self.buildJob.isSuccess()
    
    def createBGLJob(self):
	return BGLJob(self.package.split('/')[-1], \
                      self.host, \
                      executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                      noProcesses = self.noProcesses, \
                      partition = self.partition, \
                      workingDir = self.workingDir)
    
    def run(self, runlog, noRuns, runCmd = None):
        if '_bgl' in self.buildvar:
            self.runJob.run(runlog, 
                        self.parsetfile,
                        200*noRuns,
                        noRuns,
                        runCmd)
	    return 0		
        elif 'mpi' in self.buildvar:
            self.runJob = MPIJob(self.package.split('/')[-1], \
                                 self.host, \
                                 executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                                 noProcesses = self.noProcesses, \
                                 workingDir = self.workingDir)
        else:   
            self.runJob = Job(self.package.split('/')[-1], \
                              self.host, \
                              executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                              workingDir = self.workingDir)
        
        # For now set the timeout on 100 times the number of seconds to process
        self.runJob.run(runlog, 
                        self.parsetfile,
                        100*noRuns,
                        noRuns,
                        runCmd)

    def isRunDone(self):
        return self.runJob.isDone()
    
    def abortRun(self):
        self.runJob.abort()
    
    def isRunSuccess(self):
        return self.runJob.isSuccess()

class StorageSection(Section):
    def __init__(self, parset, host):
        for i in range(0, len(parset.getStringVector_new('Observation.VirtualInstrument.partitionList'))):
	    nSubbands = parset.getNrSubbands(i)
            self.nSubbandsPerPset = parset.subbandsPerPset(i)
 	    nPsetsPerStorage = parset.psetsPerStorage(i)
	    if nSubbands > 0:
                if not nSubbands % (self.nSubbandsPerPset * nPsetsPerStorage) == 0:
                    raise Exception('Not a integer number of subbands per storage node!')
		    
        self.noProcesses = len(parset.getStringVector_new('Observation.VirtualInstrument.partitionList')) * parset.getInt32('OLAP.nrStorageNodes')

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Storage', \
                         host = host, \
                         buildvar = 'gnu_openmpi-opt')
        
	if self.parset.getInt32('OLAP.nrStorageNodes') != len(self.host.getSlaves()):
	    raise Exception('key OLAP.nrStorageNodes("%d") != nrStorageNodes("%d") defined in file: CS1_Host.py' % (self.parset.getInt32('OLAP.nrStorageNodes'),  len(self.host.getSlaves())))
	
	storageIPs = [s.getExtIP() for s in self.host.getSlaves()]
	self.parset['OLAP.OLAP_Conn.BGLProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'
	
    def run(self, runlog, noRuns, runCmd = None):
        self.parsetfile = '/tmp/' + self.executable + '.parset'
	self.parset.writeToFile(self.parsetfile)
        runlog = runlog +'.runlog'
	if self.parset.getBool("OLAP.IONProc.useGather"):
	    noRuns = noRuns / self.parset.getInt32("OLAP.IONProc.integrationSteps");
        Section.run(self, runlog, noRuns, runCmd)
        
	os.remove(self.parsetfile)
	
class BGLProcSection(object):
    def __init__(self, parset, host):
        self.BGLProcSectionList = list()
	self.package = 'Appl/CEP/CS1/CS1_BGLProc'
 
        #createBGLProcSection(s)
	for i in range(0, len(parset.getStringVector_new('Observation.VirtualInstrument.partitionList'))):
	    self.BGLProcSectionList.append(BGLProc(parset,host, i, self.package))
	
	inputPsetsArray = '['
	outputPsetsArray = '['
	for bglproc in self.BGLProcSectionList:
	    if len(self.BGLProcSectionList) -1 != self.BGLProcSectionList.index(bglproc):
	       inputPsetsArray = inputPsetsArray + '"' + ','.join(str(b) for b in bglproc.inputPsets) + '",'
	       outputPsetsArray = outputPsetsArray + '"' + ','.join(str(b) for b in bglproc.outputPsets) + '",'
	    else:
	       inputPsetsArray = inputPsetsArray + '"' + ','.join(str(b) for b in bglproc.inputPsets) + '"]'
	       outputPsetsArray = outputPsetsArray + '"' + ','.join(str(b) for b in bglproc.outputPsets) + '"]'
	     
	parset['OLAP.BGLProc.inputPsets']  = inputPsetsArray
	parset['OLAP.BGLProc.outputPsets'] = outputPsetsArray
	print 'inputPsetsArray = ', inputPsetsArray
	print 'outputPsetsArray = ', outputPsetsArray

    def getName(self):
        return self.package.split('/')[-1]
    
    def run(self, runlog, noRuns, runCmd = None):
        parsetfile = '/tmp/' + self.BGLProcSectionList[0].executable + '.parset'
	self.BGLProcSectionList[0].parset.writeToFile(parsetfile)
	
	#copied files
        self.BGLProcSectionList[0].runJob.prerun(parsetfile)
        for bglproc in self.BGLProcSectionList:
	    bglproc.run(runlog, noRuns, parsetfile)
    
        os.remove(parsetfile)
   
    def isRunSuccess(self):
        for bglproc in self.BGLProcSectionList:
	    bglproc.isRunSuccess()
    
class BGLProc(Section):
    def __init__(self, parset, host, index, package):
	self.parset = parset
        self.partition = parset.getStringVector_new('Observation.VirtualInstrument.partitionList')[index]
	self.nSubbands = parset.getNrSubbands(index)
	self.nSubbandsPerPset = parset.subbandsPerPset(index)
	nPsets = 0
	self.inputPsets = list()
	self.outputPsets = list()
	
	if self.nSubbands > 0:
	    if not self.nSubbands % self.nSubbandsPerPset == 0:
		raise Exception('subbands cannot be evenly divided over psets (nSubbands = %d and nSubbandsPerPset = %d)' % (self.nSubbands, self.nSubbandsPerPset))
	    
	    nPsets = self.nSubbands / self.nSubbandsPerPset
            
	    inputNodes = parset.getInputNodes(index)
	    interfaces = IONodes.get(self.partition)
 	    self.inputPsets = [interfaces.index(j) for j in inputNodes]
	   
	    self.outputPsets = range(nPsets)
	    print 'inputNodes = ', inputNodes
	    print 'interfaces = ', interfaces
	    print 'inputPsets = ', self.inputPsets
	    print 'outputPsets = ', self.outputPsets
	else:
	    self.inputPsets.append(0)
	    self.outputPsets.append(0)  
        
	#self.noProcesses = int(nPsets) * parset.getInt32('OLAP.BGLProc.coresPerPset')
	self.noProcesses = 256 # The calculation above is not correct, because some ranks aren't used
	Section.__init__(self, parset, \
                         package, \
                         host = host, \
                         buildvar = 'gnubgl_bgl')
	    
	self.executable = 'CS1_BGL_Processing'
	self.runJob = self.createBGLJob()
	    		     
        nstations = parset.getNStations(index)
        clock = parset.getClockString()	
	    
    def run(self, runlog, noRuns, parsetfile, runCmd = None):
        self.parsetfile = parsetfile 
        coresPerPset = self.parset.getInt32('OLAP.BGLProc.coresPerPset')
        subbandsPerPset = self.nSubbandsPerPset
        actualRuns = int(noRuns * subbandsPerPset / coresPerPset)
        if not actualRuns * coresPerPset == noRuns * subbandsPerPset:
            raise Exception('illegal number of runs')
        
	Section.run(self, runlog, actualRuns, runCmd)
    
class GeneratorSection(Section):
    def __init__(self, parset, host):
        Section.__init__(self, parset, \
                         'Demo/CEP/TFlopCorrelator/TFC_Generator', \
                         host = host, \
                         buildvar = 'gnu_opt')

class Flagger(Section):
    def __init__(self, parset, host):
        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Flagger', \
                         host = host, \
                         buildvar = 'gnu_opt')

