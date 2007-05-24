import Hosts
import time
import os


class Job(object):
    """
    Represents a run of some program.

    """
    def __init__(self, name, host, executable, workingDir):
        self.workingDir = workingDir
        self.name = name
        self.host = host
        self.executable = executable
        self.remoteRunLog = self.workingDir + 'run.' + name + '.log'
        self.runlog = None

    def run(self, runlog, parsetfile, timeOut, noRuns, runCmd = None):
        self.runlog = runlog
        self.runLogRetreived = False
        self.host.sput(parsetfile, '~/')
        if runCmd == None:
            runCmd = self.executable
        self.runCommand = self.host.executeAsync('( cd ~ ; ' + runCmd + ' ' + str(noRuns) + ') &> ' + self.remoteRunLog, timeout = timeOut)
    
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
	    
            self.host.sget(self.remoteRunLog, self.runlog)
            self.runLogRetreived = True
        return self.runCommand.isSuccess()

    def checkTimeOut(self):
        if self.runCommand.isTimedOut():
            print(self.name + ' timed out, aborting ...')
            self.abort()
    
    def abort(self):
        print('Aborting ' + self.name)
        self.host.executeAsync('killall ' + self.name).waitForDone()

class MPIJob(Job):
    '''
    This is a variation on a job that runs with MPI
    '''
    def __init__(self, name, host, executable, noProcesses, workingDir):
        self.noProcesses = noProcesses
        Job.__init__(self, name, host, executable, workingDir)
    def run(self, runlog, parsetfile, timeOut, noRuns, runCmd):
        self.createMachinefile()
        if runCmd == None:
            runCmd = self.executable
        if self.name == 'CS1_InputSection':
            Job.run(self, runlog, parsetfile, timeOut, noRuns, 'mpirun_rsh -np ' + str(self.noProcesses) + \
                    ' -hostfile ~/' + self.name + '.machinefile ' + runCmd)
        else:
            Job.run(self, runlog, parsetfile, timeOut, noRuns, 'mpirun -nolocal -np ' + str(self.noProcesses) + \
                    ' -machinefile ~/' + self.name + '.machinefile ' + runCmd)
    def abort(self):
        print('Aborting ' + self.name)
        self.host.executeAsync('killall ' + self.name).waitForDone()
        # TODO ugly!! This could kill other processes too
        # We should find a way to cancel a command in a nice way
        self.host.executeAsync('cexec killall ' + self.name + ' -9').waitForDone()
        self.host.executeAsync('killall mpirun').waitForDone()

    def createMachinefile(self):
        lmf = '/tmp/CS1_tmpfile'
        slaves = self.host.getSlaves(self.noProcesses)
        outf = open(lmf, 'w')
        for slave in slaves:
            outf.write(slave.getIntName() + '\n')
        outf.close()
        self.host.sput(lmf, '~/' + self.name + '.machinefile')
        os.remove(lmf)

class BGLJob(Job):
    ''' A Job that runs on BlueGene/L'''
    def __init__(self, name, host, executable, noProcesses, partition, workingDir):
        self.partition = partition
        self.workingDir = workingDir
        self.noProcesses = noProcesses

        # this can overwrite the values that were set before this line
        Job.__init__(self, name, host, executable, workingDir)
        self.tmplog = 'CS1_Run.tmplog'
        self.jobID = '0'

    def run(self, runlog, parsetfile, timeOut, noRuns, runCmd):
        print 'NOT executing: Immediately executing ' + self.host.getSSHCommand() + ' "cp ' + self.executable + ' ' + self.workingDir + '"'
        #self.host.executeAsync('cp ' + self.executable + ' ' + self.workingDir).waitForDone()
	self.host.sput(parsetfile, self.workingDir)
	self.host.sput('OLAP.parset', self.workingDir)
        Job.run(self, runlog, parsetfile, timeOut, noRuns, 'mpirun -partition ' + self.partition + ' -np ' + str(self.noProcesses) + ' -mode VN -label -cwd ' + self.workingDir + ' ' + os.path.join(self.workingDir, self.executable.split('/')[-1]))


class BuildJob(Job):
    ''' A job that builds a LOFAR package '''
    def __init__(self, package, host, buildvar, name, workingDir):
        Job.__init__(self, name, host, None, workingDir)
        self.buildvar = buildvar
        self.package = package
        self.remoteRunLog = workingDir + '/LOFAR/build.log'
        
    def run(self, cvsupdate, doBuild):
        self.buildLogRetreived = False
        cvsCommand = 'cvs -d :pserver:' + self.host.getCVSHost() + ':/cvs/cvsroot'
        if not cvsupdate:
            rubOptions = '-noupdate'
        else:
            rubOptions = ''
        rubOptions = rubOptions + ' -cvs \'' + cvsCommand + '\' -distclean -release=main -build ' + self.buildvar + ' ' + self.package
        if doBuild:
            self.runCommand = self.host.executeAsync('cd ' + workingDir + '/LOFAR ; ./autoconf_share/rub ' + rubOptions)
        else:
            self.runCommand = self.host.executeAsync('echo build not done')

    
