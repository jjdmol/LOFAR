#!/usr/bin/python
#!/usr/bin/python

check_version = '1113'

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
from search_lib import *
from data_lib import *

os.umask(001)

logger = None

lba_modes = (1, 2, 3, 4)
hba_modes = (5, 6, 7)
rcumode = 0
db = None

# PVSS states
State = dict({'OFF':0, 'OPERATIONAL':1, 'MAINTENANCE':2, 'TEST':3, 'SUSPICIOUS':4, 'BROKEN':5})

def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
    print "-u          : update pvss"
    print
    print "Set logging level, can be: debug|info|warning|error"
    print "-ls=debug   : print all information on screen, default=info"
    print "-lf=info    : print debug|warning|error information to log file, default=debug"
    print

    print "----------------------------------------------------------------------------"

control_keys = ('R','START','STOP')

args = dict()

def addToArgs(key, value):
    global args
    if key == '':
        return
    global args
    if key in ('H','LS','LF','U'):
        if value != '-':
            args[key] = value
        else:   
            args[key] = '-'
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
    global args

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
    full_filename = os.path.join(mainPath, 'realtime_check.log')
    #backup_filename = os.path.join(mainPath, 'checkHardware_bk.log')
    #sendCmd('cp', '%s %s' %(full_filename, backup_filename))
    file_handler = logging.FileHandler(full_filename, mode='a')
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

    
    
# send comment, key and value to PVSS and write to file
def sendToPVSS(comment, pvss_key, value):
    global logger, args

    if args.has_key('NO_UPDATE'):
            return("")
            
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
        sleep(0.2)
        return(response)
    return("")

class cCountDB:
    def __init__(self, n_rcus, n_tests, max_counts):
        self.max_counts = max_counts
        self.trig_count = max_counts / 2
        self.counts = max_counts
        self.n_rcus = n_rcus
        self.n_tests = n_tests
        self.bad_list = list()
        for mode in range(1,8,1):
            msg_list = list()
            for tst in range(self.n_tests):
                rcu_list = list()
                for rcu in range(self.n_rcus):
                    rcu_list.append(0)
                msg_list.append(rcu_list)
            self.bad_list.append(msg_list)
    
    def reset(self):
        self.counts = self.max_counts
        for mode in range(0,7,1):
            for tst in range(self.n_tests):
                for rcu in range(self.n_rcus):
                    self.bad_list[mode][tst][rcu] = 0
    
    def decCounts(self):
        self.counts -= 1
        return (self.counts)
    
    def incRcuCount(self, mode, test, rcu):
        self.bad_list[mode-1][test][rcu] += 1
        return
    
    def getCount(self, mode, test, rcu):
        return (self.bad_list[mode-1][test][rcu])
    
    def isTriggered(self, mode, test, rcu):
        if self.counts == 0:
            if self.bad_list[mode-1][test][rcu] > self.trig_count:
                return (True)
        return  (False)
    
def getRcuMode():
    global rcumode
    answer = rspctl("--rcu")
    for mode in range(8):
        if answer.count("mode:%d" %(mode)) > 48:
            if mode != rcumode:
                if mode == 0:
                    logger.debug("Not observing")
                else:
                    logger.debug("Now observing in rcumode %d" %(mode))
                rcumode = mode
                return (False)
            return (True)    
    
def getAntPol(rcu):
    global rcumode
    pol_str = ('X','Y')
    ant = rcu / 2
    if rcumode == 1:
        pol_str = ('Y','X')
        ant += 48
    pol = pol_str[rcu % 2]
    return (ant, pol)

