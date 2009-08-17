import Hosts
import time
import os
from Host_Names import *

class Job(object):
    """
    Represents a run of some program.

    """
    def __init__(self, name, host, executable, workingDir, partition):
        self.workingDir = workingDir
        self.name = name
        self.host = host
        self.executable = executable
        self.remoteRunLog = self.workingDir + 'run.' + name + '.' + partition + '.log'
        self.runlog = None
	self.partition = partition
	self.logdest = None

    def run(self, runlog, parsetfile, timeOut, runCmd = None):
        self.runlog = runlog
        self.runLogRetreived = False
	self.host.sput(self.workingDir + 'LOFAR/RTCP/' + self.name + '/src/' + self.name + '.log_prop', '~/')
        if runCmd == None:
            runCmd = self.executable
	
	rCmd = '( cd '+ self.workingDir + '; ' + runCmd + ' ' + parsetfile.split('/')[-1] + ')" > ' + self.remoteRunLog + ' 2>&1'
	if self.logdest != None:
	    rCmd = '( cd '+ self.workingDir + '; ' + runCmd + ' ' + parsetfile.split('/')[-1] + ')" 2>&1 | tee ' + self.remoteRunLog + ' | '+ os.environ.get('HOME', 'default') + '/src/udp-copy.ppc stdin: ' + self.logdest.strip('[').rstrip(']') + ' 1> /dev/null 2>&1'
	
        self.runCommand = self.host.executeAsyncRTCPApps(rCmd, timeout = timeOut)

    def runIONProc(self, runlog, parsetfile, timeOut, runCmd = None):
	self.runlog = runlog
        self.runLogRetreived = False
	self.host.executeAsync('cp ' + self.executable + ' ' + self.workingDir).waitForDone()
	
	interfaces = IONodes.get(self.partition)
        Job.run(self, runlog, parsetfile, timeOut, '/bgsys/LOFAR/openmpi-ion/bin/mpirun -host ' + ','.join(interfaces) + ' ' + os.path.join(self.workingDir, self.executable.split('/')[-1]) + ' ' + parsetfile.split('/')[-1])

    def isDone(self):
        ret = self.runCommand.isDone()
        if not ret:
            self.checkTimeOut()
        return ret
    
    def waitForDone(self):
        while not self.isDone():
            time.sleep(1)
        
    def isSuccess(self):
        self.waitForDone()
        if not self.runLogRetreived:
	    listfen.sput(self.remoteRunLog, self.runlog)
            self.runLogRetreived = True
        return self.runCommand.isSuccess()

    def checkTimeOut(self):
        if self.runCommand.isTimedOut():
            print(self.name + ' timed out, aborting ...')
            self.abort()
    
    def abort(self):
        print('Aborting ' + self.name)
	if (self.name == 'IONProc'):
	    bgfen0.executeAsync('killall mpirun').waitForDone() # also kills CNProc
	elif (self.name == 'CNProc'):
	    bgfen0.executeAsync('killall mpirun').waitForDone() # also kills IONProc
	else:  
            self.host.executeAsync('killall ' + self.name).waitForDone()

    def log2SasMac(self, connectionInfo):
	self.logdest = connectionInfo

class MPIJob(Job):
    '''
    This is a variation on a job that runs with MPI
    '''
    def __init__(self, name, host, executable, noProcesses, workingDir, partition):
        self.noProcesses = noProcesses
        Job.__init__(self, name, host, executable, workingDir, partition)

    def run(self, runlog, parsetfile, timeOut, runCmd):
        self.createMachinefile()
        if runCmd == None:
            runCmd = self.executable
        Job.run(self, runlog, parsetfile, timeOut, 'mpirun -nolocal -np ' + str(self.noProcesses) + \
                ' -machinefile ' + self.workingDir + self.name + '.machinefile ' + runCmd)

    def abort(self):
        print('Aborting ' + self.name)
        self.host.executeAsync('killall ' + self.name).waitForDone()
        # TODO ugly!! This could kill other processes too
        # We should find a way to cancel a command in a nice way
        self.host.executeAsync('cexec killall ' + self.name + ' -9').waitForDone()
        self.host.executeAsync('killall mpirun').waitForDone()

    def createMachinefile(self):
        lmf = self.workingDir + 'tmp.machinefile'
        outf = open(lmf, 'w')
        for i in range(0,self.noProcesses):
  	    outf.write(self.host.getSlaves()[i].getIntName() + '\n')
        outf.close()
        self.host.sput(lmf, self.workingDir + self.name + '.machinefile')
        os.remove(lmf)

class CNJob(Job):
    ''' A Job that runs on BlueGene/L'''
    def __init__(self, name, host, executable, noProcesses, partition, workingDir):
        self.partition = partition
        self.workingDir = workingDir
        self.noProcesses = noProcesses

        # this can overwrite the values that were set before this line
        Job.__init__(self, name, host, executable, workingDir, partition)
        self.jobID = '0'

    def run(self, runlog, parsetfile, timeOut, runCmd):
        self.host.executeAsync('cp ' + self.executable + ' ' + self.workingDir).waitForDone()
        Job.run(self, runlog, parsetfile, timeOut, 'mpirun -partition ' + self.partition + ' -mode VN -env DCMF_COLLECTIVES=0 -env BG_MAPPING=XYZT -env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/cephome/romein/lib.bgp -cwd ' + self.workingDir + ' -exe ' + os.path.join(self.workingDir, self.executable.split('/')[-1] + ' -args'))
