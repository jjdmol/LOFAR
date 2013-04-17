#!/usr/bin/python

versiondate = '4 april 2013'

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
    print "-start=[date_time]: start time of first run, format [yyyymmdd_hh:mm:ss]"
    print "-stop=[date_time] : stop time of last run, format [yyyymmdd_hh:mm:ss]"
    print
    print "Set logging system, default is info, can be: info|debug|warning|error"
    print "-ls=debug         : print debug information on screen"
    print "-lf=debug         : print debug information to log file"
    print
    print "Select checks to do, can be combined with all levels"
    print "-s(rcumode)       : signal check for rcumode (1|3|5)"
    print "-o(rcumode)       : oscillation check for rcumode (1|3|5)"
    print "-sp(rcumode)      : spurious check for rcumode (1|3|5)"
    print "-n(rcumode)[=120] : noise check for rcumode (1|3|5), optional data time in seconds"
    print "                    default data time = 300 sec for hba and 180 sec for lba"
    print "-es               : HBA signal check in rcumode 5, element based"
    print "-en[=120]         : HBA noise check in rcumode 5, element based, optional data time in seconds"
    print "                    default data time = 60 sec"
    print "-eo               : HBA oscillation check in rcumode 5, element based"
    print "-esp              : HBA spurious check in rcumode 5, element based"

    print "-m                : HBA modem check, automatic selected if other hba check are selected"
    print "-sn               : HBA summator noise check"
    print
    print "-lbl              : do all LBL tests"
    print "-lbh              : do all LBH tests"
    print "-hba              : do all HBA tests"
    print "-ehba             : do all HBA element tests"
    print
    print "-rv               : RSP version check"
    print "-tv               : TBB version check"
    print "-tm               : TBB memmory check"
    print
    print "example   : ./checkHardware.py -s5 -n5=180"


    print "----------------------------------------------------------------------------"

rcu_m1_keys  = ('LBL','O1','SP1','N1','S1')
rcu_m3_keys  = ('LBH','O3','SP3','N3','S3')
rcu_m5_keys  = ('HBA','M','O5','SN','SP5','N5','S5','EHBA','EO','ESP','EN','ES')
rsp_keys     = ('RV',) + rcu_m1_keys + rcu_m3_keys + rcu_m5_keys
tbb_keys     = ('TV','TM')
rsp_check    = False
rcu_m5_check = False
tbb_check    = False

args = dict()
# get command line arguments
def getArguments():
    global args, rsp_keys, rsp_check, rcu_m5_keys, rcu_m5_check, tbb_keys, tbb_check
    
    for arg in sys.argv[1:]:
        if arg[0] == '-':
            opt = arg[1:].upper()
            valpos = opt.find('=')
            if valpos != -1:
                key, value = opt.strip().split('=')
                args[key] = value
            else:
                key = opt
                if opt == 'LBL':
                    args['O1'] = '-'
                    args['SP1'] = '-'
                    args['N1'] = '-'
                    args['S1'] = '-'
                elif opt == 'LBH':
                    args['O3'] = '-'
                    args['SP3'] = '-'
                    args['N3'] = '-'
                    args['S3'] = '-'
                elif opt == 'HBA':
                    args['O5'] = '-'
                    args['SN'] = '-'
                    args['SP5'] = '-'
                    args['N5'] = '-'
                    args['S5'] = '-'
                elif opt == 'EHBA':
                    args['SN'] = '-'
                    args['EO'] = '-'
                    args['ESP'] = '-'
                    args['EN'] = '-'
                    args['ES'] = '-'
                else:    
                    args[key] = '-'
        if key in rsp_keys:
            rsp_check = True
        if key in rcu_m5_keys:
            rcu_m5_check = True    
        if key in tbb_keys:
            tbb_check = True
                        
# get checklevel and set tests to do
def setLevelTests(conf):
    global args, rsp_keys, rsp_check, tbb_keys, tbb_check
    
    level = args.get('L', '0')
    tests = conf.getStr('level-%s-tests' %(level)).split(',')
    for tst in tests:
        key = tst.upper()
        args[key]='-'
        if key in rsp_keys:
            rsp_check = True
        if key in tbb_keys:
            tbb_check = True


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
        file_log_level   = args.get('LF', 'INFO')
    except:
        print "Not a legal log level, try again"
        sys.exit(-1)

    station = getHostName()

    # create logger
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    # create file handler
    full_filename = os.path.join(mainPath, 'checkHardware.log')
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