def checkForOscillation(data, delta=9.0):
    global logger
    global args
    global rcumode
    global lba_modes
    global hba_modes
    global db
    
    logger.debug("start oscillation check")
    test_data = data.getAll()[:,:1,:]
    result = search_oscillation(test_data, delta, 0, 511)
    if len(result) > 1:
        # get mean values from all rcu's (rcu = -1)
        rcu, ref_max_sum, ref_n_peaks, ref_rcu_low = result[0]
        
        rcu, max_sum, n_peaks, rcu_low = sorted(result[1:], reverse=True)[0]
        ant, pol = getAntPol(rcu)
        
        
        if rcumode in lba_modes:
            db.incRcuCount(rcumode, 0, rcu)
            info = "Mode-%d RCU-%03d Ant-%03d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) ref=()" %\
                   (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low)
            
            if db.isTriggered(rcumode, 0, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("oscillating", "LOFAR_PIC_LBA%03d" %(ant), State['BROKEN'])
            else:
                logger.debug(info)
        
        if rcumode in hba_modes:
            if ((max_sum > 5000.0) or (n_peaks > 50)):
                db.incRcuCount(rcumode, 0, rcu)
            info = "Mode-%d RCU-%03d Tile-%02d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) ref=()" %\
                   (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low)
                   
            if  db.isTriggered(rcumode, 0, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("oscillating", "LOFAR_PIC_HBA%02d" %(ant), State['BROKEN'])
            else:
                logger.debug(info)
    return

    
def checkForNoise(data, low_deviation, high_deviation, max_diff):
    global logger
    global args
    global rcumode
    global lba_modes
    global hba_modes
    global db
    
    logger.debug("start noise/jitter check")
    test_data = data.getAll()
    # result is a sorted list on maxvalue
    low_noise, high_noise, jitter = search_noise(test_data, low_deviation, high_deviation, max_diff)
    
    n_err = len(high_noise)
    for err in high_noise:    
        rcu, val, bad_secs, ref, diff = err
        ant, pol = getAntPol(rcu)
        if rcumode in lba_modes:
            if (n_err < 12) and (bad_secs >= (data.frames / 2.0) and (diff >= 3.0)) or (diff >= 5.0):
                db.incRcuCount(rcumode, 1, rcu)
            info = "Mode-%d RCU-%03d Ant-%03d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                   (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff)
            if db.isTriggered(rcumode, 1, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("high-noise", "LOFAR_PIC_LBA%03d" %(ant+48), State['BROKEN'])
            else:
                logger.debug(info)
                
        if rcumode in hba_modes:
            if (n_err < 12) and (bad_secs >= (data.frames / 2.0) and (diff >= 3.0)) or (diff >= 5.0):
                db.incRcuCount(rcumode, 1, rcu)
            info = "Mode-%d RCU-%03d Tile-%02d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                   (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff)
            if db.isTriggered(rcumode, 1, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("high-noise", "LOFAR_PIC_HBA%02d" %(ant), State['BROKEN'])
            else:
                logger.debug(info)
    
    n_err = len(jitter)
    for err in jitter:
        rcu, val, ref, bad_secs = err
        ant, pol = getAntPol(rcu)
        if rcumode in lba_modes:
            if (n_err < 12) and (bad_secs >= (data.frames / 2.0) and (val >= 3.0)) or (val >= 5.0):
                db.incRcuCount(rcumode, 2, rcu)
            info = "Mode-%d RCU-%03d Ant-%03d %c Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %\
                   (rcumode, rcu, ant, pol, val, ref)
            if db.isTriggered(rcumode, 2, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("jitter", "LOFAR_PIC_LBA%03d" %(ant+48), State['BROKEN'])
            else:
                logger.debug(info)
                
        if rcumode in hba_modes:
            if (n_err < 12) and (bad_secs >= (data.frames / 2.0) and (val >= 3.0)) or (val >= 5.0):
                db.incRcuCount(rcumode, 2, rcu)
            info = "Mode-%d RCU-%03d Tile-%02d %c Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %\
                   (rcumode, rcu, ant, pol, val, ref)
            if db.isTriggered(rcumode, 2, rcu):
                logger.info(info)
                if args.has_key('U'):
                    sendToPVSS("jitter", "LOFAR_PIC_HBA%02d" %(ant), State['BROKEN'])
            else:
                logger.debug(info)
    return


def checkForSummatorNoise(data):
    global logger
    global rcumode
    global db

    logger.debug("start summator-noise check")
    
    result = search_summator_noise(data=data.getAll())
    for sn in result:
        rcu, val, bin = sn
        db.incRcuCount(rcumode, 3, rcu)
        if db.isTriggered(rcumode, 3, rcu):
            tile, pol = getAntPol(rcu)
            logger.info("Mode-%d RCU-%03d Tile-%02d %c Summator-noise, cnt=%d peaks=%d" %\
                       (rcumode, rcu, tile, pol, val, bin))
            if args.has_key('U'):
                sendToPVSS("summator-noise", "LOFAR_PIC_HBA%02d" %(tile), State['BROKEN'])           
    return

    
def main():
    global args
    global logger
    global State
    global rcumode
    global lba_modes
    global hba_modes
    global db
    
    getArguments()
    #print argsb
    if args.has_key('H'):
        printHelp()
        sys.exit()
        
    init_logging()
    init_lofar_lib()
    init_data_lib()

    conf = cConfiguration()

    StID = getHostName()

    logger.info('== Start realtime check ==')
    
    removeAllDataFiles()

    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()

    n_rcus = nRSP * 8
    n_tests = 4 # oscillation, noise, jitter, summator-noise
    n_max_counts = 4
    
    rcudata = cRCUdata(n_rcus)
    db = cCountDB(n_rcus, n_tests, n_max_counts) 
    
    while True:
        stage = 0
        try:
            start = time.time()
            answer = sendCmd('swlevel')
            stage = 1
            if answer.find("ObsID") > -1:
            #if True:    
                stage = 2
                # observing, so check mode now
                getRcuMode()
                if rcumode == 0:
                    continue
                stage = 3
                rcudata.record(rec_time=10)
                stage = 4
                # if rcumode not changed, getRcuMode() returns True
                if getRcuMode() == True:
                    stage = 5
                    if rcumode in lba_modes:
                        stage = 6
                        checkForOscillation(data=rcudata, delta=4.0)
                        stage = 7
                        checkForNoise(data           = rcudata,
                                      low_deviation  = conf.getFloat('lba-noise-min-deviation', -3.0),
                                      high_deviation = conf.getFloat('lba-noise-max-deviation', 2.5),
                                      max_diff       = conf.getFloat('lba-noise-max-difference', 1.5))
                        stage = 8
                    if rcumode in hba_modes:
                        stage = 9
                        checkForOscillation(data=rcudata, delta=6.0)
                        stage = 10
                        checkForNoise(data           = rcudata,
                                      low_deviation  = conf.getFloat('hba-noise-min-deviation', -3.0),
                                      high_deviation = conf.getFloat('hba-noise-max-deviation', 2.5),
                                      max_diff       = conf.getFloat('hba-noise-max-difference', 2.0))
                        stage = 11
                        checkForSummatorNoise(data=rcudata)
                        stage = 12
                    
                    logger.debug("counts=%d" %(db.counts))
                    if db.decCounts() == -1:
                        db.reset()
            stop = time.time()
            sleeptime = 30.0 - (stop - start)
            logger.debug("sleep %1.3f seconds" %(sleeptime))
            if sleeptime > 0.0:
                time.sleep(sleeptime) 
        except:
            logger.warn("program error in stage %d (%s)" %(stage, sys.exc_value))
        
    # do db test and write result files to log directory
    log_dir = conf.getStr('log-dir-local')
    if os.path.exists(log_dir):
        logger.info("write result data")
        # write result
    else:
        logger.warn("not a valid log directory")
    logger.info("Test ready.")

    # delete files from data directory
    removeAllDataFiles()
    sys.exit(0)

if __name__ == '__main__':
    main()
