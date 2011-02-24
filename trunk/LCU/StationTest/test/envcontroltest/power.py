#!/usr/bin/python

## LOFAR script
## for reset, turn on or off the power to 48V or LCU
## P.Donker (ASTRON) 20-10-2009


## DO NOT CHANGE THINGS BELOW THIS LINE ##
## ------------------------------------ ##

from stations import *
from eccontrol import *
#import socket
import time
import struct
import sys
import string

VERSION = '1.0.1' # version of this script
 
if len(sys.argv) < 3:
    print '--------------------------------------------'
    print 'Error: no arguments found'
    print '--------------------------------------------'
    print 'usage power.py host cmd'
    print '    host = this (get IP based on LCU)'
    print '    host = ip (set ip manualy)'
    print '    cmd  = status (show status)'
    print '    cmd  = resetTrip (reset temperature trip system)'
    print '    cmd  = reset48 (reset 48V power supply)'
    print '    cmd  = resetLCU (reset LCU power supply)'
    print '    cmd  = on48 (turn on 48V power supply)'
    print '    cmd  = off48 (turn off 48V power supply)'
    print '    cmd  = onLCU (turn on LCU power supply)'
    print '    cmd  = offLCU (turn off LCU power supply)'
    print '*only 1 cmd per power supply can be given'
    print '--------------------------------------------'
    exit(0)


doResetTrip   = 0
# select only 1 
doReset48V    = 0
doPowerOn48V  = 0
doPowerOff48V = 0
# select only 1
doResetLCU    = 0
doPowerOnLCU  = 0
doPowerOffLCU = 0

Station = sys.argv[1]

for arg in sys.argv[2:]:
    cmd = str(arg)
    if cmd == 'resetTrip':
        doResetTrip = 1
    if cmd == 'reset48':
        doReset48V = 1
    if cmd == 'resetLCU':
        doResetLCU = 1
    if cmd == 'on48':
        doPowerOn48V = 1
    if cmd == 'off48':
        doPowerOff48V = 1
    if cmd == 'onLCU':
        doPowerOnLCU = 1
    if cmd == 'offLCU':
        doPowerOffLCU = 1

##=======================================================================
## start of main program
##=======================================================================
if __name__ == "__main__":
    logfile = open('ecPowerActions.log','a')
    
    if doResetTrip or doReset48V or doResetLCU: 
        name = raw_input('Enter your personal name: ')
    if doPowerOn48V or doPowerOff48V or doPowerOnLCU or doPowerOffLCU:
        name = raw_input('Enter your personal name: ')
    
    stations = STATION()
    
    if ((doReset48V + doPowerOn48V + doPowerOff48V) > 1):
        print 'error more than 1 48V cmd selected'
        exit(-1)
    if ((doResetLCU + doPowerOnLCU + doPowerOffLCU) > 1):
        print 'error more than 1 LCU cmd selected'
        exit(-1)
        
    for station in stations.ec(Station):
        ec = EC(station)
        try:
            (hostname, aliaslist, ipaddrlist) = socket.gethostbyaddr(station)
            hostname = hostname.split('.')[0]
        except:
            hostname = '  ???  '
        
        if ec.connectToHost() == True:
            ec.setSecond(int(time.gmtime()[5]))
            ## do not change if statements
            ec.printInfo(True)
            ec.getPowerStatus()
            if (doResetTrip == 1):
                logfile.write('%sc, %s, ecResetTrip by %s\n' %(string.upper(hostname), time.asctime(), name))
                if ec.getTripStatus():
                    ec.resetTrip()
            if (doReset48V == 1):
                logfile.write('%sc, %s, ecReset48 by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.resetPower(ec.P48)
            if (doResetLCU == 1):
                logfile.write('%sc, %s, ecResetLCU by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.resetPower(ec.LCU)
            if (doPowerOn48V == 1):
                logfile.write('%sc, %s, ec48On by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.setPower(48,ec.PWR_ON)
            if (doPowerOff48V == 1):
                logfile.write('%sc, %s, ec48Off by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.setPower(48,ec.PWR_OFF)
            if (doPowerOnLCU == 1):
                logfile.write('%sc, %s, ecLcuOn by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.setPower(ec.LCU,ec.PWR_ON)
            if (doPowerOffLCU == 1):
                logfile.write('%sc, %s, ecLcuOff by %s\n' %(string.upper(hostname), time.asctime(), name))
                ec.setPower(ec.LCU,ec.PWR_OFF)
            
            # print status again after 10 seconds if cmd is send 
            if (doResetTrip or doReset48V or doResetLCU or doPowerOn48V or doPowerOff48V or doPowerOnLCU or doPowerOffLCU):
                print "waiting 10 seconds"
                time.sleep(10.0)
                ec.getPowerStatus()

            # print status again after 10 seconds if power reset cmd is send 
            if (doReset48V or doResetLCU):
                print "waiting 10 seconds"
                time.sleep(10.0)
                ec.getPowerStatus()

            ec.printInfo(False)
            ec.disconnectHost()
