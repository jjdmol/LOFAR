import Hosts

class RSPCtl(object):
    """
    RSPCtl can be used to control the lcu for an rsp board. You shouldn't use this class, use MAC/SAS in stead.
    """

    def __init__(self, lcu):
        self.host = lcu
        self.rspctl = 'rspctl ' # '~/wierenga/LOFAR/installed/gnunew_debug/bin/rspctl '
        self.rcuCommand = None
        self.beamletCommand = None
        
    def startRCUStats(self, rcus, time = 10):        
        self.stopRCUStats()
        cmdstr = self.rspctl + ' --statistics --select=' + ','.join(str(rcu) for rcu in rcus)
        self.rcuCommand = self.host.executeAsync(cmdstr, timeout = time)
        
    def stopRCUStats(self):
        if self.rcuCommand:
            self.rcuCommand.abort();
        self.rcuCommand = None        
        
    def startBeamletStats(self, time = 10):        
        self.stopBeamletStats()
        cmdstr = self.rspctl + ' --statistics=beamlet'
        self.beamletCommand = self.host.executeAsync(cmdstr, timeout = time)
        
    def stopBeamletStats(self):
        if self.beamletCommand:
            self.beamletCommand.abort();
        self.beamletCommand = None        
        
    def doRSPcmd(self, cmd):
        cmd = self.rspctl + cmd
        if not self.host.executeSync(cmd):
            raise Exception(cmd + ' not succesfully executed')       

    def initBoards(self, clockstr):

        if not clockstr == '160MHz' and not clockstr == '200MHz':
            raise Exception('Illegal clock setting: ' + str(clockstr) + '. Should be 160MHz or 200MHz')
        self.clock = int(clockstr[0:3]) * 1e6

        rspctl = self.rspctl
        self.doRSPcmd(' --rspclear')
        self.doRSPcmd(' --clock=' + clockstr)
        time.sleep(15)
        self.doRSPcmd('--rcumode=3')
        self.doRSPcmd('--rcuenable')

    def selectSubbands(self, startsb):                
        if not self.clock:
            raise Exception('Cannot select subbands if clock has not been set yet.')

        subbandnr = int(startsb * 1024 / self.clock)
        if not (startsb * 1024) % self.clock == 0:
            raise Exception('Unable to set specified subband, did you mean ' + str(subbandnr * clock / 1024) + '?')
        endsb = min(subbandnr + 53, 511)
        subbandstr = str(subbandnr) + ':' + str(endsb)
        subbandstr = ','.join(4 * [subbandstr])

        self.doRSPcmd(' --subbands=' + subbandstr)
        
    def selectRCUs(self, rcus):
        self.doRSPcmd('--weights=0')
        self.doRSPcmd('--weights=1 --select=' + ','.join(str(rcu) for rcu in rcus))

    def setWG(self, rcus, freq):
        self.doRSPcmd('--wg=' + str(freq) + ' --select=' + ','.join(str(rcu) for rcu in rcus))
    
    
if __name__ == '__main__':
    r = RSPCtl()
    r.startRCUStats([0,1])
    r.startBeamletStats()
    r.setConfiguration('160MHz', 60e6, [0, 1, 8, 9])
    import time
    time.sleep(10)
    r.stopRCUStats()
    r.stopBeamletStats()
