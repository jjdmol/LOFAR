#!/usr/bin/python

info = """    ----------------------------------------------------------------------------
    Usage of arguments
    -l=2              : set level to 2 (default level is 0)
                      level 0    : manual checks, use keys listed below
                      level 1..n : see checkhardware.conf file for checks done
    
    To start a long check set number of runs or start and stop time, if start time
    is not given the first run is started immediately
    -r=1              : repeats, number of runs to do
    -start=[date_time]: start time of first run, format [YYYYMMDD_HH:MM:SS]
    -stop=[date_time] : stop time of last run, format [YYYYMMDD_HH:MM:SS]
    
    Set logging level, can be: debug|info|warning|error
    -ls=debug         : print all information on screen, default=info
    -lf=info          : print debug|warning|error information to log file, default=debug
    
    Select checks to do, can be combined with all levels
    -s(rcumode)       : signal check for rcumode
    -o(rcumode)       : oscillation check for rcumode
    -sp(rcumode)      : spurious check for rcumode
    -n(rcumode)[=120] : noise check for rcumode, optional data time in seconds
                        default data time = 300 sec for hba and 180 sec for lba
    -e(rcumode)[=120] : do all HBA element tests, optional data time in seconds
                        default data time = 10 sec
    -sn(rcumode)      : HBA summator noise check, always in rcumode 5
    -m(rcumode)       : HBA modem check, automatic selected if other hba check are selected
    
    -rcu(mode)        : do all rcu(mode) tests
    
    -rv               : RSP version check, always done
    -tv               : TBB version check, always done
    -spu              : RSP spu voltage check
    -tm               : TBB memmory check
    
    example   : ./checkHardware.py -s5 -n5=180
    ----------------------------------------------------------------------------"""


import os
import sys

check_version = '0415'

mainPath = r'/opt/stationtest'
libPath  = os.path.join(mainPath, 'lib')
sys.path.insert(0, libPath)

logPath  = r'/localhome/stationtest/log'

import time
import datetime
import logging

from general_lib import *
from lofar_lib import *
from test_lib import *
from test_db import *
from data_lib import *
from search_lib import search_version

os.umask(001)

logger = None

rcu_keys      = ('RCU1','RCU2','RCU3','RCU4','RCU5','RCU6','RCU7')
rcu_m12_keys  = ('LBL','O1','SP1','N1','S1','O2','SP2','N2','S2')
rcu_m34_keys  = ('LBH','O3','SP3','N3','S3','O4','SP4','N4','S4')
rcu_m567_keys = ('HBA','M','O5','SN5','SP5','N5','S5','E5','O6','SN6','SP6','N6','S6','E6','O7','SN7','SP7','N7','S7','E7')
rsp_keys     = ('RV','SPU') + rcu_keys + rcu_m12_keys + rcu_m34_keys + rcu_m567_keys
tbb_keys     = ('TV','TM')
control_keys = ('R','START','STOP')
all_keys     = control_keys + rsp_keys + tbb_keys
rsp_check    = False
modem_check  = False
tbb_check    = False

args = dict()
# next checks are always done
args["RV"] = '-'
args["TV"] = '-'
args["SPU"] = '-'


def printHelp():
    print info

# return readable info for test
def getTestInfo(key=''):
    if key[-1] in '1234567':
        test = key[:-1]
        mode = key[-1]
        if mode in '1234':
            if test in 'O':
                return("LBA mode-%c Oscillation test" %(mode))
            if test in 'SP':
                return("LBA mode-%c Spurious test" %(mode))
            if test in 'N':
                return("LBA mode-%c Noise test" %(mode))
            if test in 'S':
                return("LBA mode-%c RF/Down test" %(mode))    
        if mode in '567':
            if test in 'O':
                return("HBA mode-%c Oscillation test" %(mode))
            if test in 'S':
                return("HBA mode-%c RF test" %(mode))
            if test in 'N':
                return("HBA mode-%c Noise test" %(mode))
            if test in 'S':
                return("HBA mode-%c RF test" %(mode))
            if test in 'SN':
                return("HBA mode-%c Summator noise test" %(mode))
            if test in 'E':
                return("HBA mode-%c Element test" %(mode))
            if test in 'M':
                return("HBA mode-%c Modem test" %(mode))    
    if key in 'RV':
        return("RSP Version test")
    if key in 'SPU':
        return("RSP SPU test")    
    if key in 'TV':
        return("TBB Version test")
    if key in 'TM':
        return("TBB Memory test")
    if key in 'START':
        return("START checks")
    if key in 'STOP':
        return("STOP checks")    
    if key in 'R':
        return("Number of test repeats set to")
    return("")    
        
