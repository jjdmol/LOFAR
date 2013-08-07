#!/usr/bin/python

## "nlStatus.py"
## Print EC status of NL(dutch) station
## can only be used on NL (dutch) LCU
##
## usage: ./nlStatus.py
##
## Author: Pieter Donker (ASTRON)
## Last change: november 2011 

from nlEcLib import *
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
    #print host
    if host == None:
        print "==============================================="
        print "ERROR, this script can only run on a NL station"
        print "==============================================="
    else:
        ec = EC(host)
        ec.connectToHost()
        time.sleep(1.0)
        # version is used to check if function is available in firmware
        version,versionstr  = ec.getVersion()  
        ec.printInfo(True)
        print ""
        ec.getStationInfo()
        ec.getStatus()
        print ""
        ec.getTripStatus()
        print ""
        ec.printInfo(False)
        ec.disconnectHost()


