#!/usr/bin/python

check_version = '0714'

from threading import Thread
import sys
import traceback
import os
import numpy as np
import time
#import datetime
import logging

mainPath = r'/opt/stationtest'
mainDataPath = r'/localhome/stationtest'
observationsPath = r'/opt/lofar/var/run'

beamletPath = r'/localhome/data/Beamlets'

libPath  = os.path.join(mainPath, 'lib')
sys.path.insert(0, libPath)

confPath = os.path.join(mainDataPath, 'config')
logPath  = os.path.join(mainDataPath, 'log')
rtsmPath = os.path.join(mainDataPath, 'rtsm_data')

from general_lib import *
from lofar_lib import *
from search_lib import *
from data_lib import *

os.umask(001)
os.nice(15)

# make path if not exists
if not os.access(logPath, os.F_OK):
    os.mkdir(logPath)
if not os.access(rtsmPath, os.F_OK):
    os.mkdir(rtsmPath)

logger = None

def lbaMode(mode):
    if mode in (1, 2, 3, 4):
        return (True)
    return (False)

def lbaLowMode(mode):
    if mode in (1, 2):
        return (True)
    return (False)

def lbaHighMode(mode):
    if mode in (3, 4):
        return (True)
    return (False)

def hbaMode(mode):
    if mode in (5, 6, 7):
        return (True)
    return (False)

def checkStr(key):
    checks = dict({'OSC':"Oscillation", 'HN':"High-noise", 'LN':"Low-noise", 'J':"Jitter", 'SN':"Summator-noise",\
                  'CR':"Cable-reflection", 'M':"Modem-failure", 'DOWN':"Antenna-fallen", 'SHIFT':"Shifted-band"})
    return (checks.get(key, 'Unknown'))

def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
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

            if key in ('H','LS','LF'):
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
        full_filename = os.path.join(confPath, 'checkHardware.conf')
        f = open(full_filename, 'r')
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
        file_log_level   = args.get('LF', 'DEBUG')
    except:
        print "Not a legal log level, try again"
        sys.exit(-1)

    station = getHostName()

    # create logger
    _logger = logging.getLogger()
    _logger.setLevel(logging.DEBUG)

    # create file handler
    filename = '%s_rtsm.log' %(getHostName())
    full_filename = os.path.join(logPath, filename)
    file_handler = logging.FileHandler(full_filename, mode='w')
    formatter = logging.Formatter('%(asctime)s %(levelname)-8s %(message)s')
    file_handler.setFormatter(formatter)
    file_handler.setLevel(log_levels[file_log_level])
    _logger.addHandler(file_handler)

    if (len(_logger.handlers) == 1) and ('LS' in args):
        # create console handler
        stream_handler = logging.StreamHandler()
        fmt = '%s %%(levelname)-8s %%(message)s' %(station)
        formatter = logging.Formatter(fmt)
        stream_handler.setFormatter(formatter)
        stream_handler.setLevel(log_levels[screen_log_level])
        _logger.addHandler(stream_handler)
    return (_logger)

def getRcuMode(n_rcus):
# RCU[ 0].control=0x10337a9c =>  ON, mode:3, delay=28, att=06
    rcumode = -1
    rcu_info = {}
    answer = rspctl("--rcu")
    if answer.count('mode:') == n_rcus:
        for line in answer.splitlines():
            if line.find('mode:') == -1:
                continue
            rcu   = line[line.find('[')+1 : line.find(']')].strip()
            state = line[line.find('=>')+2 : line.find(',')].strip()
            mode  = line[line.find('mode:')+5]
            if rcu.isdigit() and state in ("OFF", "ON") and mode.isdigit():
                rcu_info[int(rcu)] = (state, int(mode))

        for mode in range(8):
            mode_cnt = answer.count("mode:%d" %(mode))
            if mode == 0:
                if mode_cnt == n_rcus:
                    logger.debug("Not observing")
            elif mode_cnt > (n_rcus / 3) and answer.count("mode:0") == (n_rcus - mode_cnt):
                logger.debug("Now observing in rcumode %d" %(mode))
                rcumode = mode
    return (rcumode, rcu_info)

