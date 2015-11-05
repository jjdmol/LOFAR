#!/usr/bin/python
# test lib

from general_lib import *
from lofar_lib import *
import time
#from datetime import time
import sys
import os
import numpy as np


#HBASubband = dict( DE601C=155, DE602C=155, DE603C=284, DE604C=474, DE605C=479, FR606C=155, SE607C=287, UK608C=155 )
#DefaultLBASubband = 301
#DefaultHBASubband = 155

#datadir = r'/tmp/data'

# get and return recorded data in various ways
class cRCUdata:
    def __init__(self, n_rcu, minvalue=1):
        self.n_rcu = n_rcu
        self.frames = 0
        self.minvalue = minvalue
        self.ssData = np.ones((n_rcu, 1, 512), np.float64)
        self.testSignal_X = -1.0
        self.testSubband_X = 0
        self.testSignal_Y = -1.0
        self.testSubband_Y = 0
    
    def readFile(self, full_filename):
        data = np.fromfile(full_filename, dtype=np.float64)
        self.frames = len(data) / 512
        data = data.reshape(self.frames,512)
        return (data)
        
    def readFiles(self):
        files_in_dir = sorted(os.listdir(dataDir()))
        ssdata = np.array([self.readFile(os.path.join(dataDir(),file_name)) for file_name in files_in_dir])
        # mask zero values and convert to dBm
        self.ssData = np.log10(np.ma.masked_less(ssdata, self.minvalue)) * 10.0
    
    def getSubbands(self, rcu):
        return (self.ssData[int(rcu),:,:].max(0))
    
    def getSubbandX(self):
        return (self.ssData[0::2,:,self.testSubband_Y].max(1))
    
    def getSubbandY(self):
        return (self.ssData[1::2,:,self.testSubband_Y].max(1))
                       
    def getAll(self):
        return (self.ssData)
    
    def getAllX(self):
        return (self.ssData[0::2,:,:])
    
    def getAllY(self):
        return (self.ssData[1::2,:,:])
           
    def getMedianRcu(self, rcu):
        return(np.ma.median(self.ssData[int(rcu),:,:].max(0)))

    def searchTestSignal(self, subband=-1, minsignal=75.0, maxsignal=100.0):
        # ss = median for all band over all rcu's
        ssX = np.ma.median(self.ssData[::2,:,:].max(1),0)
        ssY = np.ma.median(self.ssData[1::2,:,:].max(1),0)
        
        if subband != -1:
            if ssX[subband] > minsignal and ssY[subband] > minsignal:
                self.testSignal_X = ssX[subband]
                self.testSubband_X = subband
                self.testSignal_Y = ssY[subband]
                self.testSubband_Y = subband
                return                
                
        # no subband given or not in requested range, look for better
        # forget subband 0    
        for i in range(1,512):
            if ssX[i] > minsignal  and ssX[i] < maxsignal and ssX[i] > self.testSignal_X:
                self.testSignal_X = ssX[i]
                self.testSubband_X = i
            if ssY[i] > minsignal  and ssY[i] < maxsignal and ssY[i] > self.testSignal_Y:
                self.testSignal_Y = ssY[i]
                self.testSubband_Y = i
        return 
#### end of cRCUdata class ####


# detect bad(oscillating) antennas
# 
def detectBadAntHighSignal(data, logger):
    m_spec           = data.max(axis=1)
    nominal_m_spec   = np.ma.median(m_spec, axis=0)
    ant_badness      = np.ma.median(m_spec/nominal_m_spec[np.newaxis,:], axis=1)

    max_deviation    = 4.0*ant_badness.std()
    bad_rcus         = abs(ant_badness - np.ma.median(ant_badness)) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 4.0*masked_badness.std()
    bad_rcus         = abs(ant_badness - masked_badness.mean()) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 3.0*masked_badness.std()
    bad_rcus         = abs(ant_badness - masked_badness.mean()) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 3.0*masked_badness.std()
    
    bad_rcus         = ant_badness - masked_badness.mean() >= max_deviation
    
    proc_bad = np.zeros(len(bad_rcus),'float')
    bad_val  = np.zeros(len(bad_rcus),'float')
    n_seconds = data.shape[1]
   
    #nominal_median = np.ma.median(nominal_m_spec)
    max_deviation = 2.75 * np.ma.std(np.ma.median(data, axis=2)[~bad_rcus,:])
    masked_median = np.ma.median(data[~bad_rcus,:,:])
    print masked_median
    for rcu in np.arange(len(bad_rcus))[bad_rcus]:
        for i in range(n_seconds):
            spec_median = np.ma.median(data[rcu,i,:])
            #print rcu, i, spec_median, masked_median, spec_median-masked_median, max_deviation
            if (spec_median - masked_median) >= max_deviation:
                proc_bad[rcu] += (100./n_seconds)
                if spec_median > bad_val[rcu]:
                    bad_val[rcu] = spec_median 
    #print np.arange(len(bad_rcus))[bad_rcus]
    #print proc_bad
    return np.arange(len(bad_rcus))[bad_rcus], proc_bad[bad_rcus], bad_val[bad_rcus], masked_median


