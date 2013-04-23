#!/usr/bin/python
#
# read last test log file (.csv)
# and send test result to PVSS,
# and write to PVSS log file
#
# P.Donker

import sys
import os
from time import sleep

libPath = '/opt/stationtest/lib'
sys.path.insert(0, libPath)

from general_lib import *
from lofar_lib import *

args = dict()
logdir = ""
logger = 0
nLBL = 0
nLBH = 0
nHBA = 0

# PVSS states
State = dict({'OFF':0, 'OPERATIONAL':1, 'MAINTENANCE':2, 'TEST':3, 'SUSPICIOUS':4, 'BROKEN':5})



def main():
    global args, logdir, logger, nLBL, nLBH, nHBA
    args   = getArguments()
    logdir = getLogDir()
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()
    logger = cPVSSLogger(logdir)

    if args.has_key('RESET'):
        resetPVSS(0)

    else:
        # read last log file from checkhardware
        testfilename = '%s_StationTest.csv' %(getHostName())
        fullFilename = os.path.join(logdir, testfilename)
        f = open(fullFilename, 'r')
        testdata = f.readlines()
        f.close()

        addDataToPVSS(testdata)

# print help screen
def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
    print "-h     : this help screen"
    print "-reset : clean all, set all antennas too ok"
    print "-test  : do not send to PVSS"

# get command line arguments
def getArguments():
    args = dict()
    for i in range(len(sys.argv)):
        if sys.argv[i][0] == '-':
            if sys.argv[i].find('=') != -1:
                args[sys.argv[i][1:valpos].upper()] = int(sys.argv[i][valpos+1:])
            else:
                args[sys.argv[i][1:].upper()]='-'

        if args.has_key('H'):
            printHelp()
            sys.exit()
    return(args)

# get logdir from configuration file
def getLogDir():
    logdir = ""
    # look for log directory
    f = open("/opt/stationtest/checkHardware.conf", 'r')
    data = f.readlines()
    f.close()
    for line in data:
        if line.find('log-dir') != -1:
            key, logdir = line.strip().split('=')
    return(logdir)

# send comment, key and value to PVSS and write to file
def sendToPVSS(comment, pvss_key, value):
    global logger
    if len(comment) > 0:
        comment = 'stationtest::'+comment
    else:
        comment = 'stationtest'
    arguments = '%s %s %d' %(comment, pvss_key, value)
    logger.addLine(arguments[11:])
    if args.has_key('TEST'):
        print arguments
    else:
        response = sendCmd('setObjectState', arguments)
        sleep(0.05)
        return(response)
    return("")

# set all antenna info to ok
def resetPVSS(state=0):
    global State, nLBL, nLBH, nHBA
    
    filename = "reset_pvss.log"
    full_filename = os.path.join(libPath, filename)
    f = open(full_filename, 'w')
        
    for ant in range(nLBH):
        f.write("LOFAR_PIC_LBA%03d %d\n" %(ant, state))

    for ant in range(nLBL):
        f.write("LOFAR_PIC_LBA%03d %d\n" %(ant+48, state))

    for tile in range(nHBA):
        f.write("LOFAR_PIC_HBA%02d %d\n" %(tile, state))
        
        for elem in range(16):
            f.write("LOFAR_PIC_HBA%02d.element%02d %d\n" %(tile, elem, state))
            f.write("LOFAR_PIC_HBA%02d.element%02d.comm %d\n" %(tile, elem, state))
            f.write("LOFAR_PIC_HBA%02d.element%02d.X %d\n" %(tile, elem, state))
            f.write("LOFAR_PIC_HBA%02d.element%02d.Y %d\n" %(tile, elem, state))
    
    sendCmd("setObjectState", "stationtes:reset %s" %(full_filename))
        
