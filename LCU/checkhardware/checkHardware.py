#!/usr/bin/python

versiondate = '23 May 2013'

import sys
import os

mainPath = r'/opt/stationtest'
libPath  = os.path.join(mainPath, 'lib')
sys.path.insert(0, libPath)

import time
import datetime
import logging

from general_lib import *
from lofar_lib import *
from test_lib import *
from test_db import *

os.umask(001)

logger = None

def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
    print "-l=2              : set level to 2 (default level is 0)"
    print "                  level 0    : manual checks, use keys listed below"
    print "                  level 1..n : see checkhardware.conf file for checks done"
    print
    print "To start a long check set number of runs or start and stop time, if start time"
    print "is not given the first run is started immediately"
    print "-r=1              : repeats, number of runs to do"
    print "-start=[date_time]: start time of first run, format [YYYYMMDD_HH:MM:SS]"
    print "-stop=[date_time] : stop time of last run, format [YYYYMMDD_HH:MM:SS]"
    print
    print "Set logging level, can be: debug|info|warning|error"
    print "-ls=debug         : print all information on screen, default=info"
    print "-lf=info          : print debug|warning|error information to log file, default=debug"
    print
    print "Select checks to do, can be combined with all levels"
    print "-s(rcumode)       : signal check for rcumode (1|3|5)"
    print "-o(rcumode)       : oscillation check for rcumode (1|3|5)"
    print "-sp(rcumode)      : spurious check for rcumode (1|3|5)"
    print "-n(rcumode)[=120] : noise check for rcumode (1|3|5), optional data time in seconds"
    print "                    default data time = 300 sec for hba and 180 sec for lba"
    print "-ehba[=120]       : do all HBA element tests, optional data time in seconds"
    print "                    default data time = 30 sec"
    print "-m                : HBA modem check, automatic selected if other hba check are selected"
    print "-sn               : HBA summator noise check"
    print
    print "-lbl              : do all LBL tests"
    print "-lbh              : do all LBH tests"
    print "-hba              : do all HBA tests"
    print
    print "-rv               : RSP version check"
    print "-tv               : TBB version check"
    print "-tm               : TBB memmory check"
    print
    print "example   : ./checkHardware.py -s5 -n5=180"


    print "----------------------------------------------------------------------------"

rcu_m1_keys  = ('LBL','O1','SP1','N1','S1')
rcu_m3_keys  = ('LBH','O3','SP3','N3','S3')
rcu_m5_keys  = ('HBA','M','O5','SN','SP5','N5','S5','EHBA')
rsp_keys     = ('RV',) + rcu_m1_keys + rcu_m3_keys + rcu_m5_keys
tbb_keys     = ('TV','TM')
control_keys = ('R','START','STOP')
all_keys     = control_keys + rsp_keys + tbb_keys
rsp_check    = False
rcu_m5_check = False
tbb_check    = False

args = dict()
# get command line arguments
testInfo = dict()
testInfo['START']  = "Start checks"
testInfo['STOP']   = "Stop  checks"
testInfo['R']      = "Number of test repeats set to"
testInfo['O1']     = "LBA mode-1 Oscillation test"
testInfo['SP1']    = "LBA mode-1 Spurious test"
testInfo['N1']     = "LBA mode-1 Noise test"
testInfo['S1']     = "LBA mode-1 RF test"
testInfo['O3']     = "LBA mode-3 Oscillation test"
testInfo['SP3']    = "LBA mode-3 Spurious test"
testInfo['N3']     = "LBA mode-3 Noise test"
testInfo['S3']     = "LBA mode-3 RF test"
testInfo['O5']     = "HBA mode-5 Oscillation test"
testInfo['SP5']    = "HBA mode-5 Spurious test"
testInfo['N5']     = "HBA mode-5 Noise test"
testInfo['S5']     = "HBA mode-5 RF test"
testInfo['EHBA']   = "HBA mode-5 Element tests"
testInfo['M']      = "HBA mode-5 Modem test"
testInfo['SN']     = "HBA mode-5 Summator noise test"
testInfo['RV']     = "RSP Version test"
testInfo['TM']     = "TBB Memory test"
testInfo['TV']     = "TBB Version test"