def detectBadAntLowSignal(data, logger):
    m_spec           = data.min(axis=1)
    nominal_m_spec   = np.ma.median(m_spec, axis=0)
    ant_badness      = np.ma.median(m_spec/nominal_m_spec[np.newaxis,:], axis=1)

    max_deviation    = 4.0*ant_badness.std()
    bad_rcus         = abs(ant_badness - np.ma.median(ant_badness)) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 4.0*masked_badness.std()
    bad_rcus         = abs(ant_badness - masked_badness.mean()) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 3.0*masked_badness.std()
    bad_rcus         = abs(ant_badness - masked_badness.mean()) >= max_deviation

    masked_badness   = np.ma.array(ant_badness, mask=bad_rcus)
    max_deviation    = 3.0*masked_badness.std()
    
    bad_rcus         = ant_badness - masked_badness.mean() <= -max_deviation
   
    proc_bad = np.zeros(len(bad_rcus),'float')
    bad_val  = np.zeros(len(bad_rcus),'float')
    n_seconds = data.shape[1]
    #nominal_median = np.ma.median(nominal_m_spec)
    max_deviation = 2.75 * np.ma.std(np.ma.median(data, axis=2)[~bad_rcus,:])
    masked_median = np.ma.median(data[~bad_rcus,:,:])
    print masked_median
    for rcu in np.arange(len(bad_rcus))[bad_rcus]:
        for i in range(n_seconds):
            spec_median = np.ma.median(data[rcu,i,:])
            #print rcu, i, spec_median, masked_median, spec_median-masked_median, -max_deviation
            if (spec_median - masked_median) <= -max_deviation:
                proc_bad[rcu] += (100./n_seconds)
                if spec_median > bad_val[rcu]:
                    bad_val[rcu] = spec_median 
    #print np.arange(len(bad_rcus))[bad_rcus]
    #print proc_bad
    return np.arange(len(bad_rcus))[bad_rcus], proc_bad[bad_rcus], bad_val[bad_rcus], masked_median


# class for checking TBB boards using tbbctl
class cTBB:
    def __init__(self, db, logger):
        self.db = db
        self.log = logger
        self.nr = self.db.nr_tbb
        self.driverstate = True
        #tbbctl('--free')
    
    # wait until all boards have a working image loaded
    # returns 1 if ready or 0 if timed_out
    def waitReady(self):
        timeout = 90
        self.log.info("wait for working TBB boards ", screen=True)
        sys.stdout.flush()
        while timeout > 0:
            answer = tbbctl('--version')
            #print answer
            if answer.find('TBBDriver is NOT responding') > 0:
                time.sleep(5.0)
                timeout -= 5
                if timeout < 60:
                    return (0)
                continue
            # check if image_nr > 0 for all boards
            if answer.count('V') == (self.nr * 4):
            #if answer.count('V') == ((self.nr-1) * 4):
                self.log.info("All boards in working image")
                return (1)
            time.sleep(1.0)
            timeout -= 1
        return (0)
            
    # check software versions of driver, tbbctl and TP/MP firmware    
    def checkVersions(self, driverV, tbbctlV, tpV, mpV ):
        self.log.info("TBB Version check", screen=True)
        answer = tbbctl('--version')
        
        # check if Driver is available
        if answer.find('TBBDriver is NOT responding') > 0:
            self.log.info("No TBBDriver")
            self.driverstate = False
            self.db.tbbdriver_version = 0
        else:
            infolines = answer.splitlines()
            info = infolines[4:6] + infolines[9:-1]
            #print info
            if info[0].split()[-1] != driverV:
                self.log.info("Not right Driver version")
                self.db.tbbdriver_version = info[0].split()[-1]
            
            if info[1].split()[-1] != tbbctlV:
                self.log.info("Not right tbbctl version")
                self.db.tbbctl_version = info[1].split()[-1]
            
            # check if image_nr > 0 for all boards
            if str(info).count('V') != (self.nr * 4):
                self.log.info("WARNING, Not all boards in working image", screen=True)
            
            for tbb in self.db.tbb:
                board_info = info[2+tbb.nr].strip().split('  ')
                #print board_info
                if board_info[3].split()[1] != tpV:
                    self.log.info("Board %d Not right TP version" %(tbb.nr))
                    tbb.tp_version = board_info[3].split()[1]    

                if board_info[4].split()[1] != mpV:
                    self.log.info("Board %d Not right MP version" %(tbb.nr))
                    tbb.mp_version = board_info[4].split()[1]
        self.log.info("Done")
        self.log.printBusyTime(screen=True)
    
    # Check memory address and data lines            
    def checkMemory(self):
        self.log.info("TBB Memory check", screen=True)
        tbbctl('--free')
        for tbb in self.db.tbb:
            answer = tbbctl('--testddr=%d' %(tbb.nr))
            info = answer.splitlines()[-3:]
            ok = True
            if info[0].strip() != 'All Addresslines OK':
                self.log.info("Board %d Addresline error" %(tbb.nr))
                ok = False
            
            if info[1].strip() != 'All Datalines OK':
                self.log.info("Board %d Datalines error" %(tbb.nr))
                ok = False
            
            if not ok:
                tbb.memory_ok = 0
                self.log.info(answer)
        self.log.info("Done")
        self.log.printBusyTime(screen=True)
