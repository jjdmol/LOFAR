import Hosts
from LOFAR_Jobs import *
import time
import os
import copy
import sys

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
    
    def run(self, runlog, noRuns, runCmd = None):
        if '_bgl' in self.buildvar:
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
                                 noProcesses = self.noProcesses,
                                 workingDir = self.workingDir)
        else:   
            self.runJob = Job(self.package.split('/')[-1], \
                              self.host, \
                              executable = self.workingDir + '/LOFAR/installed/' + self.buildvar + '/bin/' + self.executable,
                              workingDir = self.workingDir)
        
        parsetfile = '/tmp/' + self.executable + '.parset'
        self.parset.writeToFile(parsetfile)
        # For now set the timeout on 100 times the number of seconds to process
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

class InputSection(Section):
    def __init__(self, parset, myhost):

        self.nrsp = parset.getInt32('OLAP.nrRSPboards')
	
        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        nSubbandsPerCell = parset.getInt32('OLAP.subbandsPerPset') * parset.getInt32('OLAP.BGLProc.psetsPerCell')
        nCells = float(nSubbands) / float(nSubbandsPerCell)
        if not nSubbands % nSubbandsPerCell == 0:
            raise Exception('Not a integer number of compute cells (nSubbands = %d and nSubbandsPerCell = %d)' % (nSubbands, nSubbandsPerCell))
        self.nCells = int(nCells)

        host = copy.deepcopy(myhost)
        slaves = host.getSlaves()

        inputNodes = parset.getInputNodes()
        outputNodes = range(1, self.nCells + 1)
        allNodes = inputNodes + [node for node in outputNodes if not node in inputNodes]
    
        inputIndices = range(len(inputNodes))
        outputIndices = [allNodes.index(node) for node in outputNodes]

        print("slaves %d"  % len(slaves))
        print("allNodes %d" % len(allNodes))
        
        newslaves = [slaves[ind - 1] for ind in allNodes]
        host.setSlaves(newslaves)
        self.noProcesses = len(newslaves)

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_InputSection', \
                         host = host, \
                         buildvar = 'gnu64_mpich-opt')

        self.parset['Input.InputNodes'] = inputIndices
        self.parset['Input.OutputNodes'] = outputIndices

        bglprocIPs = [newslaves[j].getExtIP() for j in outputIndices]
        self.parset['OLAP.OLAP_Conn.input_BGLProc_ServerHosts'] = '[' + ','.join(bglprocIPs) + ']'

    def run(self, runlog, noRuns, runCmd = None):
        Section.run(self, runlog, noRuns, runCmd)

class StorageSection(Section):
    def __init__(self, parset, host):

        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        nSubbandsPerCell = parset.getInt32('OLAP.subbandsPerPset')
        nPsetsPerStorage = parset.getInt32('OLAP.psetsPerStorage');
        if not nSubbands % (nSubbandsPerCell * nPsetsPerStorage) == 0:
            raise Exception('Not a integer number of subbands per storage node!')

        self.noProcesses = nSubbands / (nSubbandsPerCell * nPsetsPerStorage)

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Storage', \
                         host = host, \
                         buildvar = 'gnu32_openmpi-opt')

        storageIPs = [s.getExtIP() for s in self.host.getSlaves(self.noProcesses * nPsetsPerStorage)]
        self.parset['OLAP.OLAP_Conn.BGLProc_Storage_ServerHosts'] = '[' + ','.join(storageIPs) + ']'

    def run(self, runlog, noRuns, runCmd = None):
	if self.parset.getBool("OLAP.IONProc.useGather"):
	    noRuns = noRuns / self.parset.getInt32("OLAP.IONProc.integrationSteps");
        Section.run(self, runlog, noRuns, runCmd)

        
class BGLProcSection(Section):
    def __init__(self, parset, host, partition):
        self.partition = partition

        nSubbands = len(parset.getInt32Vector('Observation.subbandList'))
        nSubbandsPerCell = parset.getInt32('OLAP.subbandsPerPset') * parset.getInt32('OLAP.BGLProc.psetsPerCell')

        if not nSubbands % nSubbandsPerCell == 0:
            raise Exception('Not a integer number of compute cells!')

        nCells = nSubbands / nSubbandsPerCell
        self.noProcesses = int(nCells) * parset.getInt32('OLAP.BGLProc.nodesPerPset') * parset.getInt32('OLAP.BGLProc.psetsPerCell')
        self.noProcesses = 256 # The calculation above is not correct, because some ranks aren't used

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_BGLProc', \
                         host = host, \
                         buildvar = 'gnubgl_bgl')

        nstations = parset.getNStations()
        clock = parset.getClockString()
	self.executable = 'CS1_BGL_Processing'
        
    def run(self, runlog, noRuns, runCmd = None):
        nodesPerCell = self.parset.getInt32('OLAP.BGLProc.nodesPerPset') * self.parset.getInt32('OLAP.BGLProc.psetsPerCell')
        subbandsPerCell = self.parset.getInt32('OLAP.subbandsPerPset') * self.parset.getInt32('OLAP.BGLProc.psetsPerCell')
        actualRuns = int(noRuns * subbandsPerCell / nodesPerCell)
        if not actualRuns * nodesPerCell == noRuns * subbandsPerCell:
            raise Exception('illegal number of runs')
        Section.run(self, runlog, actualRuns, runCmd)        

class GeneratorSection(Section):
    def __init__(self, parset, host):
        Section.__init__(self, parset, \
                         'Demo/CEP/TFlopCorrelator/TFC_Generator', \
                         host = host, \
                         buildvar = 'gnu_opt')

class DelayCompensationSection(Section):
    def __init__(self, parset, host):
        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_DelayCompensation', \
                         host = host, \
                         buildvar = 'gnu_debug')

class Flagger(Section):
    def __init__(self, parset, host):
        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Flagger', \
                         host = host, \
                         buildvar = 'gnu_opt')

