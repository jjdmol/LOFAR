#!/usr/bin/env python

import os
import sys
import signal
import threading
import time

from Stations import *
from optparse import OptionParser

command = '"(' + os.getcwd() + '/BGPStats.sh)" 2>/dev/null'
killcommand = 'killall BGLStats 2> /dev/null'
killcommand2 = 'killall tcpdump 2> /dev/null'

def failed(ionode):
  print
  print ('No LOFAR data on IO node ' + ionode +'. Either the station data is not arriving, or')
  print ('the Blue Gene/L partition is not active.')

class CommandThread(threading.Thread):
  def __init__(self, commandstring, ionode):
    threading.Thread.__init__(self)
    self.commandstring = commandstring
    self.ionode = ionode

  def run(self): 
    # this may hang for a very long time
    os.system(self.commandstring)

if __name__ == '__main__':

    parser = OptionParser()

    parser.add_option("--stationlist", default="CS001_lba0", action="store", type="string", dest="stationlist", help="Stations to check. Use either a station name (e.g. LBA) or 'all' for all CS1 stations.")
    # parse the options
    (options, args) = parser.parse_args()

    # read the stations from Stations.py
    # todo: WARNING this is very dangerous, because there could be any code in the station string
    # the exec should probably be replaced by something safer, but this is only a temporary script
    # This way is convenient because a user can say CS10_4dipoles + CS01_4dipoles
    try:
	exec('stationList =' + options.stationlist)
    except:
	print 'Cannot parse station configuration: ' + str(options.stationlist)
	sys.exit(1)

    threads = []
    
    for ionode in stationList:
        t = CommandThread('ssh ' + str(ionode.getIONode()) + ' ' + command, ionode.getIONode())
        threads.append(t)
        t.setDaemon(1)
        t.start()
	time.sleep(1)

    for t in threads:
        t.join(3)
        if (t.isAlive()):
	    failed(t.ionode)

            killstring = 'ssh ' + t.ionode + ' ' + killcommand
            os.system(killstring)
            killstring = 'ssh ' + t.ionode + ' ' + killcommand2
            os.system(killstring)

        else:
            threads.remove(t)
signal.alarm(0)
sys.exit(0)









