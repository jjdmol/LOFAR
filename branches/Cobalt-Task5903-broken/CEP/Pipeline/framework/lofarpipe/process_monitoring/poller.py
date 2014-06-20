from threading import Thread
import socket
import os
import time
import subprocess as SP
import output

class polltimer(object):
    """Timer class for the poller. This class essentially offers a
    sleep of tzero, that doubles after tdouble times. So if you want 
    to start with one second and double every ten times, you end up with 
    calling the polltimer with parameters polltimer(1,10).
    NB: this means that the first ten times, we wait 1 second,
    the second ten times we wait two, etc. So the tenth time
    is inclusive. """
    
    def __init__(self, tzero, tdouble):
        """Init. tzero is the first sleep, tdouble the 
        number of times after which the ait doubles"""
        self.slptim = float(tzero)
        self.tdouble = float(tdouble)
        self.numwaits = 0
    
    def sleep(self):
        """Sleep and increment the counters (and change the wait time if needed"""
        if self.numwaits < self.tdouble:
            self.numwaits = 0
            self.slptim *= 2
        time.sleep(self.slptim)   
        
class Poller(Thread):
    """ This class will poll a command (defined in the config as 'poll_execute'.
    The path that will be searched is the list given in the config as 'searchpath'
    followed by the local folder and the folder in which the config class is
    located. The file present in that folder is essentially the default script.
    The interval time can be provided via the config parameters tzero (first interval)
    and tdouble (number of times after which this interval doubles).
    """
    def __init__(self, config):
        self.config = config
        self.pollexecutable = self.config['poll_execute']
        try:
            if isinstance(self.config['searchpath'], str):
                self.searchpath = [self.config['searchpath']]
            else:
                self.searchpath = self.config['searchpath']
        except KeyError:
            self.searchpath = list()
        self.searchpath.extend([".",self.config.path])
        self.poll_execpath = None
        for pn in self.searchpath:
            tep = pn+"/"+self.pollexecutable
            if os.access(tep, os.X_OK):
                self.poll_execpath = tep
                break
        if not self.poll_execpath:
            raise Exception # ToDo: define exception class
        self.tzero = self.config['tzero']
        self.tdouble = self.config['tdouble']
        self.outfile = self.config['outfile'].format(self.config['parentpid'])
        self.errfile = self.config['errfile'].format(self.config['parentpid'])
        if self.tzero < 1:
            raise Exception  # ToDo: define exception class
       
        self.running = True
        super(Poller, self).__init__()

    def run(self):
        """ Poll untill the stop function has been called."""
        while self.running:
            try:
                stop = self.config['stop']
            except KeyError:
                pass
            else:
                if stop:
                    self.stop()
                    continue
            clock = polltimer(self.tzero, self.tdouble)
            output_vals = output.Output()
            try:
                pidlist = self.config['obspids']
            except KeyError:
                clock.sleep()
                continue
            for pid in self.config['obspids']:
                pps = SP.Popen([self.poll_execpath, str(pid)], stdin=SP.PIPE, stdout=SP.PIPE, stderr=SP.PIPE)
                out, err = pps.communicate()
                tstamp = str(time.time())
                if out != "":
                    out = "{0} {1} {2}".format(tstamp, self.config['pidnames'][pid], out.strip())
                if err != "":
                    err = "{0} {1} {2}".format(tstamp, self.config['pidnames'][pid], err.strip())
                output_vals[pid] = (out, err)
            output_vals.tofile(self.outfile, self.errfile)
            clock.sleep()

    def stop(self):
        """Stop the thread"""
        self.running = False
        
