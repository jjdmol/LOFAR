#!/usr/bin/python

check_version = '0314'

import sys
import os
import numpy as np

mainPath = r'/opt/stationtest'
libPath  = os.path.join(mainPath, 'lib')
sys.path.insert(0, libPath)

logPath  = r'/localhome/stationtest/log'
spectraPath  = r'/localhome/stationtest/bad_spectra'

        
import time
import datetime
import logging

from general_lib import *
from lofar_lib import *
from search_lib import *
from data_lib import *

os.umask(001)
os.nice(15)

if not os.access(logPath, os.F_OK):
    os.mkdir(logPath)

logger = None
stage  = ""

def lbaMode(mode):
    if mode in (1, 2, 3, 4):
        return (True)
    return (False)    

def hbaMode(mode):
    if mode in (5, 6, 7):
        return (True)
    return (False)    
    
def checkStr(key):
    checks = dict({'OSC':"Oscillation", 'HN':"High-noise", 'LN':"Low-noise", 'J':"Jitter", 'SN':"Summator-noise",\
                  'CR':"Cable-reflection", 'M':"Modem-failure", 'DOWN':"Antenna-fallen", 'SHIFT':"Shifted-band"})
    return (checks.get(key,'Unknown'))

# PVSS states
PVSS_states = dict({'OFF':0, 'OPERATIONAL':1, 'MAINTENANCE':2, 'TEST':3, 'SUSPICIOUS':4, 'BROKEN':5})

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


def getArguments():
    args = dict()
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
                
            if key in ('H','LS','LF','U'):
                if value != '-':
                    args[key] = value
                else:   
                    args[key] = '-'
            else:
                sys.exit("Unknown key %s" %(key))    
    return (args)
                       

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
def init_logging(args):
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
    filename = '%s_rtsm.log' %(getHostName())
    full_filename = os.path.join(logPath, filename)
    file_handler = logging.FileHandler(full_filename, mode='a')
    formatter = logging.Formatter('%(asctime)s %(levelname)-8s %(message)s')
    file_handler.setFormatter(formatter)
    file_handler.setLevel(log_levels[file_log_level])
    logger.addHandler(file_handler)

    if (len(logger.handlers) == 1) and ('LS' in args):
        # create console handler
        stream_handler = logging.StreamHandler()
        fmt = '%s %%(levelname)-8s %%(message)s' %(station)
        formatter = logging.Formatter(fmt)
        stream_handler.setFormatter(formatter)
        stream_handler.setLevel(log_levels[screen_log_level])
        logger.addHandler(stream_handler)
    return (logger)
    
# send comment, key and value to PVSS and write to file
def sendToPVSS(comment, pvss_key, value):
    if len(comment) > 0:
        comment = 'rtsm::'+comment
    else:
        comment = 'rtsm'
    arguments = '%s %s %d' %(comment, pvss_key, value)
    if True:
        print arguments
    else:
        response = sendCmd('setObjectState', arguments)
        sleep(0.2)
        return(response)
    return("")

def getRcuMode():
    global stage
    stage = "getRcuMode"
    rcumode = -1
    answer = rspctl("--rcu")
    for mode in range(8):
        mode_cnt = answer.count("mode:%d" %(mode))
        if mode == 0:
            if mode_cnt == 96:
                logger.debug("Not observing")
                return (rcumode)
        elif (mode_cnt > 48) and answer.count("mode:0") == (96 - mode_cnt):
            logger.debug("Now observing in rcumode %d" %(mode))
            rcumode = mode
    return (rcumode)    

def getAntPol(rcumode, rcu):
    global stage
    last_stage = stage
    stage = "getAntPol"
    
    pol_str = ('X','Y')
    ant = rcu / 2
    if rcumode == 1:
        pol_str = ('Y','X')
        ant += 48
    pol = pol_str[rcu % 2]
    stage = last_stage
    return (ant, pol)

    