#### end of cTBB class ####




                
# class for checking RSP boards using rspctl
class cRSP:
    def __init__(self, db, logger):
        self.db = db
        self.log = logger
        self.nr = self.db.nr_rsp

    # wait until all boards have a working image loaded
    # returns 1 if ready or 0 if timed_out
    def waitReady(self):
        timeout = 60
        self.log.info("wait for working RSP boards ", screen=True)
        sys.stdout.flush()
        while timeout > 0:
            answer = rspctl('--version')
            #print answer
            if answer.count('No Response') > 0:
                time.sleep(5.0)
                timeout -= 5
                if timeout < 60:
                    return (0)
                continue
            # check if image_nr > 0 for all boards
            if answer.count('0.0') == 0:
                self.log.info("All boards in working image")
                return (1)
            time.sleep(1.0)
            timeout -= 1
        return (0)

    # function used for antenna testing        
    def resetSettings(self):
        if rspctl('--clock').find('200MHz') < 0:
            rspctl('--clock=200')
            self.log.info("Changed Clock to 200MHz", screen=True)
            time.sleep(2.0)
        rspctl('--wg=0', wait=0.0)
        rspctl('--rcuprsg=0', wait=0.0)
        rspctl('--datastream=0', wait=0.0)
        rspctl('--splitter=0', wait=0.0)
        rspctl('--specinv=0', wait=0.0)
        rspctl('--bitmode=16', wait=0.0)

    def turnonRCUs(self):
        rspctl('--rcuenable=1', wait=1.0)
        rspctl('--aweights=8000,0', wait=1.0)

    def turnoffRCUs(self):
        rspctl('--rcumode=0', wait=1.0)
        rspctl('--rcuenable=0', wait=1.0)
        rspctl('--aweights=0,0', wait=1.0)
    
    def swapXY(self, state):
        if state in (0,1):
            if state == 1:
                self.log.info("XY-output swapped", screen=True)
            else:
                self.log.info("XY-output normal", screen=True)
            rspctl('--swapxy=%d' %(state))
              
            
    # check software versions of driver, tbbctl and TP/MP firmware    
    def checkVersions(self, bpV, apV ):
        self.log.info("RSP Version check", screen=True)
        answer = rspctl('--version')

        # check if Driver is available
        if answer.find('No Response') > 0:
            self.log.info("No RSPDriver")
            self.db.rspdriver_version = 0
        else:
            infolines = answer.splitlines()
            info = infolines
            
            images_ok = True
            # check if image_nr > 0 for all boards
            if str(info).count('0.0') != 0:
                self.log.info("WARNING, Not all boards in working image", screen=True)
                images_ok = False
            
            for rsp in self.db.rsp:
                board_info = info[rsp.nr].split(',')
                
                if board_info[1].split()[3] != bpV:
                    self.log.info("Board %d Not right BP version" %(rsp.nr))
                    rsp.bp_version = board_info[1].split()[3]
                    images_ok = False
            
                if board_info[2].split()[3] != apV:
                    self.log.info("Board %d Not right AP version" %(rsp.nr))
                    rsp.ap_version = board_info[2].split()[3]
                    images_ok = False
        
        self.log.info("Done")
        self.log.printBusyTime(screen=True)
        return (images_ok)
#### end of cRSP class ####