def getAntPol(rcumode, rcu):
    pol_str = ('X','Y')
    ant = rcu / 2
    if rcumode == 1:
        pol_str = ('Y','X')
        ant += 48
    pol = pol_str[rcu % 2]
    return (ant, pol)

class CSV:
    station  = ""
    obs_id   = ""
    filename = ""
    rcu_mode = 0
    record_timestamp = 0
    @staticmethod
    def setObsID(obs_id):
        CSV.station  = getHostName()
        CSV.obs_id   = obs_id
        CSV.filename = "%s_%s_open.dat" %(CSV.station, CSV.obs_id)
        CSV.rcu_mode = 0
        CSV.rec_timestamp = 0
        CSV.writeHeader()
        return
    @staticmethod
    def setRcuMode(rcumode):
        CSV.rcu_mode = rcumode
        return
    @staticmethod
    def setRecordTimestamp(timestamp):
        CSV.record_timestamp = timestamp
        return
    @staticmethod
    def writeHeader():
        full_filename = os.path.join(rtsmPath, CSV.filename)
        # write only if new file
        if not os.path.exists(full_filename):
            f = open(full_filename, 'w')
            f.write('# SPECTRA-INFO=rcu,rcumode,obs-id,check,startfreq,stopfreq,rec-timestamp\n')
            f.write('#\n')
            f.flush()
            f.close()
        return
    @staticmethod
    def writeSpectra(data, rcu, check):
        #dumpTime = time.gmtime(CSV.record_timestamp)
        #date_str = time.strftime("%Y%m%d", dumpTime)

        full_filename = os.path.join(rtsmPath, CSV.filename)

        logger.debug("start dumping data to %s" %(full_filename))

        f = open(full_filename, 'a')

        if   CSV.rcu_mode in (1, 2, 3, 4):
            freq = (0  , 100)
        elif CSV.rcu_mode in (5,):
            freq = (100, 200)
        elif CSV.rcu_mode in (6,):
            freq = (160, 240)
        elif CSV.rcu_mode in (7,):
            freq = (200, 300)

        spectra_info = "SPECTRA-INFO=%d,%d,%s,%s,%d,%d,%f\n" %\
                       (rcu, CSV.rcu_mode, CSV.obs_id, check, freq[0], freq[1], CSV.record_timestamp)

        mean_spectra = "MEAN-SPECTRA=["
        for i in np.nan_to_num(data.getMeanSpectra(rcu%2)):
            mean_spectra += "%3.1f " %(i)
        mean_spectra += "]\n"

        bad_spectra = "BAD-SPECTRA=["
        for i in np.nan_to_num(data.getSpectra(rcu)):
            bad_spectra += "%3.1f " %(i)
        bad_spectra += "]\n\n"

        f.write(spectra_info)
        f.write(mean_spectra)
        f.write(bad_spectra)

        f.close()
        return
    @staticmethod
    def writeInfo(start_time, stop_time, obsid_samples):
        full_filename = os.path.join(rtsmPath, CSV.filename)
        logger.debug("add obs_info to %s" %(full_filename))
        f = open(full_filename, 'a')
        f.write('# OBS-ID-INFO=obs_id,start_time,stop_time,obsid_samples\n')
        f.write('OBS-ID-INFO=%s,%5.3f,%5.3f,%d\n\n' %(CSV.obs_id, start_time, stop_time, obsid_samples))
        f.flush()
        f.close()
        return
    @staticmethod
    def closeFile():
        full_filename = os.path.join(rtsmPath, CSV.filename)
        filename_new = CSV.filename.replace('open','closed')
        full_filename_new = os.path.join(rtsmPath, filename_new)
        logger.debug("rename file from %s to %s" %(full_filename, full_filename_new))
        os.rename(full_filename, full_filename_new)
        CSV.obs_id = ""
        CSV.filename = ""
        return