def dumpSpectra(data, metadata, rcu, check):
    global stage
    last_stage = stage
    stage = "dumpSpectra"
    
    (station, rcumode, rec_timestamp, obs_id) = metadata
    logger.debug("start dumping data")
    dumpTime = time.gmtime(rec_timestamp)
    date_str = time.strftime("%Y%m%d", dumpTime)
    
    filename = "%s_%s_open.dat" %(station, obs_id)
    full_filename = os.path.join(spectraPath, filename) 
    f = open(full_filename, 'a')
    
    if rcumode in (1, 2, 3, 4):
        freq = (0  , 100)
    elif rcumode in (5,):
        freq = (100, 200)
    elif rcumode in (6,):
        freq = (160, 240)
    elif rcumode in (7,):
        freq = (200, 300)
    
    spectra_info = "SPECTRA-INFO=%d,%d,%s,%s,%d,%d,%f\n" %\
                   (rcu, rcumode, obs_id, check, freq[0], freq[1], rec_timestamp)
                   
    mean_spectra = "MEAN-SPECTRA=["
    for i in data.getMeanSpectra(rcu%2):
        mean_spectra += "%3.1f " %(i)
    mean_spectra += "]\n"    

    bad_spectra = "BAD-SPECTRA=["
    for i in data.getSpectra(rcu):
        bad_spectra += "%3.1f " %(i)
    bad_spectra += "]\n\n"    
    
    f.write(spectra_info)
    f.write(mean_spectra)
    f.write(bad_spectra)
    
    f.close()
    stage = last_stage
    return
    
    
def checkForOscillation(data, metadata, delta, pvss=False):
    global stage
    stage = "checkForOscillation"
    
    (station, rcumode, rec_timestamp, obs_id) = metadata
    logger.debug("start oscillation check")
    check_pol   = 0
    for test_data in (data.getAllX(), data.getAllY()):
        #test_data = data.getAll()[:,:1,:]
        result = search_oscillation(test_data, delta, 0, 511)
        
        
        if len(result) > 1:
            # get mean values from all rcu's (rcu = -1)
            bin, ref_max_sum, ref_n_peaks, ref_rcu_low = result[0]
            
            #rcu, max_sum, n_peaks, rcu_low = sorted(result[1:], reverse=True)[0]
            if len(result) == 2:
                bin, max_sum, n_peaks, rcu_low = result[1]
            else:
                ref_low = result[0][3]
                max_low_rcu = (-1, -1)
                max_sum_rcu = (-1, -1)
                for i in result[1:]:
                    bin, max_sum, n_peaks, rcu_low = i
                    if max_sum > max_sum_rcu[0]: max_sum_rcu = (max_sum, bin) 
                    if (rcu_low - ref_low) > max_low_rcu[0]: max_low_rcu = (rcu_low, bin)
                
                rcu_low, bin = max_low_rcu    
            
            rcu = (bin * 2) + check_pol
            ant, pol = getAntPol(rcumode, rcu)
            
            if lbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Ant-%03d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) (=ref)" %\
                           (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low))
                dumpSpectra(data, metadata, rcu, "OSC")
                if pvss:
                    sendToPVSS("rtsm oscillating", "LOFAR_PIC_LBA%03d" %(ant), PVSS_states['BROKEN'])
            
            if hbaMode(rcumode):
                if ((max_sum > 5000.0) or (n_peaks > 50)):
                    logger.info("Mode-%d RCU-%03d Tile-%02d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) ref=()" %\
                               (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low))
                    dumpSpectra(data, metadata, rcu, "OSC")
                    if pvss:
                        sendToPVSS("rtsm oscillating", "LOFAR_PIC_HBA%02d" %(ant), PVSS_states['BROKEN'])
            check_pol = 1
    return

    
