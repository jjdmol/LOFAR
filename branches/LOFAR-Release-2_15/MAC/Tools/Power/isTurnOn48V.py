#!/usr/bin/python

## "isOff48V.py"
## Turn off 48V powersupply on IS (international station)
## can only be used on IS (international) LCU
##
## usage: ./isOff48V.py
##
## Author: Pieter Donker (ASTRON)
## Last change: May 2013 

from isEcLib import *
import sys
import time

VERSION = '0.0.1' # version of this script    

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
        ec.setPower(ec.P_48, ec.PWR_ON)
        print "waiting 10 seconds"
        time.sleep(10.0)
        ec.getPowerStatus()
        ec.printInfo(False)
        ec.disconnectHost()


