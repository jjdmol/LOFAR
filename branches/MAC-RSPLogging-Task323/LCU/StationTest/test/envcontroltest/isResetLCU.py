#!/usr/bin/python

## "isResetLCU.py"
## RESET LCU power on IS (international station)
## can only be used on IS LCU
##
## USE ONLY IF NORMAL SHUTDOWN IS NOT POSSIBLE
## usage: ./isResetLCU.py
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
        print "Turn off the mains voltage for 10 seconds"
        print "Use only if normal shutdown in not possible"
        if 'yes' == raw_input("Do you realy want to cycle LCU power [yes/no] : "):
            print
            print "================================"
            print "  cycle LCU power in 5 seconds  "
            print "================================"
            time.sleep(5.0)
            ec.resetLCU()


