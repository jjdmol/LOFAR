#!/usr/bin/python

## "isReset48V.py"
## RESET 48V powersupply on IS (international station)
## can only be used on IS (international) LCU
##
## usage: ./isReset48V.py
##
## Author: Pieter Donker (ASTRON)
## Last change: november 2011 

from isEcLib import *
import sys
import time

VERSION = '1.1.0' # version of this script    

# used variables
version = 0   # EC version
versionstr = 'V-.-.-'

##=======================================================================
## start of main program
##=======================================================================
if __name__ == '__main__':
    host = getIP()
    if host == None:
        print "==============================================="
        print "ERROR, this script can only run on a IS station"
        print "==============================================="
    else:
        ec = EC(host)
        ec.connectToHost()
        time.sleep(1.0)
        ec.printInfo(True)
        ec.getPowerStatus()
        ec.resetPower()
        print "waiting 10 seconds"
        time.sleep(10.0)
        ec.getPowerStatus()
        print "waiting 10 seconds"
        time.sleep(10.0)
        ec.getPowerStatus()
        ec.printInfo(False)
        ec.disconnectHost()