def addToArgs(key, value):
    if key == '':
        return
    global args, rsp_keys, rsp_check, rcu_m5_keys, rcu_m5_check, tbb_keys, tbb_check
    if key in rsp_keys or key in tbb_keys or key in ('H','L','LS','LF','R','START','STOP'):
        if value != '-':
            args[key] = value
        else:   
            if key == 'LBL':
                args['O1'] = '-'
                args['SP1'] = '-'
                args['N1'] = '-'
                args['S1'] = '-'
            elif key == 'LBH':
                args['O3'] = '-'
                args['SP3'] = '-'
                args['N3'] = '-'
                args['S3'] = '-'
            elif key == 'HBA':
                args['O5'] = '-'
                args['SN'] = '-'
                args['SP5'] = '-'
                args['N5'] = '-'
                args['S5'] = '-'
            else:    
                args[key] = '-'
        
        if key in rcu_m5_keys:
            rcu_m5_check = True
        if key in rsp_keys:
            rsp_check = True
        if key in tbb_keys:
            tbb_check = True
    else:
        sys.exit("Unknown key %s" %(key))
    return
    
           
def getArguments():
    key   = ''
    value = '-'
    for arg in sys.argv[1:]:
        if arg[0] == '-':
            opt = arg[1:].upper()
            valpos = opt.find('=')
            if valpos != -1:
                key, value = opt.strip().split('=')
            else:
                key, value = opt, '-'
            addToArgs(key=key, value=value)
    return
                       
# get checklevel and set tests to do
def setLevelTests(conf):
    global args
    
    level = args.get('L', '0')
    if level == '0':
        return
    tests = conf.getStr('level-%s-tests' %(level)).split(',')
    for tst in tests:
        opt = tst.upper()
        valpos = opt.find('=')
        if valpos != -1:
            key, value = opt.strip().split('=')
        else:
            key, value = opt, '-'
        key = tst.upper()
        addToArgs(key=key, value=value)
    return

# get and unpack configuration file
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
                key = key.replace('_','-')
                self.conf[key] = value
            except:
                print "Not a valid configuration setting: %s" %(line)

    def getInt(self,key, default=0):
        return (int(self.conf.get(key, str(default))))

    def getFloat(self,key, default=0.0):
        return (float(self.conf.get(key, str(default))))

    def getStr(self,key):
        return (self.conf.get(key, ''))


# setup default python logging system
# logstream for screen output
# filestream for program log file
def init_logging():
    global logger

    log_levels = {'DEBUG'  : logging.DEBUG,
                  'INFO'   : logging.INFO,
                  'WARNING': logging.WARNING,
                  'ERROR'  : logging.ERROR}

    try:
        screen_log_level = args.get('LS', 'INFO')
        file_log_level   = args.get('LF', 'DEBUG')
    except:
        print "Not a legal log level, try again"
        sys.exit(-1)

    station = getHostName()

    # create logger
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    # create file handler
    full_filename = os.path.join(mainPath, 'checkHardware.log')
    #backup_filename = os.path.join(mainPath, 'checkHardware_bk.log')
    #sendCmd('cp', '%s %s' %(full_filename, backup_filename))
    file_handler = logging.FileHandler(full_filename, mode='w')
    formatter = logging.Formatter('%(asctime)s %(levelname)-8s %(message)s')
    file_handler.setFormatter(formatter)
    file_handler.setLevel(log_levels[file_log_level])
    logger.addHandler(file_handler)

    if len(logger.handlers) == 1:
        # create console handler
        stream_handler = logging.StreamHandler()
        fmt = '%s %%(levelname)-8s %%(message)s' %(station)
        formatter = logging.Formatter(fmt)
        stream_handler.setFormatter(formatter)
        stream_handler.setLevel(log_levels[screen_log_level])
        logger.addHandler(stream_handler)
    return


def waitForStart(start_datetime):
    start_time = time.mktime(start_datetime.timetuple())
    if start_time > time.time():
        logger.info("delayed start, sleep till %s" %(time.asctime(start_datetime.timetuple())))
    
    while start_time > time.time():
        wait_time = start_time - time.time()
        sleep_time = min(wait_time, 3600.0)
        time.sleep(sleep_time)
    return