# class for testing LBA antennas
class cLBA:
    # mode='lba_low' or 'lba_high'
    def __init__(self, lba, logger):
        self.lba = lba
        self.log = logger
            
        self.rcudata = cRCUdata(self.lba.nr_antennas*2)
        
        # Average normal value = 150.000.000 (81.76 dBm) -3dB +3dB
        # LOW/HIGH LIMIT is used for calculating mean value
        self.lowLimit = -3.0 #dB
        self.highLimit = 3.0 #dB
        
        # MEAN LIMIT is used to check if mean of all antennas is ok
        self.meanLimit = 66.0 #dB
        
        
    def checkNaS(self, mode, sampletime):
        self.lba.check_done = 1
        self.lba.check_time_noise = sampletime 
        self.log.info("Start %s Noise test" %(self.lba.label), screen=True)
        
        rspctl('--rcumode=%d' %(mode), wait=2.0)
        recordTime = sampletime
        removeAllDataFiles()
        self.log.info("Wait %d seconds while recording data" %(recordTime), screen=True)
        rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=0:%d' %(recordTime, dataDir(), (self.lba.nr_antennas*2)-1), wait=0.0)
        self.rcudata.readFiles()
        
        bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllX(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("%s %d X Low Noise, %3.1f%% bad, min.val %5.3f" %(self.lba.label, nr, proc[i], val[i]), screen=True)
            self.lba.ant[nr].x_low_noise = 1
            self.lba.ant[nr].x_low_proc  = proc[i]
            self.lba.ant[nr].x_low_val   = val[i]
            self.lba.ant[nr].x_low_ref   = ref
            
        
        bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllX(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("%s %d X High Noise, %3.1f%% bad, max.val %5.3f" %(self.lba.label, nr, proc[i], val[i]), screen=True)
            self.lba.ant[nr].x_high_noise = 1
            self.lba.ant[nr].x_high_proc  = proc[i]
            self.lba.ant[nr].x_high_val   = val[i]
            self.lba.ant[nr].x_high_ref   = ref
        
        bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllY(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("%s %d Y Low Noise, %3.1f%% bad, min.val %5.3f" %(self.lba.label, nr, proc[i], val[i]), screen=True)
            self.lba.ant[nr].y_low_noise = 1
            self.lba.ant[nr].y_low_proc  = proc[i]
            self.lba.ant[nr].y_low_val   = val[i]
            self.lba.ant[nr].y_low_ref   = ref
        
        bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllY(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("%s %d Y High Noise, %3.1f%% bad, max.val %5.3f" %(self.lba.label, nr, proc[i], val[i]), screen=True)
            self.lba.ant[nr].y_high_noise = 1
            self.lba.ant[nr].y_high_proc  = proc[i]
            self.lba.ant[nr].y_high_val   = val[i]
            self.lba.ant[nr].y_high_ref   = ref
        
        
    def checkSignal(self, mode, subband, min_deviation, max_deviation):
        self.lba.check_done = 1
        self.log.info("Start %s RF test" %(self.lba.label), screen=True)
              
        rspctl('--rcumode=%d' %(mode), wait=2.0)
        removeAllDataFiles()
        print rspctl('--statistics --duration=2 --integration=1 --directory=%s --select=0:%d' %(dataDir(), (self.lba.nr_antennas*2)-1), wait=0.0)
        self.rcudata.readFiles()
        self.rcudata.searchTestSignal(subband=subband, minsignal=75.0, maxsignal=90.0)
        
        self.log.info("For X used test subband=%d (%3.1f dB) in mode %d" %\
                     (self.rcudata.testSubband_X, self.rcudata.testSignal_X, mode), screen=True)
        self.log.info("For Y used test subband=%d (%3.1f dB) in mode %d" %\
                     (self.rcudata.testSubband_Y, self.rcudata.testSignal_Y, mode), screen=True)             
        
        if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_Y == 0:
            self.log.info("LBA mode %d, No test signal found" %(mode), screen=True)
            return
        
        ssdataX = self.rcudata.getSubbandX()
        ssdataY = self.rcudata.getSubbandY()
        #if np.ma.count(ssdataX) == 0 or np.ma.count(ssdataY) == 0:
            # all zeros (missing settings!!)
        #    return
            
        # use only values between lowLimit and highLimit for average calculations    
        dataInBandX = np.ma.masked_outside(ssdataX, (self.rcudata.testSignal_X + self.lowLimit), (self.rcudata.testSignal_X + self.highLimit))
        medianValX = np.ma.median(dataInBandX)
        
        dataInBandY = np.ma.masked_outside(ssdataY, (self.rcudata.testSignal_Y + self.lowLimit), (self.rcudata.testSignal_Y + self.highLimit))
        medianValY = np.ma.median(dataInBandY)
                
        self.log.info("used medianValX=%f" %(medianValX), screen=True)
        self.log.info("used medianValY=%f" %(medianValY), screen=True)
        if medianValX < self.meanLimit or medianValY < self.meanLimit:
            self.lba.avg_2_low = 1
            self.lba.avg_x = medianValX
            self.lba.avg_y = medianValY
        
        self.lba.test_signal_x = medianValX
        self.lba.test_signal_y = medianValY
        self.lba.test_subband_x = self.rcudata.testSubband_X
        self.lba.test_subband_y = self.rcudata.testSubband_Y

        for ant in self.lba.ant:
            ant.x_signal = ssdataX[ant.nr]
            ant.y_signal = ssdataY[ant.nr]
            
            loginfo = False
            if ssdataX[ant.nr] < (medianValX + min_deviation):
                ant.x_too_low = 1
                if ssdataX[ant.nr] < 2.0:
                    ant.x_rcu_error = 1
                loginfo = True
            
            if ssdataX[ant.nr] > (medianValX + max_deviation):
                ant.x_too_high = 1
                loginfo = True
            
            if ssdataY[ant.nr] < (medianValY + min_deviation):
                ant.y_too_low = 1
                if ssdataY[ant.nr] < 2.0:
                    ant.y_rcu_error = 1
                loginfo = True
            
            if ssdataY[ant.nr] > (medianValY + max_deviation):
                ant.y_too_high = 1
                loginfo = True
            
            if loginfo:
                self.log.info("%s %2d  RCU %3d/%3d   X=%5.1fdB  Y=%5.1fdB" %(self.lba.label, ant.nr, ant.x_rcu, ant.y_rcu, ssdataX[ant.nr], ssdataY[ant.nr]), screen=True)
                     
        # mark lba as down if top of band is lower than normal and top is shifted more than 10 subbands to left or right
        minWindow = -40
        maxWindow = 40
        for ant in self.lba.ant:
            rcudataX = self.rcudata.getSubbands(ant.x_rcu)
            rcudataY = self.rcudata.getSubbands(ant.y_rcu)
            
            maxSubbandOffsetX = -1
            maxSubbandValueX = 0
            maxSubbandOffsetY = -1
            maxSubbandValueY = 0
            
            # possible down if higher than (median - 2dB), and lower than (median - 16dB)
            downX = ssdataX[ant.nr] < (medianValX - 2.0) and ssdataX[ant.nr] > (medianValX - 16.0)
            downY = ssdataY[ant.nr] < (medianValY - 2.0) and ssdataY[ant.nr] > (medianValY - 16.0)
            
            # search for shifted top of band
            for offset in range(minWindow, maxWindow+1):
                if downX:
                    if rcudataX[self.rcudata.testSubband_X+offset] > maxSubbandValueX:
                        maxSubbandValueX = rcudataX[self.rcudata.testSubband_X+offset]
                        maxSubbandOffsetX = offset
                if downY:
                    if rcudataY[self.rcudata.testSubband_Y+offset] > maxSubbandValueY:
                        maxSubbandValueY = rcudataY[self.rcudata.testSubband_Y+offset]
                        maxSubbandOffsetY = offset
            
            # if shifted more than 10 subbands, its probably down
            if maxSubbandOffsetX < -10 or maxSubbandOffsetX > 10:
                ant.x_offset = maxSubbandOffsetX
                ant.down = 1
            if maxSubbandOffsetY < -10 or maxSubbandOffsetY > 10:
                ant.y_offset = maxSubbandOffsetY
                ant.down = 1
                
        self.log.info("Done")
        self.log.printBusyTime(screen=True)  
        return
#### end of cLBA class ####        


# class for testing HBA antennas
class cHBA:
    def __init__(self, hba, logger):
        self.hba = hba
        self.log = logger
        self.rcudata = cRCUdata(hba.nr_tiles*2)
        self.rcumode = 0
    
    def turnOnAllTiles(self):
        for tile in self.hba.tile:
            tile.x_rcu_off = 0
            tile.y_rcu_off = 0
        rspctl("--rcuenable=1")
        return
    
    def turnOffBadTiles(self):
        for tile in self.hba.tile:
            no_modem = 0
            modem_error = 0
            for elem in tile.element:
                if elem.no_modem:
                    no_modem += 1
                if elem.modem_error:
                    modem_error += 1
            if (no_modem + modem_error) >= 15:
                tile.x_rcu_off = 1
                tile.y_rcu_off = 1
                rspctl("--rcuenable=0 --select=%d,%d" %(tile.x_rcu, tile.y_rcu), wait=1.0)
                self.log.info("turned off tile %d RCU(%d,%d)" %(tile.nr, tile.x_rcu, tile.y_rcu), screen=True)
        return
    
    def checkModem(self):
        self.hba.check_done = 1
        self.log.info("Start HBA modem test", screen=True)
        
        if self.rcumode < 5:
            rsp_rcumode(mode=5, n_rcus=self.hba.nr_tiles*2)
            self.rcumode = 5
            
        ctrlstr1 = ('128,'* 16)[:-1] 
        ctrlstr2 = ('253,'* 16)[:-1]
        for ctrl in (ctrlstr1, ctrlstr2):
            rspctl('--hbadelay=%s' %(ctrl), wait=6.0)
            data = rspctl('--realdelays', wait=0.0).splitlines()
            ctrllist = ctrl.split(',')
            for line in data:
                if line[:3] == 'HBA':
                    rcu = int(line[line.find('[')+1:line.find(']')])
                    hba_nr = rcu / 2
                    pol = rcu % 2
                    realctrllist = line[line.find('=')+1:].strip().split()
                    for elem in self.hba.tile[hba_nr].element:
                        if ctrllist[elem.nr] != realctrllist[elem.nr]:
                            self.log.info("Modemtest Tile=%d RCU=%d Element=%d ctrlword=%s response=%s" %\
                                         (hba_nr, rcu, elem.nr+1, ctrllist[elem.nr], realctrllist[elem.nr]), screen=True)
                            
                            if realctrllist[elem.nr].count('?') == 3:
                                elem.no_modem = 1
                            else:
                                elem.modem_error = 1
        
        self.log.info("Done HBA modem test")
        self.log.printBusyTime(screen=True)
        return

    
    def checkNaS(self, mode, sampletime):
        self.hba.check_done = 1
        self.hba.check_time_noise = sampletime
        self.log.info("Start HBA Noise test", screen=True)
        
        if self.rcumode < 5:
            rsp_rcumode(mode=5, n_rcus=self.hba.nr_tiles*2)
            self.rcumode = 5
        
        recordTime = sampletime
        removeAllDataFiles()
        # turn off rf for a second
        rspctl('--hbadelay=%s' %(('0,'* 16)[:-1]), wait=6.0)
        self.log.info("Wait %d seconds while recording data" %(recordTime), screen=True)
        rspctl('--hbadelay=%s' %(('253,'* 16)[:-1]), wait=6.0)
        rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=0:%d' %(recordTime, dataDir(), (self.hba.nr_tiles*2)-1), wait=0.0)
        self.rcudata.readFiles()
        
        bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllX(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("HBA Tile %d X Low Noise, %3.1f%% bad, min.val %5.3f" %(nr, proc[i], val[i]), screen=True)
            self.hba.tile[nr].x_low_noise = 1
            self.hba.tile[nr].x_low_proc  = proc[i]
            self.hba.tile[nr].x_low_val   = val[i]
            self.hba.tile[nr].x_low_ref   = ref
        
        bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllX(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("HBA Tile %d X High Noise, %3.1f%% bad, max.val %5.3f" %(nr, proc[i], val[i]), screen=True)
            self.hba.tile[nr].x_high_noise = 1
            self.hba.tile[nr].x_high_proc  = proc[i]
            self.hba.tile[nr].x_high_val   = val[i]
            self.hba.tile[nr].x_high_ref   = ref
        
        bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllY(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("HBA Tile %d Y Low Noise, %3.1f%% bad, min.val %5.3f" %(nr, proc[i], val[i]), screen=True)
            self.hba.tile[nr].y_low_noise = 1
            self.hba.tile[nr].y_low_proc  = proc[i]
            self.hba.tile[nr].y_low_val   = val[i]
            self.hba.tile[nr].y_low_ref   = ref
        
        bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllY(), self.log)
        for i in range(len(bad)):
            nr = bad[i]
            self.log.info("HBA Tile %d Y High Noise, %3.1f%% bad, max.val %5.3f" %(nr, proc[i], val[i]), screen=True)
            self.hba.tile[nr].y_high_noise = 1
            self.hba.tile[nr].y_high_proc  = proc[i]
            self.hba.tile[nr].y_high_val   = val[i]
            self.hba.tile[nr].y_high_ref   = ref
        
    def checkNaS_elements(self, mode, sampletime):
        self.hba.check_done = 1
        self.hba.check_time_noise_elements = sampletime
        self.log.info("Start HBA Element Noise test", screen=True)
        
        if self.rcumode < 5:
            rsp_rcumode(mode=5, n_rcus=self.hba.nr_tiles*2)
            self.rcumode = 5
        
        record_time = sampletime
        nr_elements = self.hba.tile[0].nr_elements
        
        # turn off rf for a second
        rspctl('--hbadelay=%s' %(('0,'* 16)[:-1]), wait=6.0)
            
        for elem_nr in range(nr_elements):
            removeAllDataFiles()
            ctrlstring = ('2,'*elem_nr + '253,' + '2,'*15)[:33]
            rspctl('--hbadelay=%s' %(ctrlstring), wait=6.0)
            self.log.info("Wait %d seconds while recording data for elements %d" %(record_time, elem_nr+1), screen=True)
            rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=0:%d' %(record_time, dataDir(), (self.hba.nr_tiles*2)-1), wait=0.0)
            self.rcudata.readFiles()
            
            bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllX(), self.log)
            for i in range(len(bad)):
                nr = bad[i]
                if self.hba.tile[nr].x_rcu_off or self.hba.tile[nr].y_rcu_off:
                    self.log.info("skip nas test for tile %d, RCUs are turned off" %(nr), screen=True)
                    continue
                self.log.info("HBA Tile %d Element %d X Low Noise, %3.1f%% bad, min.val %5.3f" %(nr, elem_nr+1, proc[i], val[i]), screen=True)
                self.hba.tile[nr].element[elem_nr].x_low_noise = 1
            
            bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllX(), self.log)
            for i in range(len(bad)):
                nr = bad[i]
                if self.hba.tile[nr].x_rcu_off or self.hba.tile[nr].y_rcu_off:
                    self.log.info("skip nas test for tile %d, RCUs are turned off" %(nr), screen=True)
                    continue
                self.log.info("HBA Tile %d Element %d X High Noise, %3.1f%% bad, max.val %5.3f" %(nr, elem_nr+1, proc[i], val[i]), screen=True)
                self.hba.tile[nr].element[elem_nr].x_high_noise = 1
            
            bad, proc, val, ref = detectBadAntLowSignal(self.rcudata.getAllY(), self.log)
            for i in range(len(bad)):
                nr = bad[i]
                if self.hba.tile[nr].x_rcu_off or self.hba.tile[nr].y_rcu_off:
                    self.log.info("skip nas test for tile %d, RCUs are turned off" %(nr), screen=True)
                    continue
                self.log.info("HBA Tile %d Element %d Y Low Noise, %3.1f%% bad, min.val %5.3f" %(nr, elem_nr+1, proc[i], val[i]), screen=True)
                self.hba.tile[nr].element[elem_nr].y_low_noise = 1    
            
            bad, proc, val, ref = detectBadAntHighSignal(self.rcudata.getAllY(), self.log)
            for i in range(len(bad)):
                nr = bad[i]
                if self.hba.tile[nr].x_rcu_off or self.hba.tile[nr].y_rcu_off:
                    self.log.info("skip nas test for tile %d, RCUs are turned off" %(nr), screen=True)
                    continue
                self.log.info("HBA Tile %d Element %d Y High Noise, %3.1f%% bad, max.val %5.3f" %(nr, elem_nr+1, proc[i], val[i]), screen=True)
                self.hba.tile[nr].element[elem_nr].y_high_noise = 1
            
    #
    # 8bit control word 
    # 
    # bit-7  RF on/off   1 = on
    # bit-6  delay       1 = 8 ns
    # bit-5  delay       1 = 4 ns
    # bit-4  delay       1 = 2 ns
    # bit-3  delay       1 = 1 ns
    # bit-2  delay       1 = 0.5 ns
    # bit-1  LNA on/off  1 = off
    # bit-0  LED on/off  1 = on
    #
    # control = 0 (signal - 30 db)
    # control = 2 (signal - 40 db)
    #
    def checkSignal(self, mode, subband, min_deviation, max_deviation):
        self.hba.check_done = 1
        self.log.info("Start HBA RF test", screen=True)
        
        nr_elements = self.hba.tile[0].nr_elements
        
        if not self.rcumode:
            rsp_rcumode(mode=mode, n_rcus=self.hba.nr_tiles*2)
            self.rcumode = mode
        
        # only for testing
        if False:
            removeAllDataFiles()
            ctrlstring = ('2,'*16)[:-1]
            rspctl('--hbadelay=%s' %(ctrlstring), wait=6.0)
            rspctl('--statistics --duration=2 --integration=1 directory=%s --select=0:%d' %((dataDir(), self.hba.nr_tiles*2)-1), wait=0.0)
            self.rcudata.readFiles()
            data = self.rcudata.getAll()
            print "median of reference data=%5.1f" %(np.ma.median(data))
        
        
        
        # check twice
        # 128 ...
        # 253 ...
        for ctrl in ('128', '253'):
            if ctrl == '128': ctrl_nr = 0
            elif ctrl == '253': ctrl_nr = 1
                
            self.log.info("HBA signal test, ctrl word %s" %(ctrl), screen=True)
            for elem_nr in range(nr_elements):
                self.log.info("HBA signal test, element %d" %(elem_nr+1), screen=True)
                ctrlstring = ('2,'*elem_nr + ctrl + ',' + '2,'*15)[:33]
                
                removeAllDataFiles()
                rspctl('--hbadelay=%s' %(ctrlstring), wait=6.0)
                rspctl('--statistics --duration=2 --integration=1 --directory=%s --select=0:%d' %(dataDir(), (self.hba.nr_tiles*2)-1), wait=0.0)
                self.rcudata.readFiles()
                
                self.rcudata.searchTestSignal(subband=subband, minsignal=90.0, maxsignal=120.0)
                self.log.info("HBA, X used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_X, self.rcudata.testSignal_X), screen=True)
                self.log.info("HBA, Y used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_Y, self.rcudata.testSignal_Y), screen=True)
                
                ssdataX = self.rcudata.getSubbandX()
                ssdataY = self.rcudata.getSubbandY()
                avgX = self.rcudata.testSignal_X
                avgY = self.rcudata.testSignal_Y
                minX = ssdataX.min()
                minY = ssdataY.min()
                
                # if all elements in range
                #if minX < (avgX + self.min_dB) and minY < (avgY + self.min_dB):
                #    continue
                    
                self.log.info("X data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minX, ssdataX.max(), avgX))
                self.log.info("Y data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minY, ssdataY.max(), avgY))
                                
                if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_X == 0:
                    self.log.info("HBA, No valid test signal", screen=True)
                    continue
                
                for tile in self.hba.tile:
                    if tile.x_rcu_off or tile.y_rcu_off:
                        self.log.info("skip signal test for tile %d, RCUs are turned off" %(tile.nr), screen=True)
                        continue
                    tile.element[elem_nr].x_test_signal[ctrl_nr] = avgX
                    tile.element[elem_nr].y_test_signal[ctrl_nr] = avgY
                    tile.element[elem_nr].x_test_subband[ctrl_nr] = self.rcudata.testSubband_X
                    tile.element[elem_nr].y_test_subband[ctrl_nr] = self.rcudata.testSubband_Y
                    tile.element[elem_nr].x_signal[ctrl_nr] = ssdataX[tile.nr]
                    tile.element[elem_nr].y_signal[ctrl_nr] = ssdataY[tile.nr]
                    
                    loginfo = False
                    if ssdataX[tile.nr] < (avgX + min_deviation):
                        tile.element[elem_nr].x_too_low = 1
                        if ssdataX[tile.nr] > 55.0 and ssdataX[tile.nr] < 65.0:
                            tile.element[elem_nr].no_power = 1
                        if ssdataX[tile.nr] < 2.0:
                            tile.element[elem_nr].x_no_signal = 1
                        loginfo = True
                        
                    if ssdataX[tile.nr] > (avgX + max_deviation):
                        tile.element[elem_nr].x_too_high = 1
                        loginfo = True

                    if ssdataY[tile.nr] < (avgY + min_deviation):
                        tile.element[elem_nr].y_too_low = 1
                        if ssdataY[tile.nr] > 55.0 and ssdataY[tile.nr] < 65.0:
                            tile.element[elem_nr].no_power = 1
                        if ssdataY[tile.nr] < 2.0:
                            tile.element[elem_nr].y_no_signal = 1
                        loginfo = True
                        
                    if ssdataY[tile.nr] > (avgY + max_deviation):
                        tile.element[elem_nr].y_too_high = 1
                        loginfo = True

                        
                    if loginfo:    
                        self.log.info("HBA Tile=%d  Element=%d Error:  X=%3.1fdB  Y=%3.1fdB" %\
                                     (tile.nr, elem_nr+1, ssdataX[tile.nr], ssdataY[tile.nr]), screen=True)
                        
                            
                self.log.printBusyTime(screen=True)
                
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=5.0)
        self.log.info("Done HBA signal test")
#### end of cHBA class ####        
        
        