def addToArgs(key, value):
    if key == '':
        return
    global args, rsp_check, modem_check, tbb_check
    if key in rsp_keys or key in tbb_keys or key in ('H','L','LS','LF','R','START','STOP'):
        if value != '-':
            args[key] = value
        else:
            if key in rcu_keys:
                mode = key[-1]
                args['O%c' %(mode)] = '-'
                args['SP%c' %(mode)] = '-'
                args['N%c' %(mode)] = '-'
                args['S%c' %(mode)] = '-'
                if mode in '567':
                    modem_check = True
                    args['SN%c' %(mode)] = '-'
            elif key == 'LBL':
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
                args['SN5'] = '-'
                args['SP5'] = '-'
                args['N5'] = '-'
                if 'S7' not in args:
                    args['S5'] = '-'
            elif key == 'ES7':
                args['E7'] = '-'
            elif key == 'EHBA':
                if 'E7' not in args:
                    args['E5'] = '-'
            else:    
                args[key] = '-'
        
        if key in rcu_m567_keys:
            modem_check = True
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
    #global args
    
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
        screen_log_level = args.get('LS', 'WARNING')
        file_log_level   = args.get('LF', 'DEBUG')
    except:
        print "Not a legal log level, try again"
        sys.exit(-1)

    station = getHostName()

    # create logger
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)
    
    # check if log dir exist  
    if not os.access(logPath, os.F_OK):
        os.mkdir(logPath)
    
    # create file handler
    full_filename = os.path.join(logPath, 'checkHardware.log')
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

