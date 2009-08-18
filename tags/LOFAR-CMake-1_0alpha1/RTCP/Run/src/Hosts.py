from Commands import *

class Host(object):
    """
    Represents a computer
    """
    def __init__(self, name, address):
        self.name = name
        self.address = address
        self.sshCommand = 'ssh ' + self.address
        self.CVSHost = 'cvs.astron.nl'
        
    def getCVSHost(self):
        return self.CVSHost

    def getSSHCommand(self):
        return self.sshCommand

    def executeAsyncRTCPApps(self, commandstr, logfile = '/dev/null', timeout = None):
        return AsyncThreadCommand(self.sshCommand + ' "' + commandstr, timeout)
	
    def executeAsync(self, commandstr, logfile = '/dev/null', timeout = None):
        return AsyncThreadCommand(self.sshCommand + ' "' + commandstr + '" >> ' + logfile, timeout)

    def sget(self, remoteFile, localFile, logfile = '/dev/null'):
        return ExtCommand('scp ' + self.address + ':' + remoteFile + ' ' + localFile + ' 2>&1 >> ' + logfile)

    def sput(self, localFile, remoteFile, logfile = '/dev/null'):
        return ExtCommand('scp ' + localFile + ' ' + self.address + ':' + remoteFile + ' 2>&1 >> ' + logfile)

    def executeSync(self, commandstr, logfile = '/dev/null', timeout = None):
        return self.executeAsync(commandstr, logfile, timeout).isSuccess()
        
class ClusterSlave(object):
    def __init__(self, intName, extIP):
        self.intName = intName
        self.extIP = extIP
    def getIntName(self):
        return self.intName
    def getExtIP(self):
        return self.extIP

class ClusterFEN(Host):
    def __init__(self, name, address, slaves = list()):
        self.slaves = slaves
        Host.__init__(self, name, address)
    def getSlaves(self, number = None):
        return self.slaves[0:number]
    def setSlaves(self, slaves):
        self.slaves = slaves
    def setSlavesByPattern(self, intNamePattern, extIPPattern, numberRange):
        # e.g.
        # setSlavesByPattern('lii%03d', '10.20.150.%d', range(1, 13))
        # setSlavesByPattern('list%03d', '10.20.170.%d', range(1, 13))
        self.slaves = list()
        for number in numberRange:
            self.slaves.append(ClusterSlave(intNamePattern % number, extIPPattern % number))

class UserId(object):
    """
    Represents a userId
    """
    def __init__(self, host, name):
        self.name = name
	self.host = host
    
    def getName(self):
        return self.name
        
    def getHost(self):
        return self.host
