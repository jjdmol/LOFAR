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
	self.partition = parset.getPartition()
        
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
    
    def run(self, runlog, noRuns, runCmd = None):
        if '_bgp' in self.buildvar:
            self.runJob = BGLJob(self.package.split('/')[-1], \
                                 self.host, \
                                 executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                                 noProcesses = self.noProcesses, \
                                 partition = self.partition, \
                                 workingDir = self.workingDir)
        elif 'mpi' in self.buildvar:
            self.runJob = MPIJob(self.package.split('/')[-1], \
                                 self.host, \
                                 executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                                 noProcesses = self.noProcesses, \
                                 workingDir = self.workingDir,
				 partition = self.partition)
        else:   
            self.runJob = Job(self.package.split('/')[-1], \
                              self.host, \
                              executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable, \
                              workingDir = self.workingDir, \
			      partition = self.partition)

        parsetfile = '/tmp/' + self.executable + '.parset'
        self.parset.writeToFile(parsetfile)
        # For now set the timeout on 100 times the number of seconds to process
        if 'IONProc' in self.executable:
	    self.runJob.runIONProc(runlog, 
                                   parsetfile,
                                   100*noRuns,
                                   noRuns,
                                   runCmd)
	else:
	    self.runJob.run(runlog, 
                            parsetfile,
                            100*noRuns,
                            noRuns,
                            runCmd)
        os.remove(parsetfile)

    def isRunDone(self):
        return self.runJob.isDone()
    
    def abortRun(self):
        self.runJob.abort()
    
    def isRunSuccess(self):
        return self.runJob.isSuccess()
	
    def inOutPsets(self, parset):
        nSubbands = parset.getNrSubbands()
        nSubbandsPerPset = parset.getInt32('OLAP.subbandsPerPset')

        if not nSubbands % nSubbandsPerPset == 0:
            raise Exception('subbands cannot be evenly divided over psets (nSubbands = %d and nSubbandsPerPset = %d)' % (nSubbands, nSubbandsPerPset))

        nPsets = nSubbands / nSubbandsPerPset
        self.noProcesses = int(nPsets) * parset.getInt32('OLAP.BGLProc.coresPerPset')
        self.noProcesses = 256 # The calculation above is not correct, because some ranks aren't used

        inputNodes = parset.getInputNodes()
	interfaces = IONodes.get(self.partition)

	inputPsets = [interfaces.index(i) for i in inputNodes]
	outputPsets = range(nPsets)

	if nPsets > len(interfaces):
	  raise Exception('need too many output psets --- increase nrSubbandsPerPset')
	
	if not parset.isDefined('OLAP.BGLProc.inputPsets') and not parset.isDefined('OLAP.BGLProc.outputPsets'):
	    print 'inputNodes = ', inputNodes
	    print 'interfaces = ', interfaces
	    print 'inputPsets = ', inputPsets
	    print 'outputPsets = ', outputPsets

	parset['OLAP.BGLProc.inputPsets']  = inputPsets
	parset['OLAP.BGLProc.outputPsets'] = outputPsets
	parset['OLAP.BGLProc.psetDimensions'] = psetDimensions.get(self.partition)
	
class StorageSection(Section):
    def __init__(self, parset, host):

        nSubbands = parset.getNrSubbands()
        nSubbandsPerPset = parset.getInt32('OLAP.subbandsPerPset')
        nPsetsPerStorage = parset.getInt32('OLAP.psetsPerStorage');
        if not nSubbands % (nSubbandsPerPset * nPsetsPerStorage) == 0:
            raise Exception('Not a integer number of subbands per storage node!')

        self.noProcesses = nSubbands / (nSubbandsPerPset * nPsetsPerStorage)

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Storage', \
                         host = host, \
                         buildvar = 'gnu_openmpi-opt')

        storageIPs = [s.getExtIP() for s in self.host.getSlaves(self.noProcesses * nPsetsPerStorage)]
        self.parset['OLAP.OLAP_Conn.BGLProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'

    def run(self, runlog, noRuns, runCmd = None):
	noRuns = noRuns / self.parset.getInt32("OLAP.IONProc.integrationSteps");
        Section.run(self, runlog, noRuns, runCmd)

class IONProcSection(Section):
    def __init__(self, parset, host, partition):
        self.partition = partition

	Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_IONProc', \
                         host = host, \
                         buildvar = 'gnu_opt')
	
        self.inOutPsets(parset)
	
    def run(self, runlog, noRuns, runCmd = None):
        coresPerPset = self.parset.getInt32('OLAP.BGLProc.coresPerPset')
        subbandsPerPset = self.parset.getInt32('OLAP.subbandsPerPset')
        actualRuns = int(noRuns * subbandsPerPset / coresPerPset)
        if not actualRuns * coresPerPset == noRuns * subbandsPerPset:
            raise Exception('illegal number of runs')
        Section.run(self, runlog, actualRuns, runCmd)        
	
        
class BGLProcSection(Section):
    def __init__(self, parset, host, partition):
        self.partition = partition

        self.inOutPsets(parset)

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_BGLProc', \
                         host = host, \
                         buildvar = 'gnubgp_bgp')

        nstations = parset.getNStations()
        clock = parset.getClockString()
	self.executable = 'CS1_BGL_Processing'
        
    def run(self, runlog, noRuns, runCmd = None):
        coresPerPset = self.parset.getInt32('OLAP.BGLProc.coresPerPset')
        subbandsPerPset = self.parset.getInt32('OLAP.subbandsPerPset')
        actualRuns = int(noRuns * subbandsPerPset / coresPerPset)
        #if not actualRuns * coresPerPset == noRuns * subbandsPerPset:
            #raise Exception('illegal number of runs')
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