def main():
    global args
    global rsp_check, rcu_m5_check, tbb_check
    global logger

    init_logging()
    init_lofar_lib()
    init_test_db()
    init_test_lib()

    getArguments()
    #print args
    if len(args) == 0 or args.has_key('H'):
        printHelp()
        sys.exit()

    conf = cConfiguration()

    setLevelTests(conf)

    StID = getHostName()

    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()

    # setup intern database with station layout
    db = cDB(StID, nRSP, nTBB, nLBL, nLBH, nHBA)

    logger.info('== START HARDWARE CHECK ==')

    logger.info("used check level = %s" %(args.get('L', '0')))

    # make temporary data directory in tmp dir

    if os.access(dataDir(), os.F_OK):
        removeAllDataFiles()
    else:
        os.mkdir(dataDir())

    # use format YYYYMMDD_HH:MM:SS
    start_time = time.time()
    stop_time = -1
    if args.has_key('STOP'):
        if args.has_key('START'):
            start = args.get('START')
            if len(start) != 17:
                sys.exit("wrong starttime format must be YYYYMMDD_HH:MM:SS")
            start_datetime = datetime.datetime(int(start[:4]),int(start[4:6]),int(start[6:8]), \
                                               int(start[9:11]),int(start[12:14]),int(start[15:]))
            start_time = time.mktime(start_datetime.timetuple())
        stop = args.get('STOP')
        if len(stop) != 17:
                sys.exit("wrong stoptime format must be YYYYMMDD_HH:MM:SS")
        stop_datetime = datetime.datetime(int(stop[:4]),int(stop[4:6]),int(stop[6:8]), \
                                          int(stop[9:11]),int(stop[12:14]),int(stop[15:]))
        stop_time = time.mktime(stop_datetime.timetuple())

        sleep_time = start_time - time.time()
        if sleep_time > 0.0:
            logger.info("delayed start, sleep till %s" %(time.asctime(start_datetime.timetuple())))
            time.sleep(sleep_time)
        logger.info("run checks till %s" %(time.asctime(stop_datetime.timetuple())))

    db.check_start_time = time.gmtime()
    writeMessage('!!!  This station will be in use for a test! Please do not use the station!  (script version %s)  !!!' %(versiondate))
    start_level = swlevel()
    sw_level = swlevel(2)
    # if swlevel is 2
    if sw_level == 2:
        tbb = cTBB(db)
        rsp = cRSP(db)

        # do RSP tests if requested
        if rsp_check == True:
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

                # run noise test for "repeats" times
                # all other test only the first run
                runtime = 0
                while (repeat_cnt <= repeats) \
                      or ((stop_time > -1) and ((time.time() + runtime) < stop_time)):

                    runstart = time.time()
                    
                    db.tests = ''
                    logger.info("\n=== Start testrun %d of %d ===\n" %(repeat_cnt, repeats))
                    
                    if StID in CoreStations or StID in RemoteStations:
                        if repeat_cnt == 1 and args.has_key('S1'):
                                lbl.checkSignal(mode=1,
                                                subband=conf.getInt('lbl-test-sb',301),
                                                min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                                min_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                                max_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))
                        if args.has_key('O1'):
                            lbl.checkOscillation(mode=1)
    
                        if args.has_key('SP1'):
                            lbl.checkSpurious(mode=1)
    
                        if args.has_key('N1'):
                            if args.get('N1') == '-':
                                recordtime = 180
                            else:
                                recordtime = int(args.get('N1'))
                            if StID in CoreStations or StID in RemoteStations:
                                lbl.checkNoise(mode=1,
                                               record_time=recordtime,
                                               min_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                               max_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                               max_diff=conf.getFloat('lba-noise-max-difference', 2.0))
    
    
                    if repeat_cnt == 1 and args.has_key('S3'):
                        lbh.checkSignal(mode=3, 
                                        subband=conf.getInt('lbh-test-sb',301),
                                        min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                        min_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                        max_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))
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
                                       min_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                       max_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                       max_diff=conf.getFloat('lba-noise-max-difference', 1.5))


                    # do always a modem check if an other hba check is requested
                    if repeat_cnt == 1 and rcu_m5_check:
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
                                       min_deviation=conf.getFloat('hba-noise-min-deviation', -3.0),
                                       max_deviation=conf.getFloat('hba-noise-max-deviation', 2.5),
                                       max_diff=conf.getFloat('hba-noise-max-difference', 2.0))


                    if repeat_cnt == 1 and args.has_key('S5'):
                        hba.checkSignal(mode=5, subband=conf.getInt('hba-test-sb',155),
                                                min_signal=conf.getFloat('hba-rf-min-signal', 80.0),
                                                min_deviation=conf.getFloat('hba-rf-min-deviation', -24.0),
                                                max_deviation=conf.getFloat('hba-rf-max-deviation', 12.0))

                    if args.has_key('EO'):
                        hba.checkOscillationElements(mode=5)

                    if args.has_key('ESP'):
                        hba.checkSpuriousElements(mode=5)

                    if args.has_key('EN'):
                        if args.get('EN') == '-':
                            recordtime = 60
                        else:
                            recordtime = int(args.get('EN'))
                        hba.checkNoiseElements(mode=5,
                                               record_time=recordtime,
                                               min_deviation=conf.getFloat('noise-min-deviation', -3.0),
                                               max_deviation=conf.getFloat('noise-max-deviation', 2.5),
                                               max_diff=conf.getFloat('noise-max-difference', 1.5))

                    if repeat_cnt == 1 and args.has_key('ES'):
                        hba.checkSignalElements(mode=5, 
                                                subband=conf.getInt('hba-test-sb',155),
                                                min_signal=conf.getFloat('hba-rf-min-signal', 80.0),
                                                min_deviation=conf.getFloat('hba-rf-min-deviation', -24.0),
                                                max_deviation=conf.getFloat('hba-rf-max-deviation', 12.0))
                    
                    # one run done
                    repeat_cnt += 1
                    runtime = time.time() - runstart


                #turnoffRCUs()
        # do TBB tests if requested
        if tbb_check == True:
            # wait for TBB boards ready
            if waitTBBready(nTBB) == 1:
                if args.has_key('TV'):
                    db.tests += ',TV'
                    tbb.checkVersions(conf.getStr('tbbdriver-version'), conf.getStr('tbbctl-version'),
                                      conf.getStr('tp-version'), conf.getStr('mp-version'))
                if args.has_key('TM'):
                    db.tests += ',TM'
                    tbb.checkMemory()
    
    resetRSPsettings()
    db.check_stop_time = time.gmtime()

    # do db test and write result files to log directory
    log_dir = conf.getStr('log-dir-global')
    if not os.path.exists(log_dir):
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