# add result data from checkhardware to PVSS
def addDataToPVSS(data):
    global State
    for line in data:
        if line[0] == '#':
            continue
        keyinfo = dict()
        info = line.split(',')
        #print info
        date    = info[0]
        part    = info[1]
        partNr = '---'
        if info[2] != '---':
            partNr  = int(info[2])
        msgType = info[3].strip()
        for i in range(4,len(info)):
                if info[i].find('=') != -1:
                    key, valstr = info[i].split('=')
                    vallist = valstr.split()
                    if len(vallist) == 1:
                        keyinfo[key] = vallist[0]
                    elif len(vallist) > 1:
                        keyinfo[key] = vallist
                else:
                    keyinfo[info[i]] = '-'

        if part == 'LBL':
            if msgType == 'LOW_NOISE':
                sendToPVSS("low-noise", "LOFAR_PIC_LBA%03d" %(partNr+48), State['BROKEN'])

            elif msgType == 'HIGH_NOISE':
                sendToPVSS("high-noise", "LOFAR_PIC_LBA%03d" %(partNr+48), State['BROKEN'])

            if msgType == 'FAIL':
                comment = "rf-fail-"
                if keyinfo.has_key('X'):
                    comment += "X"
                    #sendToPVSS("rf-fail", "LOFAR_PIC_LBA%03d.X" %(partNr+48), State['BROKEN'])
                if keyinfo.has_key('Y'):
                    comment += "Y"
                    #sendToPVSS("rf-fail", "LOFAR_PIC_LBA%03d.Y" %(partNr+48), State['BROKEN'])
                sendToPVSS(comment, "LOFAR_PIC_LBA%03d" %(partNr+48), State['BROKEN'])
                
            elif msgType == 'DOWN':
                sendToPVSS("down", "LOFAR_PIC_LBA%03d" %(partNr+48), State['BROKEN'])
                #sendToPVSS("Down", "LOFAR_PIC_LBA%03d.X" %(partNr+48), State['BROKEN'])
                #sendToPVSS("Down", "LOFAR_PIC_LBA%03d.Y" %(partNr+48), State['BROKEN'])

        if part == 'LBH':
            if msgType == 'LOW_NOISE':
                sendToPVSS("low-noise", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])

            elif msgType == 'HIGH_NOISE':
                sendToPVSS("high-noise", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])

            elif msgType == 'FAIL':
                comment = "rf-fail-"
                if keyinfo.has_key('X'):
                    comment += "X"
                    #sendToPVSS("rf-fail", "LOFAR_PIC_LBA%03d.X" %(partNr), State['BROKEN'])
                if keyinfo.has_key('Y'):
                    comment += "Y"
                    #sendToPVSS("rf-fail", "LOFAR_PIC_LBA%03d.Y" %(partNr), State['BROKEN'])
                sendToPVSS(comment, "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
            elif msgType == 'DOWN':
                sendToPVSS("down", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                #sendToPVSS("Down", "LOFAR_PIC_LBA%03d.X" %(partNr), State['BROKEN'])
                #sendToPVSS("Down", "LOFAR_PIC_LBA%03d.Y" %(partNr), State['BROKEN'])

        if part == 'HBA':
            if msgType == 'LOW_NOISE':
                sendToPVSS("low-noise", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])

            elif msgType == 'HIGH_NOISE':
                sendToPVSS("high-noise", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])

            elif msgType == 'C_SUMMATOR':
                sendToPVSS("Moden-fail", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])

            elif msgType == 'FAIL':
                sendToPVSS("", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                for elem_nr in range(1,17,1):

                    if    keyinfo.has_key('M%d' %(elem_nr)) \
                       or keyinfo.has_key('X%d' %(elem_nr)) \
                       or keyinfo.has_key('Y%d' %(elem_nr)) \
                       or keyinfo.has_key('LNX%d' %(elem_nr)) \
                       or keyinfo.has_key('HNX%d' %(elem_nr)) \
                       or keyinfo.has_key('LNY%d' %(elem_nr)) \
                       or keyinfo.has_key('HNY%d' %(elem_nr)):
                        sendToPVSS("rf-fail", "LOFAR_PIC_HBA%02d.element%02d" %(partNr, elem_nr-1), State['BROKEN'])

                    if keyinfo.has_key('M%d' %(elem_nr)):
                        sendToPVSS("rf-fail", "LOFAR_PIC_HBA%02d.element%02d.comm" %(partNr, elem_nr-1), State['BROKEN'])
                    
                    comment = ""
                    if    keyinfo.has_key('X%d' %(elem_nr)) \
                       or keyinfo.has_key('LNX%d' %(elem_nr)) \
                       or keyinfo.has_key('HNX%d' %(elem_nr)):
                        
                        if keyinfo.has_key('X%d' %(elem_nr)):
                            comment += "rf-fail&"
    
                        if keyinfo.has_key('LNX%d' %(elem_nr)):
                            comment += "low-noise&"
                        
                        if keyinfo.has_key('HNX%d' %(elem_nr)):
                            comment += "high-noise&"
                        sendToPVSS(comment[:-1], "LOFAR_PIC_HBA%02d.element%02d.X" %(partNr, elem_nr-1), State['BROKEN'])
                    
                    comment = ""
                    if    keyinfo.has_key('Y%d' %(elem_nr)) \
                       or keyinfo.has_key('LNY%d' %(elem_nr)) \
                       or keyinfo.has_key('HNY%d' %(elem_nr)):
                        
                        if keyinfo.has_key('Y%d' %(elem_nr)):
                            comment += "rf-fail&"
    
                        if keyinfo.has_key('LNY%d' %(elem_nr)):
                            comment += "low-noise&"
                        
                        if keyinfo.has_key('HNY%d' %(elem_nr)):
                            comment += "high-noise&"
                        sendToPVSS(comment[:-1], "LOFAR_PIC_HBA%02d.element%02d.Y" %(partNr, elem_nr-1), State['BROKEN'])


if __name__ == "__main__":
    main()