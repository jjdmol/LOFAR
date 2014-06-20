#!/usr/bin/python

## "isStatus.py"
## Print EC status of IS (international station)
## can only be used on IS LCU
##
## usage: ./isStatus.py
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
        # version is used to check if function is available in firmware
        version,versionstr  = ec.getVersion()  
        ec.printInfo(True)
        ec.getStatus()
        ec.printInfo(False)
        ec.disconnectHost()


