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

    def __init__(self, parset, package, host, buildvar = 'gnu_opt', workingDir = '~/'):
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
                                 workingDir = self.bglWorkingDir)
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

        self.nrsp = parset.getInt32('Input.NRSPBoards')
        nSubbands = parset.getInt32('Observation.NSubbands')
        nSubbandsPerCell = parset.getInt32('General.SubbandsPerPset') * parset.getInt32('BGLProc.PsetsPerCell')
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
	self.parset['Connections.Input_BGLProc.ServerHosts'] = '[' + ','.join(bglprocIPs) + ']'

    def run(self, runlog, noRuns, runCmd = None):
        Section.run(self, runlog, noRuns, runCmd)

class StorageSection(Section):
    def __init__(self, parset, host):

        nSubbands = parset.getInt32('Observation.NSubbands')
        nSubbandsPerCell = parset.getInt32('General.SubbandsPerPset') * parset.getInt32('BGLProc.PsetsPerCell')
        if not nSubbands % nSubbandsPerCell == 0:
            raise Exception('Not a integer number of subbands per compute cells!')

        self.noProcesses = nSubbands / nSubbandsPerCell

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_Storage', \
                         host = host, \
                         buildvar = 'gnu32_openmpi-opt')

        storageIPs = [s.getExtIP() for s in self.host.getSlaves(self.noProcesses)]
        self.parset['Connections.BGLProc_Storage.ServerHosts'] = '[' + ','.join(storageIPs) + ']'

        
class BGLProcSection(Section):
    def __init__(self, parset, host, partition, workingDir):
        self.partition = partition
        self.bglWorkingDir = workingDir

        nSubbands = parset.getInt32('Observation.NSubbands')
        nSubbandsPerCell = parset.getInt32('General.SubbandsPerPset') * parset.getInt32('BGLProc.PsetsPerCell')
        if not nSubbands % nSubbandsPerCell == 0:
            raise Exception('Not a integer number of compute cells!')
        nCells = nSubbands / nSubbandsPerCell
        self.noProcesses = int(nCells) * parset.getInt32('BGLProc.NodesPerPset') * parset.getInt32('BGLProc.PsetsPerCell')
        self.noProcesses = 256 # The calculation above is not correct, because some ranks aren't used

        Section.__init__(self, parset, \
                         'Appl/CEP/CS1/CS1_BGLProc', \
                         host = host, \
                         buildvar = 'gnu_bgl')


        # The executable for BGL_Processing should be recompiled for every combination of the number
        # of stations and the clock setting. So for every combination there should be an executable present.
        # todo: We should check here if the executable exists
        nstations = parset.getNStations()
        clock = parset.getClockString()
        #self.executable = 'CS1_BGL_Processing.' + str(nstations) + 'st.' + clock
	self.executable = 'CS1_BGL_Processing'
        
    def run(self, runlog, noRuns, runCmd = None):
        nodesPerCell = self.parset.getInt32('BGLProc.NodesPerPset') * self.parset.getInt32('BGLProc.PsetsPerCell')
        subbandsPerCell = self.parset.getInt32('General.SubbandsPerPset') * self.parset.getInt32('BGLProc.PsetsPerCell')
        actualRuns = int(noRuns * subbandsPerCell / nodesPerCell)
        if not actualRuns * nodesPerCell == noRuns * subbandsPerCell:
            raise Exception('illegal number of runs')
        Section.run(self, runlog, int(noRuns * subbandsPerCell / nodesPerCell), runCmd)        

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