def checkForNoise(data, metadata, low_deviation, high_deviation, max_diff, pvss=False):
    global stage
    stage = "checkForNoise"
    
    (station, rcumode, rec_timestamp, obs_id) = metadata
    logger.debug("start noise and jitter check")
    check_pol = 0
    for test_data in (data.getAllX(), data.getAllY()):
        #test_data = data.getAll()
        # result is a sorted list on maxvalue
        low_noise, high_noise, jitter = search_noise(test_data, low_deviation, high_deviation, max_diff)
        
        n_err = len(high_noise)
        for err in high_noise:    
            bin, val, bad_secs, ref, diff = err
            rcu = (bin * 2) + check_pol
            ant, pol = getAntPol(rcumode, rcu)
            if lbaMode(rcumode):
                if (n_err < 6) and (bad_secs >= (data.frames / 2.0) and (diff >= 3.0)) or (diff >= 5.0):
                    logger.info("Mode-%d RCU-%03d Ant-%03d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                               (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                    dumpSpectra(data, metadata, rcu, "HN")
                    if pvss:
                        sendToPVSS("rtsm high-noise", "LOFAR_PIC_LBA%03d" %(ant+48), PVSS_states['BROKEN'])
                    
            if hbaMode(rcumode):
                if (n_err < 6) and (bad_secs >= (data.frames / 2.0) and (diff >= 3.0)) or (diff >= 5.0):
                    logger.info("Mode-%d RCU-%03d Tile-%02d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                               (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                    dumpSpectra(data, metadata, rcu, "HN")
                    if pvss:
                        sendToPVSS("rtsm high-noise", "LOFAR_PIC_HBA%02d" %(ant), PVSS_states['BROKEN'])
        
        n_err = len(jitter)
        for err in jitter:
            bin, val, ref, bad_secs = err
            rcu = (bin * 2) + check_pol
            ant, pol = getAntPol(rcumode, rcu)
            if lbaMode(rcumode):
                if (n_err < 6) and (bad_secs >= (data.frames / 2.0) and (val >= 3.0)) or (val >= 5.0):
                    logger.info("Mode-%d RCU-%03d Ant-%03d %c Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %\
                               (rcumode, rcu, ant, pol, val, ref))
                    dumpSpectra(data, metadata, rcu, "J")
                    if pvss:
                        sendToPVSS("rtsm jitter", "LOFAR_PIC_LBA%03d" %(ant+48), PVSS_states['BROKEN'])
                    
            if hbaMode(rcumode):
                if (n_err < 6) and (bad_secs >= (data.frames / 2.0) and (val >= 3.0)) or (val >= 5.0):
                    logger.info("Mode-%d RCU-%03d Tile-%02d %c Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %\
                               (rcumode, rcu, ant, pol, val, ref))
                    dumpSpectra(data, metadata, rcu, "J")
                    if pvss:
                        sendToPVSS("rtsm jitter", "LOFAR_PIC_HBA%02d" %(ant), PVSS_states['BROKEN'])
        check_pol = 1            
    return


def checkForSummatorNoise(data, metadata, pvss=False):
    global stage
    stage = "checkForSummatorNoise"
    
    (station, rcumode, rec_timestamp, obs_id) = metadata
    logger.debug("start summator-noise check")
    check_pol = 0
    for test_data in (data.getAllX(), data.getAllY()):
        # sn=SummatorNoise  cr=CableReflections
        sn, cr = search_summator_noise(data=test_data, min_peak=2.0)
        for msg in sn:
            bin, peaks, max_peaks = msg
            rcu = (bin * 2) + check_pol
            tile, pol = getAntPol(rcumode, rcu)
            logger.info("Mode-%d RCU-%03d Tile-%02d %c Summator-noise, cnt=%d peaks=%d" %\
                       (rcumode, rcu, tile, pol, peaks, max_peaks))
            dumpSpectra(data, metadata, rcu, "SN")
            if pvss:
                sendToPVSS("rtsm summator-noise", "LOFAR_PIC_HBA%02d" %(tile), PVSS_states['BROKEN'])           
        for msg in cr:
            bin, peaks, max_peaks = msg
            rcu = (bin * 2) + check_pol
            tile, pol = getAntPol(rcumode, rcu)
            logger.info("Mode-%d RCU-%03d Tile-%02d %c Cable-reflections, cnt=%d peaks=%d" %\
                       (rcumode, rcu, tile, pol, peaks, max_peaks))
            #dumpSpectra(data, metadata, rcu, "CR")
        check_pol = 1
    return

def checkForDown(data, metadata, subband, pvss=False):
    global stage
    stage = "checkForDown"
    (station, rcumode, rec_timestamp, obs_id) = metadata
    logger.debug("start down check")
    _data = data.getAll()
    down, shifted = searchDown(_data, subband)
    for msg in down:
        ant, max_x_sb, max_y_sb, mean_max_sb = msg
        rcu = ant * 2
        max_x_offset = max_x_sb - mean_max_sb
        max_y_offset = max_y_sb - mean_max_sb
        ant, pol = getAntPol(rcumode, rcu)
        logger.info("Mode-%d RCU-%02d/%02d Ant-%02d Down, x-offset=%d y-offset=%d" %\
                   (rcumode, rcu, (rcu+1), ant, max_x_offset, max_y_offset))
        dumpSpectra(data, metadata, rcu, "DOWN")
        if pvss:
            sendToPVSS("rtsm fallen", "LOFAR_PIC_LBA%02d" %(ant), PVSS_states['BROKEN'])           
                
    for msg in shifted:
        rcu, max_sb, mean_max_sb = i
        offset = max_sb - mean_max_sb
        ant, pol = getAntPol(rcumode, rcu)
        logger.info("Mode-%d RCU-%02d Ant-%02d Shifted, offset=%d" %\
                   (rcumode, rcu, ant, offset))
        dumpSpectra(data, metadata, rcu, "SHIFT")
    return    

def full_listdir(dir_name):
     return sorted([os.path.join(dir_name, file_name) for file_name in os.listdir(dir_name)])
     
def getOpenFile():
    files = os.listdir(spectraPath)
    for file in files:
        if file.find("open") > 0:
            return(file, file.split('_')[1])
    return ('','')        

def closeOpenFiles(obsid=""):
    files = os.listdir(spectraPath)
    for filename in files:
        if obsid == "" or filename.find("_%s_open" %(obsid)) == -1:
            if filename.find('open') > -1: 
                full_filename = os.path.join(spectraPath, filename)
                filename_new = filename.replace('open','closed')
                full_filename_new = os.path.join(spectraPath, filename_new)
                os.rename(full_filename, full_filename_new)
    return

def closeFile(filename):
    full_filename = os.path.join(spectraPath, filename)
    filename_new = filename.replace('open','closed')
    full_filename_new = os.path.join(spectraPath, filename_new)
    os.rename(full_filename, full_filename_new)
    return
    
def main():
    global logger
    global State
    stage    = "main"
    filename = ""
    obs_id   = ""
    rcumode  = 0
    station  = getHostName()
    
    args = getArguments()
    if args.has_key('H'):
        printHelp()
        sys.exit()
        
    logger = init_logging(args)
    init_lofar_lib()
    init_data_lib()

    conf = cConfiguration()

    StID = getHostName()

    logger.info('== Start rtsm (Real Time Station Monitor) ==')
    
    removeAllDataFiles()

    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA, HBA_SPLIT = readStationConfig()

    n_rcus = nRSP * 8
    
    data = cRCUdata(n_rcus)
    
    start_time    = 0
    stop_time     = 0
    day_samples   = ["",0,0,0,0,0,0,0]
    obsid_samples = 0
    last_date     = ""
    now_date = last_date = time.strftime("%Y%m%d", time.gmtime(time.time()))
    
    logger.debug("first filename=%s, obsid=%s" %(filename, obs_id))
    while True:
        try:
            start = time.time()
            answer = sendCmd('swlevel')
            if answer.find("ObsID") > -1:
                s1 = answer.find("ObsID:")+6
                s2 = answer.find("]")
                id = answer[s1:s2].strip().replace(' ','-')
                if id != obs_id:
                    # close last file if exist
                    if obs_id != "":
                        stop_time = time.time()
                        full_filename = os.path.join(spectraPath, filename) 
                        f = open(full_filename, 'a')
                        f.write('OBS-ID-INFO=%s,%3.1f,%3.1f,%d\n\n' %(obs_id,start_time,stop_time,obsid_samples))
                        f.flush()
                        f.close()
                        #closeFile(filename)
                    
                    # set new obsid
                    obs_id       = id
                    obsid_samples = 0
                    start_time   = time.time() 
                    filename = "%s_%s_open.dat" %(getHostName(), obs_id)
                    full_filename = os.path.join(spectraPath, filename) 
                    if not os.path.exists(full_filename):
                        f = open(full_filename, 'a')
                        f.write('# SPECTRA-INFO=rcu,rcumode,obs-id,check,startfreq,stopfreq,rec-timestamp\n')
                        f.write('# OBS-ID-INFO=obs_id,start_time,stop_time,obsid_samples\n')
                        f.flush()
                        f.close()
                
                closeOpenFiles(obs_id)
                now_date = time.strftime("%Y%m%d", time.gmtime(time.time()))    
                if now_date != last_date:
                    filename = "%s_%s_dayinfo.dat" %(getHostName(), last_date)
                    full_filename = os.path.join(spectraPath, filename) 
                    f = open(full_filename, 'a')
                    f.write('#samples  date,M1,M2,M3,M4,M5,M6,M7\n')  
                    f.write('DAY-INFO=%s,%d,%d,%d,%d,%d,%d,%d\n' %\
                           (last_date, day_samples[1], day_samples[2], day_samples[3], day_samples[4], day_samples[5], day_samples[6], day_samples[7]))
                    f.close()
                    day_samples   = ["",0,0,0,0,0,0,0]
                    last_date = now_date
                # observing, so check mode now
                rcumode = getRcuMode()
                if rcumode == 0:
                    continue
                rec_timestamp  = time.time()+3.0
                data.record(rec_time=1, read=True, slow=True)
                # if rcumode not changed do tests
                if rcumode == getRcuMode():
                    day_samples[rcumode] += 1
                    obsid_samples        += 1
                    metadata = (station, rcumode, rec_timestamp, obs_id)
                    if lbaMode(rcumode):
                        checkForOscillation(data, metadata, 4.0)
                        #checkForNoise(data, metadata, conf.getFloat('lba-noise-min-deviation', -3.0),
                        #              conf.getFloat('lba-noise-max-deviation', 2.5),
                        #              conf.getFloat('lba-noise-max-difference', 1.5))
                    
                    if hbaMode(rcumode):
                        checkForOscillation(data, metadata, 6.0)
                        #checkForNoise(data, metadata, conf.getFloat('hba-noise-min-deviation', -3.0),
                        #              conf.getFloat('hba-noise-max-deviation', 2.5),
                        #              conf.getFloat('hba-noise-max-difference', 2.0))
                        checkForSummatorNoise(data, metadata)
                        
            else:
                if len(filename) > 0:
                    stop_time = time.time()
                    full_filename = os.path.join(spectraPath, filename) 
                    f = open(full_filename, 'a')
                    f.write('OBS-ID-INFO=%s,%3.1f,%3.1f,%d\n\n' %(obs_id,start_time,stop_time,obsid_samples))
                    f.flush()
                    f.close()
                    filename = ""
                    obs_id   = ""
                closeOpenFiles()
                    
            stop = time.time()
            sleeptime = 60.0 - (stop - start)
            logger.debug("sleep %1.3f seconds" %(sleeptime))
            if sleeptime > 0.0:
                time.sleep(sleeptime) 
        except KeyboardInterrupt:
            logger.info("stopped by user")
            sys.exit()
        except:
            logger.warn("program error(%s) in stage %s, rcudata shape = %s" %(sys.exc_value, stage, str(data.getAll().shape)))
        
    # do test and write result files to log directory
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
