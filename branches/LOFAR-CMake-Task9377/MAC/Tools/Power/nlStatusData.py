#!/usr/bin/python

"""
write status-data to stdout
stdout format:
    time [0] data_cab0 [1] data_cab1 [3] data_cab3

values in  data_cabx:
    temperature humidity fansstate heaterstate
        temperature : actual temperature in cabinet 
        humidity    : actual humidity in cabinet
        fanstate    : which fans are on
                      bit 0 outer fan front
                      bit 1 inner fan front
                      bit 2 inner fan back
                      bit 4 outer fan back
        heaterstate : only available in cabinet 3
                      0 = off
                      1 = on

example, returned data:
1333702601 [0] 24.71 16.81 4 0 [1] 24.72 43.36 4 0 [3] 14.69 41.73 2 0 

"""

#from eccontrol import *
#from stations import *
import nlEcLib as eclib
import sys
import time

VERSION = '1.0.0' # version of this script    
Station = ''

##=======================================================================
## start of main program
##=======================================================================
def main():
    ec = eclib.EC(eclib.getIP())
    ec.connectToHost()
    ec.printInfo(False) # print NOT to screen   
   
    # version is used to check if function is available in firmware
    PL2 = ec.getStatusData()
    print '%1.0f' %(time.time()),
    cabs = [0,1,3]
    for cab in cabs:
        # print cabnr, temperature, humidity, fansstate, heaterstate
        print '[%d] %1.2f %1.2f %d %d' %\
                ( cab, PL2[(cab*7)+2]/100., PL2[(cab*7)+3]/100.,
                  PL2[(cab*7)+4] & 0x0f, PL2[(cab*7)+6]),
    print    
    
    ##----------------------------------------------------------------------
    ## do not delete next lines
    ec.disconnectHost()


if __name__ == '__main__':
    main()

