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

        # is this correct? I do not see the numwaits being increased.
        # further the if statement is always true.
        # What happens of tdouble is 0?
        if self.numwaits > self.tdouble:
            self.numwaits = 0
            self.slptim += 1
        else:
            self.numwaits += 1
        time.sleep(self.slptim)   


def pid_is_active(pid):        
    """ Check For the existence of a unix pid. """
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    else:
        return True

        
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
            # this is fragile.
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
        itter = 0
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
                clock.sleep()  #!!!! What happens here?
                continue

            spids = self.config['stoppedpids'].copy()
            for pid in spids:
                tstamp = str(time.time())
                out = "{0} {1} {2}".format(tstamp, self.config['pidnames'][pid], "stopped")
                del(self.config['pidnames'][pid])
                self.config['stoppedpids'].remove(pid)
                self.config['obspids'].remove(pid)
                output_vals[pid] = (out, out)

            starts = self.config['startpids'].copy()
            for sp in starts:
                self.config['obspids'].add(sp)
                self.config['startpids'].remove(sp)

            ## tear down monitor if all pid to monitor are dead
            #if itter > 10 and len(self.config['obspids']) == 0:
            #    self.stop()
            #    raise Exception("bwaaaaa")


            dead_pids = []
            # here the call with the script getting info from proc
            for pid in self.config['obspids']:
                # skip logging this pid
                if not pid_is_active(pid):
                    dead_pids.append(pid)
                    continue

                pps = SP.Popen([self.poll_execpath, str(pid)], stdin=SP.PIPE, stdout=SP.PIPE, stderr=SP.PIPE)
                out, err = pps.communicate()
                tstamp = str(time.time())
                if out != "":
                    out = "{0}".format( out.strip())
                if err != "":
                    err = "{0} {1} {2}".format(tstamp, self.config['pidnames'][pid], err.strip())
                output_vals[pid] = (out, err)

            output_vals.tofile(self.outfile, self.errfile)

            # remove dead pids from the scan list
            if len(dead_pids) > 0:
                for pid in dead_pids:
                    self.config['obspids'].remove(pid)
            itter+=1
            clock.sleep()

    def stop(self):
        """Stop the thread"""
        self.running = False
        