def backupLogFiles():
    for nr in range(8,-1,-1):
        if nr == 0:
            full_filename = os.path.join(logPath, 'checkHardware.log')
        else:
            full_filename = os.path.join(logPath, 'checkHardware.log.%d' %(nr))
        full_filename_new = os.path.join(logPath, 'checkHardware.log.%d' %(nr+1))
        if os.path.exists(full_filename):
            os.rename(full_filename, full_filename_new)
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
    getArguments()
    #print args
    if len(args) == 0 or args.has_key('H'):
        printHelp()
        sys.exit()

    backupLogFiles() # backup logfiles, max 10 logfiles .9 is the oldest    
    
    init_logging()
    init_lofar_lib()
    init_test_db()
    init_test_lib()
    init_data_lib()

    conf = cConfiguration()

    setLevelTests(conf)

    StID = getHostName()

    logger.info('== START HARDWARE CHECK ==')
    logger.info("== requested checks and settings ==")
    logger.info("-"*40)
    for i in all_keys:
        if args.has_key(i):
            if args.get(i) == '-':
                logger.info(" %s" %(getTestInfo(i)))
            else:
                logger.info(" %s, time = %s" %(getTestInfo(i), args.get(i)))
    logger.info("-"*40)
    
    removeAllDataFiles()

    # use format YYYYMMDD_HH:MM:SS
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
            if (time.mktime(start_datetime.timetuple()) < time.time()):
                #print time.mktime(start_datetime.timetuple()), time.time()
                logger.error("Stop program, StartTime in past")
                sys.exit(2)
            if(time.mktime(start_datetime.timetuple()) > stop_time):
                logger.error("Stop program, stop before start")
                sys.exit(2)    
            waitForStart(start_datetime)
        
        logger.info("run checks till %s" %(time.asctime(stop_datetime.timetuple())))
    
    start_time = time.gmtime()
    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA, HBA_SPLIT = readStationConfig()

    # setup intern database with station layout
    db = cDB(StID, nRSP, nTBB, nLBL, nLBH, nHBA, HBA_SPLIT)
    
    
    if (stop_time > 0.0):
        db.setTestEndTime((stop_time-120.0))
        
                    
    # set manualy marked bad antennas
    log_dir = conf.getStr('log-dir-global')
    host = getHostName()
    if os.path.exists(log_dir):
        logger.info("add bad_antenna_list data to db")
        f = open(os.path.join(log_dir, "bad_antenna_list.txt"), 'r')
        data = f.readlines()
        for line in data:
            if line[0] == '#':
                continue
            ant_list = line.strip().split(' ')
            if ant_list[0].strip().upper() == host.upper():
                if len(ant_list) > 1:
                    for ant in ant_list[1:]:
                        ant_type = ant[:3].strip().upper()
                        if ant_type == 'LBA':
                            ant_nr = int(ant[3:].strip())
                            #print "ant type=%s nr=%d" %(ant_type, ant_nr)
                            if ant_nr < nLBH:
                                db.lbh.ant[ant_nr].on_bad_list = 1
                            else:
                                db.lbl.ant[ant_nr-nLBH].on_bad_list = 1
                        elif ant_type == 'HBA':
                            ant_nr = int(ant[3:].strip())
                            #print "ant type=%s nr=%d" %(ant_type, ant_nr)
                            db.hba.tile[ant_nr].on_bad_list = 1
                break
    
    #
    db.script_versions = 'CHECK=%s,DB=%s,TEST=%s,SEARCH=%s,LOFAR=%s,GENERAL=%s' %\
                         (check_version, db_version, test_version, search_version, lofar_version, general_version)
    db.check_start_time = time.gmtime()
    
    writeMessage('!!!  This station will be in use for a test! Please do not use the station!  (script version %s)  !!!' %(check_version))
    start_level, board_errors = swlevel()
    sw_level, board_errors = swlevel(2)
    if start_level == 1:
        logger.info("Wait 30 seconds while startup RSPDriver")
        time.sleep(30.0)
    if sw_level == 2:
        tbb = cTBB(db)
        rsp = cRSP(db)
        spu = cSPU(db)

        # do RSP tests if requested
        if rsp_check == True:
            # check if RSPDriver is running
            if checkActiveRSPDriver() == False:
                logger.warn("RSPDriver not running")
            else:
                # wait for RSP boards ready, and reset 48V if needed, max 2x if no board errors after 48V reset
                rsp_ready = False
                restarts = 2
                while (not rsp_ready) and (restarts > 0):
                    if waitRSPready() == False:
                        logger.warn("Not all RSP boards ready, reset 48V to recover")
                        swlevel(1)
                        reset48V()
                        restarts -= 1
                        time.sleep(30.0)
                        level, board_errors = swlevel(2)
                        if len(board_errors) > 0:
                            db.board_errors = board_errors
                            restarts = 0
                        else:
                            time.sleep(30.0)
                    else:
                        rsp_ready = True
                restarts = 2
                
                # if all rsp boards ready do all tests        
                if rsp_ready:
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
                    while (repeat_cnt <= repeats) or ((stop_time > -1) and ((time.time() + runtime) < stop_time)):
                        
                        try:
                            runstart = time.time()
                            if stop_time > -1:
                                logger.info("\n=== Start testrun %d ===\n" %(repeat_cnt))
                            else:
                                logger.info("\n=== Start testrun %d of %d ===\n" %(repeat_cnt, repeats))
                            
                            if args.has_key('SPU'):
                                spu.checkStatus()
                        
                            # do all rcumode 1 tests if available
                            if StID in CoreStations or StID in RemoteStations:
                                for mode in (1, 2):
                                    if args.has_key('O%d' %(mode)):
                                        lbl.checkOscillation(mode=mode)
                
                                    if args.has_key('SP%d' %(mode)):
                                        lbl.checkSpurious(mode=mode)
                
                                    if args.has_key('N%d' %(mode)):
                                        if args.get('N%d' %(mode)) == '-':
                                            recordtime = 180
                                        else:
                                            recordtime = int(args.get('N%d' %(mode)))
                                        lbl.checkNoise(mode=mode,
                                                       record_time=recordtime,
                                                       low_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                                       high_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                                       max_diff=conf.getFloat('lba-noise-max-difference', 2.0))
                
                                    if repeat_cnt == 1 and args.has_key('S%d' %(mode)):
                                       lbl.checkSignal(mode=mode,
                                                       subband=conf.getInt('lbl-test-sb',301),
                                                       min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                                       low_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                                       high_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))                        
                            
                            for mode in (3, 4):
                                # do all rcumode 3,4 tests        
                                if args.has_key('O%d' %(mode)):
                                    lbh.checkOscillation(mode=mode)
            
                                if args.has_key('SP%d' %(mode)):
                                    lbh.checkSpurious(mode=mode)
            
                                if args.has_key('N%d' %(mode)):
                                    if args.get('N%d' %(mode)) == '-':
                                        recordtime = 180
                                    else:
                                        recordtime = int(args.get('N%d' %(mode)))
                                    lbh.checkNoise(mode=mode,
                                                   record_time=recordtime,
                                                   low_deviation=conf.getFloat('lba-noise-min-deviation', -3.0),
                                                   high_deviation=conf.getFloat('lba-noise-max-deviation', 2.5),
                                                   max_diff=conf.getFloat('lba-noise-max-difference', 1.5))
                              
                                if repeat_cnt == 1 and args.has_key('S%d' %(mode)):
                                    lbh.checkSignal(mode=mode, 
                                                    subband=conf.getInt('lbh-test-sb',301),
                                                    min_signal=conf.getFloat('lba-rf-min-signal', 75.0),
                                                    low_deviation=conf.getFloat('lba-rf-min-deviation', -2.0),
                                                    high_deviation=conf.getFloat('lba-rf-max-deviation', 2.0))
        
                            for mode in (5, 6, 7):
                                # do all rcumode 5, 6, 7 tests    
                                # do always a modem check if an other hba check is requested
                                if modem_check:
                                    hba.checkModem(mode=mode)
                                    hba.turnOffBadTiles()
            
                                if args.has_key('O%d' %(mode)):
                                    hba.checkOscillation(mode=mode)
            
                                if args.has_key('SN%d' %(mode)):
                                    hba.checkSummatorNoise(mode=mode)
            
                                if args.has_key('SP%d' %(mode)):
                                    hba.checkSpurious(mode=mode)
            
                                if args.has_key('N%d' %(mode)):
                                    if args.get('N%d' %(mode)) == '-':
                                        recordtime = 300
                                    else:
                                        recordtime = int(args.get('N%d' %(mode)))
                                    hba.checkNoise(mode=mode,
                                                   record_time=recordtime,
                                                   low_deviation=conf.getFloat('hba-noise-min-deviation', -3.0),
                                                   high_deviation=conf.getFloat('hba-noise-max-deviation', 2.5),
                                                   max_diff=conf.getFloat('hba-noise-max-difference', 2.0))
            
            
                                if repeat_cnt == 1 and args.has_key('S%d' %(mode)):
                                    hba.checkSignal(mode=mode, subband=conf.getInt('hba-test-sb',155),
                                                            min_signal=conf.getFloat('hba-rf-min-signal', 80.0),
                                                            low_deviation=conf.getFloat('hba-rf-min-deviation', -24.0),
                                                            high_deviation=conf.getFloat('hba-rf-max-deviation', 12.0))
                                
                                runtime = (time.time() - runstart)
                                
                                # All element test
                                if args.has_key('E%d' %(mode)):
                                    if args.get('E%d' %(mode)) == '-':
                                        recordtime = 10
                                    else:
                                        recordtime = int(args.get('E%d' %(mode)))
                                    
                                    hba.checkElements(  mode=mode, 
                                                        record_time=recordtime,
                                                        subband=conf.getInt('hba-test-sb',155),
                                                        noise_low_deviation=conf.getFloat('ehba-noise-min-deviation', -3.0),
                                                        noise_high_deviation=conf.getFloat('ehba-noise-max-deviation', 2.5),
                                                        noise_max_diff=conf.getFloat('ehba-noise-max-difference', 1.5),
                                                        rf_min_signal=conf.getFloat('ehba-rf-min-signal', 70.0),
                                                        rf_low_deviation=conf.getFloat('ehba-rf-min-deviation', -24.0),
                                                        rf_high_deviation=conf.getFloat('ehba-rf-max-deviation', 12.0),
                                                        skip_signal_test=args.has_key('ES7'))
                                
                            # stop test if driver stopped
                            db.rsp_driver_down = not checkActiveRSPDriver()
                            if db.rsp_driver_down and (restarts > 0):
                                restarts -= 1
                                reset48V()
                                time.sleep(30.0)
                                level, board_errors = swlevel(2)
                                if len(board_errors) > 0:
                                    db.board_errors = board_errors
                                    break
                                else:
                                    time.sleep(30.0)
                                
                            # one run done
                            repeat_cnt += 1
                            

                        except:
                            logger.error("Program fault, RSP test (%s)" %(sys.exc_value))
                            #raise
                            break
                            
                db.rsp_driver_down = not checkActiveRSPDriver()    
                if not db.rsp_driver_down:
                    resetRSPsettings()
        
        # do TBB tests if requested
        db.tbb_driver_down = not checkActiveTBBDriver()
        if (not db.tbb_driver_down) and (tbb_check == True):
            # wait for TBB boards ready
            if waitTBBready(nTBB) == 1:
                try:
                    if args.has_key('TV'):
                        db.addTestDone('TV')
                        tbb.checkVersions(conf.getStr('tbbdriver-version'), conf.getStr('tbbctl-version'),
                                          conf.getStr('tp-version'), conf.getStr('mp-version'))
                    if args.has_key('TM'):
                        db.addTestDone('TM')
                        tbb.checkMemory()
                except:
                    logger.error("Program fault, TBB test (%s)" %(sys.exc_value))
    db.check_stop_time = time.gmtime()

    # do db test and write result files to log directory
    log_dir = conf.getStr('log-dir-local')
    if os.path.exists(log_dir):
        logger.info("write result data")
        db.test(log_dir)
    else:
        logger.warn("not a valid log directory")
    if not db.rsp_driver_down:
        logger.info("Going back to swlevel %d" %(start_level))
        swlevel(start_level)
    logger.info("Test ready.")
    writeMessage('!!!     The test is ready and the station can be used again!               !!!')

    # delete files from data directory
    removeAllDataFiles()
    sys.exit(0)


if __name__ == '__main__':
    main()
