#!/usr/bin/python

import sys
import os

libPath = '/opt/stationtest/lib'
sys.path.insert(0, libPath)

from general_lib import *
from lofar_lib import *
from test_lib import *
from test_db import cDB
import time

os.umask(001)

#import numpy as np

def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
    print "-c   : clean test (do not use last test results)"
    print "-l 2 : set level too 2 (default level is 0)"
    print "       level 0   : manual tests, use keys listed below"
    print "       level 1..n: see checkhardware.conf file for tests done"
    print 
    print "For testing only one part, set level too 0 and use one of the following keys"
    print "-s1  : LBA signal test rcumode 1"
    print "-s3  : LBA signal test rcumode 3"
    print "-s5  : HBA signal test rcumode 5"
    print "-m   : HBA modem test"
    print "-n1  : LBA noise and spurious test rcumode 1"
    print "-n3  : LBA noise and spurious test rcumode 3"
    print "-n5  : HBA noise and spurious test rcumode 5"
    print "-ne  : HBA noise and spurious test rcumode 5 element based"
    print "-rv  : RSP version test"
    print "-tv  : TBB version test"
    print "-tm  : TBB memeory test"
    
    
    print "----------------------------------------------------------------------------"
args = dict()
# get command line arguments
def getArguments():
    global args
    args['L'] = 0 # default test level
    i = 1
    while i < len(sys.argv):
        if sys.argv[i][0] == '-':
            opt = sys.argv[i][1:]
            if opt in ('t','T','l','L'): # for testing
                optval = sys.argv[i+1]
                args[opt.upper()] = optval.upper()
                i += 2
            else:
                args[opt.upper()] = '-'
                i += 1       

def setLevelTests(conf):
    global args
    level = args.get('L')
    tests = conf.getStr('level-%s-tests' %(level)).split(',')
    for tst in tests:
        args[tst.upper()]='-'
            

# get Configuration file
class cConfiguration:
    def __init__(self):
        self.conf = dict()
        f = open("/opt/stationtest/checkHardware.conf", 'r')
        data = f.readlines()
        f.close()
        for line in data:
            if line[0] in ('#','\n',' '):
                continue
            if line.find('#') > 0:
                line = line[:line.find('#')]
            try:
                key, value = line.strip().split('=')
                self.conf[key] = value
            except:
                print "Not a valid configuration setting: %s" %(line)
    
    def getInt(self,key, default=0):
        return (int(self.conf.get(key, str(default))))
    
    def getFloat(self,key, default=0.0):
        return (float(self.conf.get(key, str(default))))
    
    def getStr(self,key):
        return (self.conf.get(key, ''))
                
    
def main():
    global args
    getArguments()
    if args.has_key('H'):
        printHelp()
        sys.exit()
    
    conf = cConfiguration()
    
    setLevelTests(conf)

    StID = getHostName()
    
    logger = cLogger('/opt/stationtest/', 'checkHardware.log', screenPrefix=StID)
    
    # Read in RemoteStation.conf 
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()
    print  ID, nRSP, nTBB, nLBL, nLBH, nHBA
   
    # setup intern database
    db = cDB(StID, nRSP, nTBB, nLBL, nLBH, nHBA, clean=args.has_key('C'))
    
    logger.info('== START OF HARDWARE CHECK ==', screen=True)
    logger.resetStartTime(screen=True)
    
    logger.info("Check Level %s" %(args.get('L')), screen=True)
            
    # make temporary data directory in tmp dir
    
    if os.access(dataDir(), os.F_OK):
        removeAllDataFiles()
    else:
        os.mkdir(dataDir())
    
    writeMessage('!!!   (new version)  This station will be in use for a test! Please do not use the station!  (new version)   !!!')
    start_level = swlevel()
    sw_level = swlevel(2)
    # if swlevel is 2
    if sw_level == 2:
        tbb = cTBB(db, logger)
        rsp = cRSP(db, logger)
        
        # do RSP tests if requested
        if args.has_key('S1') or args.has_key('S3') or args.has_key('S5') or \
           args.has_key('N1') or args.has_key('N3') or args.has_key('N5') or args.has_key('NE') or \
           args.has_key('M') or args.has_key('RV'):
            # wait for RSP boards ready
            if rsp.waitReady():
                if args.has_key('RV'):
                    db.tests += ',RV'
                    rsp.checkVersions(conf.getStr('bp-version'), conf.getStr('ap-version'))
            
                # if right images loaded
                if rsp.checkVersions(conf.getStr('bp-version'), conf.getStr('ap-version')):
                    rsp.resetSettings()
                    rsp.waitReady()
                    rsp.turnonRCUs()
                    #rspctl("--rcuenable=0 --sel=0,1,8,11")
                
                    lbl = cLBA(db.lbl, logger)
                    lbh = cLBA(db.lbh, logger)
                    hba = cHBA(db.hba, logger)

                    rsp.swapXY(1)
                    if args.has_key('S1'):
                        db.tests += ',S1'
                        if StID in CoreStations or StID in RemoteStations:
                            lbl.checkSignal(mode=1, subband=conf.getInt('lbl-test-sb',301),
                                                    min_deviation=conf.getFloat('lba-min-deviation', -2.0), 
                                                    max_deviation=conf.getFloat('lba-max-deviation', 2.0))
                    if args.has_key('N1'):
                        db.tests += ',N1'
                        if StID in CoreStations or StID in RemoteStations:
                            lbl.checkNaS(mode=1, sampletime=180)  # sampletime in seconds
                
                    rsp.swapXY(0)               
                    if args.has_key('S3'):
                        db.tests += ',S3'
                        lbh.checkSignal(mode=3, subband=conf.getInt('lbh-test-sb',301),
                                                min_deviation=conf.getFloat('lba-min-deviation', -2.0), 
                                                max_deviation=conf.getFloat('lba-max-deviation', 2.0))
                    if args.has_key('N3'):
                        db.tests += ',N3'
                        lbh.checkNaS(mode=3, sampletime=180)  # sampletime in seconds
                        
                    if args.has_key('M'):
                        db.tests += ',M'   
                        hba.checkModem()
                        hba.turnOffBadTiles()
                
                    if args.has_key('N5'):
                        db.tests += ',N5'   
                        hba.checkNaS(mode=5, sampletime=180)  # sampletime in seconds
                
                    if args.has_key('NE'):
                        db.tests += ',NE'   
                        hba.checkNaS_elements(mode=5, sampletime=120)
    
                    if args.has_key('S5'):
                        db.tests += ',S5'   
                        hba.checkSignal(mode=5, subband=conf.getInt('hba-test-sb',155),
                                                min_deviation=conf.getFloat('hba-min-deviation', -24.0), 
                                                max_deviation=conf.getFloat('hba-max-deviation', 12.0))    
                
                    rsp.turnoffRCUs()
                
        # do TBB tests if requested
        if args.has_key('TV') or args.has_key('TM'):
            # wait for RSP boards ready
            if tbb.waitReady() == 1:
                if args.has_key('TV'):
                    db.tests += ',TV'
                    tbb.checkVersions(conf.getStr('tbbdriver-version'), conf.getStr('tbbctl-version'),
                                      conf.getStr('tp-version'), conf.getStr('mp-version'))
                if args.has_key('TM'):
                    db.tests += ',TM'
                    tbb.checkMemory()


    # do db test and write result files to log directory
    db.test(conf.getStr('log-dir'))
    
    swlevel(start_level)
    logger.printBusyTime(screen=True)
    writeMessage('!!!     The test is ready and the station can be used again!               !!!')
    
    # delete files from data directory
    removeAllDataFiles()


if __name__ == '__main__':
    main()