def checkForOscillation(data, rcumode, error_list, delta):
    logger.debug("start oscillation check")
    for pol_nr, pol in enumerate(('X', 'Y')):
        #test_data = data.getAll()[:,:1,:]
        result = search_oscillation(data, pol, delta)

        if len(result) > 1:
            # get mean values from all rcu's (rcu = -1)
            bin_nr, ref_max_sum, ref_n_peaks, ref_rcu_low = result[0]

            #rcu, max_sum, n_peaks, rcu_low = sorted(result[1:], reverse=True)[0]
            if len(result) == 2:
                bin_nr, max_sum, n_peaks, rcu_low = result[1]
            else:
                ref_low = result[0][3]
                max_low_rcu = (-1, -1)
                max_sum_rcu = (-1, -1)
                for i in result[1:]:
                    bin_nr, max_sum, n_peaks, rcu_low = i
                    if max_sum > max_sum_rcu[0]: max_sum_rcu = (max_sum, bin_nr)
                    if (rcu_low - ref_low) > max_low_rcu[0]: max_low_rcu = (rcu_low, bin_nr)

                rcu_low, bin_nr = max_low_rcu

            rcu = (bin_nr * 2) + pol_nr
            ant, pol = getAntPol(rcumode, rcu)

            if lbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Ant-%03d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) (=ref)" %\
                           (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low))
                if rcu not in error_list:
                    error_list.append(rcu)
                    CSV.writeSpectra(data, rcu, "OSC")

            if hbaMode(rcumode):
                if ((max_sum > 5000.0) or (n_peaks > 40)):
                    logger.info("Mode-%d RCU-%03d Tile-%02d %c Oscillation, sum=%3.1f(%3.1f) peaks=%d(%d) low=%3.1fdB(%3.1f) ref=()" %\
                               (rcumode, rcu, ant, pol, max_sum, ref_max_sum, n_peaks, ref_n_peaks, rcu_low, ref_rcu_low))
                    if rcu not in error_list:
                        error_list.append(rcu)
                        CSV.writeSpectra(data, rcu, "OSC")
    return