def main():
    global args
    global all_keys
    global rsp_check, rcu_m5_check, tbb_check
    global logger

    getArguments()
    #print args
    if len(args) == 0 or args.has_key('H'):
        printHelp()
        sys.exit()

        
    init_logging()
    init_lofar_lib()
    init_test_db()
    init_test_lib()

    conf = cConfiguration()

    setLevelTests(conf)

    StID = getHostName()

    logger.info('== START HARDWARE CHECK ==')
    logger.info("== requested checks and settings ==")
    logger.info("-"*40)
    for i in all_keys:
        if args.has_key(i):
            if args.get(i) == '-':
                logger.info(" %s" %(testInfo.get(i)))
            else:
                logger.info(" %s, time = %s" %(testInfo.get(i), args.get(i)))
    logger.info("-"*40)
    
    if os.access(dataDir(), os.F_OK):
        removeAllDataFiles()
    else:
        os.mkdir(dataDir())

    # use format YYYYMMDD_HH:MM:SS
    start_time = time.time()
    stop_time = -1
    if args.has_key('STOP'):
        stop = args.get('STOP')
        if len(stop) != 17:
                sys.exit("wrong stoptime format must be YYYYMMDD_HH:MM:SS")
        stop_datetime = datetime.datetime(int(stop[:4]),int(stop[4:6]),int(stop[6:8]), \
                                          int(stop[9:11]),int(stop[12:14]),int(stop[15:]))
        stop_time = time.mktime(stop_datetime.timetuple())
        
        if args.has_key('START'):
            start = args.get('START')
            if len(start) != 17:
                sys.exit("wrong starttime format must be YYYYMMDD_HH:MM:SS")
            start_datetime = datetime.datetime(int(start[:4]),int(start[4:6]),int(start[6:8]), \
                                               int(start[9:11]),int(start[12:14]),int(start[15:]))

            waitForStart(start_datetime)
        
        logger.info("run checks till %s" %(time.asctime(stop_datetime.timetuple())))
    
    
    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()

    # setup intern database with station layout
    db = cDB(StID, nRSP, nTBB, nLBL, nLBH, nHBA)

    db.check_start_time = time.gmtime()
    writeMessage('!!!  This station will be in use for a test! Please do not use the station!  (script version %s)  !!!' %(versiondate))
    start_level = swlevel()
    sw_level = swlevel(2)
    if start_level == 1:
        logger.info("Wait 30 seconds while startup RSPDriver")
        time.sleep(30.0)
    if sw_level == 2:
        tbb = cTBB(db)
        rsp = cRSP(db)

        # do RSP tests if requested
        if rsp_check == True:
            # check if RSPDriver is running
            if checkActiveRSPDriver() == False:
                logger.warn("RSPDriver not running")
            else:
                # wait for RSP boards ready
                if waitRSPready():
                    if args.has_key('RV'):
                        rsp.checkVersions(conf.getStr('bp-version'), conf.getStr('ap-version'))
    
                    resetRSPsettings()
    
                    lbl = cLBA(db, db.lbl)
                    lbh = cLBA(db, db.lbh)
                    hba = cHBA(db, db.hba)
    
                    repeats = int(args.get('R','1'))
                    repeat_cnt = 1
    
                    runtime = 0
                    db.tests = ''
                    while (repeat_cnt <= repeats) \
                       or ((stop_time > -1) and ((time.time() + runtime) < stop_time)):
    
                        runstart = time.time()
                        if stop_time > -1:
                            logger.info("\n=== Start testrun %d ===\n" %(repeat_cnt))
                        else:
                            logger.info("\n=== Start testrun %d of %d ===\n" %(repeat_cnt, repeats))
                        
                        # do all rcumode 1 tests if available
                        if StID in CoreStations or StID in RemoteStations:
                            if args.has_key('O1'):
                                lbl.checkOscillation(mode=1)
        
                            if args.has_key('SP1'):
                                lbl.checkSpurious(mode=1)
        
                            if args.has_key('N1'):
                                if args.get('N1') == '-':
                                    recordtime = 180
                                else:
                                    recordtime = int(args.get('N1'))
                                lbl.checkNoise(mode=1,
                                               record_time=recordtime,
                                               low_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                               high_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                               max_diff=conf.getFloat('lba-noise-max-difference', 2.0))
        
                            if repeat_cnt == 1 and args.has_key('S1'):
                               lbl.checkSignal(mode=1,
                                               subband=conf.getInt('lbl-test-sb',301),
                                               min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                               low_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                               high_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))                        
                        
                        # do all rcumode 3 tests        
                        if args.has_key('O3'):
                            lbh.checkOscillation(mode=3)
    
                        if args.has_key('SP3'):
                            lbh.checkSpurious(mode=3)
    
                        if args.has_key('N3'):
                            if args.get('N3') == '-':
                                recordtime = 180
                            else:
                                recordtime = int(args.get('N3'))
                            lbh.checkNoise(mode=3,
                                           record_time=recordtime,
                                           low_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                           high_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                           max_diff=conf.getFloat('lba-noise-max-difference', 1.5))
                      
                        if repeat_cnt == 1 and args.has_key('S3'):
                            lbh.checkSignal(mode=3, 
                                            subband=conf.getInt('lbh-test-sb',301),
                                            min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                            low_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                            high_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))
    
                        # do all rcumode 5 tests    
                        # do always a modem check if an other hba check is requested
                        if rcu_m5_check:
                            hba.checkModem(mode=5)
                            hba.turnOffBadTiles()
    
                        if args.has_key('O5'):
                            hba.checkOscillation(mode=5)
    
                        if args.has_key('SN'):
                            hba.checkSummatorNoise(mode=5)
    
                        if args.has_key('SP5'):
                            hba.checkSpurious(mode=5)
    
                        if args.has_key('N5'):
                            if args.get('N5') == '-':
                                recordtime = 300
                            else:
                                recordtime = int(args.get('N5'))
                            hba.checkNoise(mode=5,
                                           record_time=recordtime,
                                           low_deviation=conf.getFloat('hba-noise-min-deviation', -3.0),
                                           high_deviation=conf.getFloat('hba-noise-max-deviation', 2.5),
                                           max_diff=conf.getFloat('hba-noise-max-difference', 2.0))
    
    
                        if repeat_cnt == 1 and args.has_key('S5'):
                            hba.checkSignal(mode=5, subband=conf.getInt('hba-test-sb',155),
                                                    min_signal=conf.getFloat('hba-rf-min-signal', 80.0),
                                                    low_deviation=conf.getFloat('hba-rf-min-deviation', -24.0),
                                                    high_deviation=conf.getFloat('hba-rf-max-deviation', 12.0))
                        
    
                        
                        # All element test
                        if args.has_key('EHBA'):
                            if args.get('EHBA') == '-':
                                recordtime = 15
                            else:
                                recordtime = int(args.get('EHBA'))
                            hba.checkElements(  mode=5, 
                                                record_time=recordtime,
                                                subband=conf.getInt('hba-test-sb',155),
                                                noise_low_deviation=conf.getFloat('ehba-noise-min-deviation', -3.0),
                                                noise_high_deviation=conf.getFloat('ehba-noise-max-deviation', 2.5),
                                                noise_max_diff=conf.getFloat('ehba-noise-max-difference', 1.5),
                                                rf_min_signal=conf.getFloat('ehba-rf-min-signal', 70.0),
                                                rf_low_deviation=conf.getFloat('ehba-rf-min-deviation', -24.0),
                                                rf_high_deviation=conf.getFloat('ehba-rf-max-deviation', 12.0))
                            
                        # one run done
                        repeat_cnt += 1
                        runtime = time.time() - runstart
                        
                
                    resetRSPsettings()
                    #turnoffRCUs()
        
        # do TBB tests if requested
        if tbb_check == True:
            # wait for TBB boards ready
            if waitTBBready(nTBB) == 1:
                if args.has_key('TV'):
                    db.addTestDone('TV')
                    tbb.checkVersions(conf.getStr('tbbdriver-version'), conf.getStr('tbbctl-version'),
                                      conf.getStr('tp-version'), conf.getStr('mp-version'))
                if args.has_key('TM'):
                    db.addTestDone('TM')
                    tbb.checkMemory()
    
    db.check_stop_time = time.gmtime()

    # do db test and write result files to log directory
    log_dir = conf.getStr('log-dir-local')
    if os.path.exists(log_dir):
        logger.info("write result data")
        db.test(log_dir)
    else:
        logger.warn("not a valid log directory")

    swlevel(start_level)
    writeMessage('!!!     The test is ready and the station can be used again!               !!!')

    # delete files from data directory
    removeAllDataFiles()


if __name__ == '__main__':
    main()