def checkForNoise(data, rcumode, error_list, low_deviation, high_deviation, max_diff):
    logger.debug("start noise check")
    for pol_nr, pol in enumerate(('X', 'Y')):
        low_noise, high_noise, jitter = search_noise(data, pol, low_deviation, high_deviation*1.5, max_diff)

        for err in high_noise:
            bin_nr, val, bad_secs, ref, diff = err
            rcu = (bin_nr * 2) + pol_nr
            ant, pol = getAntPol(rcumode, rcu)
            if lbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Ant-%03d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                           (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                if rcu not in error_list:
                    error_list.append(rcu)
                    CSV.writeSpectra(data, rcu, "HN")

            if hbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Tile-%02d %c High-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                           (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                if rcu not in error_list:
                    error_list.append(rcu)
                    CSV.writeSpectra(data, rcu, "HN")

        for err in low_noise:
            bin_nr, val, bad_secs, ref, diff = err
            rcu = (bin_nr * 2) + pol_nr
            ant, pol = getAntPol(rcumode, rcu)
            if lbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Ant-%03d %c Low-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                           (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                if rcu not in error_list:
                    error_list.append(rcu)
                    CSV.writeSpectra(data, rcu, "LN")

            if hbaMode(rcumode):
                logger.info("Mode-%d RCU-%03d Tile-%02d %c Low-noise, value=%3.1fdB bad=%d(%d) limit=%3.1fdB diff=%3.1fdB" %\
                           (rcumode, rcu, ant, pol, val, bad_secs, data.frames, ref, diff))
                if rcu not in error_list:
                    error_list.append(rcu)
                    CSV.writeSpectra(data, rcu, "LN")
    return

def checkForSummatorNoise(data, rcumode, error_list):
    logger.debug("start summator-noise check")
    for pol_nr, pol in enumerate(('X', 'Y')):
        # sn=SummatorNoise  cr=CableReflections
        sn, cr = search_summator_noise(data=data, pol=pol, min_peak=2.0)
        for msg in sn:
            bin_nr, peaks, max_peaks = msg
            rcu = (bin_nr * 2) + pol_nr
            tile, pol = getAntPol(rcumode, rcu)
            logger.info("Mode-%d RCU-%03d Tile-%02d %c Summator-noise, cnt=%d peaks=%d" %\
                       (rcumode, rcu, tile, pol, peaks, max_peaks))
            if rcu not in error_list:
                error_list.append(rcu)
                CSV.writeSpectra(data, rcu, "SN")
        for msg in cr:
            bin_nr, peaks, max_peaks = msg
            rcu = (bin_nr * 2) + pol_nr
            tile, pol = getAntPol(rcumode, rcu)
            logger.info("Mode-%d RCU-%03d Tile-%02d %c Cable-reflections, cnt=%d peaks=%d" %\
                       (rcumode, rcu, tile, pol, peaks, max_peaks))
            #if rcu not in error_list:
                #error_list.append(rcu)
                #CSV.writeSpectra(data, rcu, "CR")
    return

def checkForDown(data, rcumode, error_list, subband):
    logger.debug("start down check")
    down, shifted = searchDown(data, subband)
    for msg in down:
        ant, max_x_sb, max_y_sb, mean_max_sb = msg
        rcu = ant * 2
        max_x_offset = max_x_sb - mean_max_sb
        max_y_offset = max_y_sb - mean_max_sb
        ant, pol = getAntPol(rcumode, rcu)
        logger.info("Mode-%d RCU-%02d/%02d Ant-%02d Down, x-offset=%d y-offset=%d" %\
                   (rcumode, rcu, (rcu+1), ant, max_x_offset, max_y_offset))
        if rcu not in error_list:
            error_list.append(rcu)
            error_list.append(rcu+1)
            CSV.writeSpectra(data, rcu, "DOWN")
            CSV.writeSpectra(data, rcu+1, "DOWN")
    return

def checkForFlat(data, rcumode, error_list):
    logger.debug("start flat check")
    flat = searchFlat(data)
    for msg in flat:
        rcu, mean_val = msg
        ant, pol = getAntPol(rcumode, rcu)
        logger.info("Mode-%d RCU-%02d Ant-%02d Flat, value=%5.1fdB" %\
                   (rcumode, rcu, ant, mean_val))
        if rcu not in error_list:
            error_list.append(rcu)
            CSV.writeSpectra(data, rcu, "FLAT")
    return

def checkForShort(data, rcumode, error_list):
    logger.debug("start short check")
    short = searchShort(data)
    for msg in short:
        rcu, mean_val = msg
        ant, pol = getAntPol(rcumode, rcu)
        logger.info("Mode-%d RCU-%02d Ant-%02d Short, value=%5.1fdB" %\
                   (rcumode, rcu, ant, mean_val))
        if rcu not in error_list:
            error_list.append(rcu)
            CSV.writeSpectra(data, rcu, "SHORT")
    return

def closeAllOpenFiles():
    files = os.listdir(rtsmPath)
    for filename in files:
        if filename.find('open') > -1:
            full_filename = os.path.join(rtsmPath, filename)
            filename_new = filename.replace('open','closed')
            full_filename_new = os.path.join(rtsmPath, filename_new)
            os.rename(full_filename, full_filename_new)
    return

class cDayInfo:
    def __init__(self):
        self.date = time.strftime("%Y%m%d", time.gmtime(time.time()))
        self.filename = "%s_%s_dayinfo.dat" %(getHostName(), self.date)
        self.samples = [0,0,0,0,0,0,0] # RCU-mode 1..7
        self.obs_info = list()
        self.deleteOldDays()
        self.readFile()

    def addSample(self, rcumode=-1):
        date = time.strftime("%Y%m%d", time.gmtime(time.time()))
        # new day reset data and set new filename
        if self.date != date:
            self.date = date
            self.reset()
        if rcumode in range(1,8,1):
            self.samples[rcumode-1] += 1
            self.writeFile()

    def addObsInfo(self, obs_id, start_time, stop_time, rcu_mode, samples):
        self.obs_info.append([obs_id, start_time, stop_time, rcu_mode, samples])

    def reset(self):
        self.filename = "%s_%s_dayinfo.dat" %(getHostName(), self.date)
        self.samples = [0,0,0,0,0,0,0] # RCU-mode 1..7
        self.obs_info = list()
        self.deleteOldDays()

    # after a restart, earlier data is imported
    def readFile(self):
        full_filename = os.path.join(rtsmPath, self.filename)
        if os.path.exists(full_filename):
            f = open(full_filename, 'r')
            lines = f.readlines()
            f.close()
            for line in lines:
                if len(line.strip()) == 0 or line.strip()[0] == '#':
                    continue
                key,data  = line.split('=')
                if key == 'DAY-INFO':
                    self.samples = [int(i) for i in data.split(',')[1:]]
                if key == 'OBSID-INFO':
                    d = data.split(',')
                    self.obs_info.append([d[0],float(d[1]),float(d[2]),int(d[3]), int(d[4])])

    # rewrite file every sample
    def writeFile(self):
        full_filename = os.path.join(rtsmPath, self.filename)
        f = open(full_filename, 'w')
        f.write('#DAY-INFO date,M1,M2,M3,M4,M5,M6,M7\n')
        f.write('DAY-INFO=%s,%d,%d,%d,%d,%d,%d,%d\n' %\
               (self.date, self.samples[0], self.samples[1], self.samples[2], self.samples[3], self.samples[4], self.samples[5], self.samples[6]))
        f.write('\n#OBS-ID-INFO obs_id, start_time, stop_time, rcu_mode, samples\n')
        for i in self.obs_info:
            f.write('OBS-ID-INFO=%s,%5.3f,%5.3f,%d,%d\n' %\
                   (i[0],i[1],i[2],i[3],i[4]))
        f.close()

    def deleteOldDays(self):
        files = os.listdir(rtsmPath)
        backup = True
        for filename in files:
            if filename.find('closed') != -1:
                backup = False
        if backup == True:
            for filename in files:
                if filename.find('dayinfo') != -1:
                    if filename.split('.')[0].split('_')[1] != self.date:
                        full_filename = os.path.join(rtsmPath, filename)
                        os.remove(full_filename)

        
def getObsId():
    #obs_start_str  = ""
    #obs_stop_str   = ""
    #obs_start_time = 0.0
    #obs_stop_time  = 0.0
    obsids = ""
    answer = sendCmd('swlevel')
    if answer.find("ObsID") > -1:
        s1 = answer.find("ObsID:")+6
        s2 = answer.find("]")
        obsids = answer[s1:s2].strip().split()
    return (obsids)

    
def getObsIdInfo(obsid):
    filename = "Observation%s" %(obsid.strip())
    fullfilename = os.path.join(observationsPath, filename)
    f = open(fullfilename, 'r')
    obsinfo = f.read()
    f.close()
    m1 = obsinfo.find("Observation.startTime")
    m2 = obsinfo.find("\n", m1)
    obs_start_str  = obsinfo[m1:m2].split("=")[1].strip()
    obs_start_time = time.mktime(time.strptime(obs_start_str, "%Y-%m-%d %H:%M:%S"))
    
    m1 = obsinfo.find("Observation.stopTime",m2)
    m2 = obsinfo.find("\n", m1)
    obs_stop_str  = obsinfo[m1:m2].split("=")[1].strip()
    obs_stop_time = time.mktime(time.strptime(obs_stop_str, "%Y-%m-%d %H:%M:%S"))
    
    logger.debug("obsid %s  %s .. %s" %(obsid, obs_start_str, obs_stop_str))
    return(obsid, obs_start_time, obs_stop_time)

class RecordBeamletStatistics(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.running = False
        self.reset()
    
    def reset(self):
        self.dump_dir = ''
        self.obsid = ''
        self.duration = 0
            
    def set_obsid(self, obsid):
        self.dump_dir = os.path.join(beamletPath, obsid)
        try:
            os.mkdir(self.dump_dir)
        except:
            pass
        self.obsid = obsid
    
    def set_duration(self, duration):
        self.duration = duration
    
    def is_running(self):
        return self.running
    
    def kill_recording(self):
        if self.running:
            logger.debug("kill recording beamlet statistics")
            sendCmd(cmd='pkill', args='rspctl')
            logger.debug("recording killed")
            #self.running = False
            #self.make_plots()
    
    def make_plots(self):
        if self.obsid:
            try:
                response = sendCmd(cmd='/home/fallows/inspect_bsts.bash', args=self.obsid)
                logger.debug('response "inspect.bsts.bash" = {%s}' % response)
            except:
                logger.debug('exception while running "inspect.bsts.bash"')
        self.reset()
        
    def run(self):
        if self.duration:
            self.running = True
            logger.debug("start recording beamlet statistics for %d seconds" % self.duration)
            rspctl('--statistics=beamlet --duration=%d --integration=1 --directory=%s' % (self.duration, self.dump_dir))
            logger.debug("recording done")
            self.make_plots()
            self.running = False
    
def main():
    global logger
    obs_id   = ""
    active_obs_id  = ""
    rcumode  = 0
    #station  = getHostName()
    DI       = cDayInfo()

    args = getArguments()
    if args.has_key('H'):
        printHelp()
        sys.exit()

    logger = init_logging(args)
    init_lofar_lib()
    init_data_lib()

    conf = cConfiguration()

    #StID = getHostName()

    logger.info('== Start rtsm (Real Time Station Monitor) ==')

    removeAllDataFiles()

    # Read in RemoteStation.conf
    ID, nRSP, nTBB, nLBL, nLBH, nHBA, HBA_SPLIT = readStationConfig()

    n_rcus = nRSP * 8

    data = cRCUdata(n_rcus)

    obs_start_time = 0
    obs_stop_time  = 0
    obsid_samples  = 0
    
    beamlet_recording = RecordBeamletStatistics()
     
    
    while True:
        try:
            # get active obsid from swlevel
            obsids = getObsId()
            
            time_now = time.time()
            # stop if no more obsids or observation is stoped
            if obs_stop_time > 0.0: 
                if active_obs_id not in obsids or len(obsids) == 0 or time_now > obs_stop_time:
                    logger.debug("save obs_id %s" %(obs_id))
                    DI.addObsInfo(obs_id, obs_start_time, obs_stop_time, rcumode, obsid_samples)
                    DI.writeFile()
                    CSV.writeInfo(obs_start_time, obs_stop_time, obsid_samples)
                    CSV.closeFile()
                    active_obs_id  = ""
                    obs_start_time = 0.0
                    obs_stop_time  = 0.0
                    # if still running kill recording
                    if beamlet_recording:
                        if beamlet_recording.is_running():
                            beamlet_recording.kill_recording()
                        beamlet_recording = 0
            
            # if no active observation get obs info if obsid available
            if active_obs_id == "":
                # if still running kill recording
                if beamlet_recording:
                    if beamlet_recording.is_running():
                        beamlet_recording.kill_recording()
                    beamlet_recording = 0
                
                for id in obsids:
                    obsid, start, stop = getObsIdInfo(id)
                    if time_now >= (start - 60.0) and (time_now + 15) < stop:
                        active_obs_id  = obsid
                        obs_start_time = start
                        obs_stop_time  = stop
                        break
            
            if time_now < obs_start_time:
                logger.debug("waiting %d seconds for start of observation" %(int(obs_start_time - time_now)))
                time.sleep((obs_start_time - time_now) + 1.0)
                    
            # start recording beamlets
            if not beamlet_recording:
                if obs_start_time > 0.0 and time.time() >= obs_start_time:
                    duration = obs_stop_time - time.time() - 10
                    if duration > 2:
                        beamlet_recording = RecordBeamletStatistics()
                        beamlet_recording.set_obsid(active_obs_id)
                        beamlet_recording.set_duration(duration)
                        beamlet_recording.start()
            
            check_start = time.time()
            # if new obs_id save data and reset settings
            if obs_id != active_obs_id:
                # start new file and set new obsid
                obs_id        = active_obs_id
                obsid_samples = 0
                CSV.setObsID(obs_id)

            # it takes about 11 seconds to record data, for safety use 15    
            if (time.time() + 15.0) < obs_stop_time:
                # observing, so check mode now
                rcumode, rcu_info = getRcuMode(n_rcus)
                if rcumode <= 0:
                    continue

                active_rcus = []
                for rcu in rcu_info:
                    state, mode = rcu_info[rcu]
                    if state == 'ON':
                        active_rcus.append(rcu)
                data.setActiveRcus(active_rcus)

                rec_timestamp  = time.time()+3.0
                data.record(rec_time=1, read=True, slow=True)
                #data.fetch()

                CSV.setRcuMode(rcumode)
                CSV.setRecordTimestamp(rec_timestamp)
                DI.addSample(rcumode)
                obsid_samples += 1
                logger.debug("do tests")
                mask = extractSelectStr(conf.getStr('mask-rcumode-%d' %(rcumode)))
                data.setMask(mask)
                if len(mask) > 0:
                    logger.debug("mask=%s" %(str(mask)))

                error_list = []
                # do LBA tests
                if lbaMode(rcumode):
                    checkForDown(data, rcumode, error_list,
                                  conf.getInt('lbh-test-sb',301))
                    checkForShort(data, rcumode, error_list)
                    checkForFlat(data, rcumode, error_list)
                    checkForOscillation(data, rcumode, error_list, 6.0)
                    checkForNoise(data, rcumode, error_list,
                                  conf.getFloat('lba-noise-min-deviation', -3.0),
                                  conf.getFloat('lba-noise-max-deviation', 2.5),
                                  conf.getFloat('lba-noise-max-difference', 1.5))
                # do HBA tests
                if hbaMode(rcumode):
                    checkForOscillation(data, rcumode, error_list, 9.0)
                    checkForSummatorNoise(data, rcumode, error_list)
                    checkForNoise(data, rcumode, error_list,
                                  conf.getFloat('hba-noise-min-deviation', -3.0),
                                  conf.getFloat('hba-noise-max-deviation', 2.5),
                                  conf.getFloat('hba-noise-max-difference', 2.0))
            else:
                
                closeAllOpenFiles()
            
            if active_obs_id == "":
                # if not observing check every 30 seconds for observation start
                sleeptime = 30.0
                logger.debug("no observation, sleep %1.0f seconds" %(sleeptime))
            else:
                # if observing do check every 60 seconds
                check_stop = time.time()
                sleeptime = 60.0 - (check_stop - check_start)
                logger.debug("sleep %1.0f seconds till next check" %(sleeptime))
            while sleeptime > 0.0:
                wait = min(1.0, sleeptime)
                sleeptime -= wait
                time.sleep(wait)

        except KeyboardInterrupt:
            logger.info("stopped by user")
            sys.exit()
        except:
            logger.error('Caught %s', str(sys.exc_info()[0]))
            logger.error(str(sys.exc_info()[1]))
            logger.error('TRACEBACK:\n%s', traceback.format_exc())
            logger.error('Aborting NOW')
            sys.exit(0)
            

    # do test and write result files to log directory
    log_dir = conf.getStr('log-dir-local')
    if os.path.exists(log_dir):
        logger.info("write result data")
        # write result
    else:
        logger.warn("not a valid log directory")
    logger.info("Test ready.")

    # if still running kill recording
    if beamlet_recording:
        if beamlet_recording.is_running():
            beamlet_recording.kill_recording()
        beamlet_recording = 0

    # delete files from data directory
    removeAllDataFiles()
    sys.exit(0)

if __name__ == '__main__':
    main()
