#!/usr/bin/python
# test lib

from general_lib import *
from lofar_lib import *
from search_lib import *
import os
import numpy as np
import logging

logger = None
def init_test_lib():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger test_lib")


#HBASubband = dict( DE601C=155, DE602C=155, DE603C=284, DE604C=474, DE605C=479, FR606C=155, SE607C=287, UK608C=155 )
#DefaultLBASubband = 301
#DefaultHBASubband = 155


# get and return recorded data in various ways
class cRCUdata:
    global logger
    def __init__(self, n_rcus, minvalue=1):
        self.n_rcus = n_rcus
        self.frames = 0
        self.minvalue = minvalue
        self.ssData = np.ones((n_rcus, 1, 512), np.float64)
        self.testSignal_X = -1.0
        self.testSubband_X = 0
        self.testSignal_Y = -1.0
        self.testSubband_Y = 0
    
    def record(self, rec_time=2):
        removeAllDataFiles()
        logger.info("Wait %d seconds while recording data" %(rec_time))
        rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=0:%d' %(rec_time, dataDir(), self.n_rcus-1), wait=0.0)
        self.readFiles()
    
    def readFile(self, full_filename):
        data = np.fromfile(full_filename, dtype=np.float64)
        self.frames = len(data) / 512
        data = data.reshape(self.frames,512)
        #logger.info("recorded data shape %s" %(str(data.shape)))
        return (data)
        
    def readFiles(self):
        files_in_dir = sorted(os.listdir(dataDir()))
        ssdata = np.array([self.readFile(os.path.join(dataDir(),file_name)) for file_name in files_in_dir])
        # mask zero values and convert to dBm
        self.ssData = np.log10(np.ma.masked_less(ssdata, self.minvalue)) * 10.0
    
    def getSubbands(self, rcu):
        return (self.ssData[int(rcu),:,1:].mean(axis=0))
    
    def getSubbandX(self):
        return (self.ssData[0::2,:,self.testSubband_Y].mean(axis=1))
    
    def getSubbandY(self):
        return (self.ssData[1::2,:,self.testSubband_Y].mean(axis=1))
                       
    def getAll(self):
        return (self.ssData[:,:,1:])
    
    def getAllX(self):
        return (self.ssData[0::2,:,1:])
    
    def getAllY(self):
        return (self.ssData[1::2,:,1:])
           
    def getMedianRcu(self, rcu):
        return(np.ma.median(self.ssData[int(rcu),:,:].mean(axis=0)))

    def searchTestSignal(self, subband=-1, minsignal=75.0, maxsignal=100.0):
        # ss = median for all band over all rcu's
        # forget subband 0    
        ssX = np.ma.median(self.ssData[::2,:,:].mean(axis=1),axis=0)
        ssY = np.ma.median(self.ssData[1::2,:,:].mean(axis=1),axis=0)
        
        if subband != -1:
            if ssX[subband] > minsignal and ssY[subband] > minsignal:
                self.testSignal_X = ssX[subband]
                self.testSubband_X = subband
                self.testSignal_Y = ssY[subband]
                self.testSubband_Y = subband
                return                
                
        # no subband given or not in requested range, look for better
        for i in range(ssX.shape[0]):
            if ssX[i] > minsignal  and ssX[i] < maxsignal and ssX[i] > self.testSignal_X:
                self.testSignal_X = ssX[i]
                self.testSubband_X = i
            if ssY[i] > minsignal  and ssY[i] < maxsignal and ssY[i] > self.testSignal_Y:
                self.testSignal_Y = ssY[i]
                self.testSubband_Y = i
        return
#### end of cRCUdata class ####

# class for checking TBB boards using tbbctl
class cTBB:
    global logger
    def __init__(self, db):
        self.db = db
        self.nr = self.db.nr_tbb
        self.driverstate = True
        #tbbctl('--free')
            
    # check software versions of driver, tbbctl and TP/MP firmware    
    def checkVersions(self, driverV, tbbctlV, tpV, mpV ):
        logger.info("TBB Version check")
        answer = tbbctl('--version')
        
        # check if Driver is available
        if answer.find('TBBDriver is NOT responding') > 0:
            logger.warn("No TBBDriver")
            self.driverstate = False
            self.db.tbbdriver_version = 0
        else:
            infolines = answer.splitlines()
            info = infolines[4:6] + infolines[9:-1]
            #print info
            if info[0].split()[-1] != driverV:
                logger.warn("Not right Driver version")
                self.db.tbbdriver_version = info[0].split()[-1]
            
            if info[1].split()[-1] != tbbctlV:
                logger.warn("Not right tbbctl version")
                self.db.tbbctl_version = info[1].split()[-1]
            
            # check if image_nr > 0 for all boards
            if str(info).count('V') != (self.nr * 4):
                logger.warn("WARNING, Not all boards in working image")
            
            for tbb in self.db.tbb:
                board_info = info[2+tbb.nr].strip().split('  ')
                #print board_info
                if board_info[3].split()[1] != tpV:
                    logger.warn("Board %d Not right TP version" %(tbb.nr))
                    tbb.tp_version = board_info[3].split()[1]    

                if board_info[4].split()[1] != mpV:
                    logger.warn("Board %d Not right MP version" %(tbb.nr))
                    tbb.mp_version = board_info[4].split()[1]
        logger.info("Done")
        self.db.tests += ',TV'
        return
    
    # Check memory address and data lines            
    def checkMemory(self):
        logger.info("TBB Memory check")
        tbbctl('--free')
        for tbb in self.db.tbb:
            answer = tbbctl('--testddr=%d' %(tbb.nr))
            info = answer.splitlines()[-3:]
            ok = True
            if info[0].strip() != 'All Addresslines OK':
                logger.warn("Board %d Addresline error" %(tbb.nr))
                ok = False
            
            if info[1].strip() != 'All Datalines OK':
                logger.warn("Board %d Datalines error" %(tbb.nr))
                ok = False
            
            if not ok:
                tbb.memory_ok = 0
                logger.info(answer)
        logger.info("Done")
        self.db.tests += ',TM'
        return
#### end of cTBB class ####

                
# class for checking RSP boards using rspctl
class cRSP:
    global logger
    def __init__(self, db):
        self.db = db
        self.nr = self.db.nr_rsp
    
    # check software versions of driver, tbbctl and TP/MP firmware    
    def checkVersions(self, bpV, apV ):
        logger.info("RSP Version check")
        answer = rspctl('--version')

        # check if Driver is available
        if answer.find('No Response') > 0:
            logger.warn("No RSPDriver")
            self.db.rspdriver_version = 0
        else:
            infolines = answer.splitlines()
            info = infolines
            
            images_ok = True
            # check if image_nr > 0 for all boards
            if str(info).count('0.0') != 0:
                logger.warn("WARNING, Not all boards in working image")
                images_ok = False
            
            for rsp in self.db.rsp:
                board_info = info[rsp.nr].split(',')
                
                if board_info[1].split()[3] != bpV:
                    logger.warn("Board %d Not right BP version" %(rsp.nr))
                    rsp.bp_version = board_info[1].split()[3]
                    images_ok = False
            
                if board_info[2].split()[3] != apV:
                    logger.warn("Board %d Not right AP version" %(rsp.nr))
                    rsp.ap_version = board_info[2].split()[3]
                    images_ok = False
        
        logger.info("Done")
        self.db.tests += ',RV'
        return (images_ok)
#### end of cRSP class ####


# class for testing LBA antennas
class cLBA:
    global logger
    # mode='lba_low' or 'lba_high'
    def __init__(self, db, lba):
        self.db  = db
        self.lba = lba
        self.rcudata = cRCUdata(self.lba.nr_antennas*2)
        
        # Average normal value = 150.000.000 (81.76 dBm) -3dB +3dB
        # LOW/HIGH LIMIT is used for calculating mean value
        self.lowLimit = -3.0 #dB
        self.highLimit = 3.0 #dB
        
        # MEAN LIMIT is used to check if mean of all antennas is ok
        self.meanLimit = 66.0 #dB
    
    def turnOffAnt(self, ant_nr):
        ant = self.lba.ant[ant_nr]
        ant.x.rcu_off = 1
        ant.y.rcu_off = 1
        logger.info("turned off antenna %d RCU(%d,%d)" %(ant.nr, ant.x.rcu, ant.y.rcu))
        rspctl("--rcumode=0 --select=%d,%d" %(ant.x.rcu, ant.y.rcu), wait=2.0)
        return
    
    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    def checkOscillation(self, mode):
        logger.info("Start LBA Oscillation test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.lba.resetRcuState()
        
        clean = False
        while not clean:
            clean = True
            self.rcudata.record(rec_time=2)
            
            # result is a sorted list on maxvalue
            result = search_oscillation(self.rcudata.getAll(), delta=3.0, start_sb=150, stop_sb=445)
            if len(result) > 0:    
                clean = False
                # get strongest signal, its on array position 0
                peaks_sum, n_peaks, rcu  = result[0]
                ant = rcu / 2
                ant_polarity = rcu % 2
                logger.info("RCU %d LBA %d Oscillation sum=%3.1f peaks=%d" %(rcu, ant, peaks_sum, n_peaks))
                self.turnOffAnt(ant)
                if ant_polarity == 0:
                    self.lba.ant[ant].x.osc = 1
                else:
                    self.lba.ant[ant].y.osc = 1
        
        self.lba.oscillation_check_done = 1
        self.db.tests += ',O%d' %(mode)
        return
    
    def checkNoise(self, mode, record_time, min_deviation, max_deviation, max_diff):
        logger.info("Start LBA Noise test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.lba.resetRcuState()
        
        for ant in self.lba.ant:
            if ant.x.rcu_off or ant.y.rcu_off:
                logger.info("skip low-noise test for antenna %d, RCUs turned off" %(ant.nr))
        
        self.rcudata.record(rec_time=record_time)
        
        # result is a sorted list on maxvalue
        low_noise, high_noise, jitter = search_noise(self.rcudata.getAll(), min_deviation, max_deviation, max_diff)
        
        for n in low_noise:
            rcu, val, proc, ref, diff = n
            ant = rcu / 2
            if self.lba.ant[ant].x.rcu_off or self.lba.ant[ant].y.rcu_off:
                continue
            self.turnOffAnt(ant)
            logger.info("RCU %d Ant %d Low-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, ant, val, proc, ref, diff))
            
            if rcu%2 == 0:                             
                antenna = self.lba.ant[ant].x
            else:                          
                antenna = self.lba.ant[ant].y
                
            if proc > self.lba.ant[ant].x.low_proc:
                antenna.low_noise = 1
                antenna.low_val   = val
                antenna.low_proc  = proc
                antenna.low_ref   = ref
                antenna.low_diff  = diff
            
        for n in high_noise:    
            rcu, val, proc, ref, diff = n
            ant = rcu / 2
            self.turnOffAnt(ant)
            logger.info("RCU %d Ant %d High-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, ant, val, proc, ref, diff))
            
            if rcu%2 == 0:                             
                antenna = self.lba.ant[ant].x
            else:                          
                antenna = self.lba.ant[ant].y
            
            if proc > self.lba.ant[ant].x.high_proc:
                antenna.high_noise = 1
                antenna.high_val   = val
                antenna.high_proc  = proc
                antenna.high_ref   = ref
                antenna.high_diff  = diff
        
        for n in jitter:
            rcu, diff, ref = n
            ant = rcu / 2
            logger.info("RCU %d Ant %d Jitter diff=%3.1f  ref=%3.1f" %(rcu, ant, diff, ref))
            
            if rcu%2 == 0:                             
                antenna = self.lba.ant[ant].x
            else:                          
                antenna = self.lba.ant[ant].y           
            
            antenna.jitter     = 1
            antenna.jitter_val = diff
            antenna.jitter_ref = ref
                
        self.lba.noise_check_done = 1        
        self.db.tests += ',NS%d=%d' %(mode, record_time)
        return    
    
    def checkSpurious(self, mode):
        logger.info("Start LBA Spurious test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.lba.resetRcuState()

        self.rcudata.record(rec_time=1)
        
        # result is a sorted list on maxvalue
        result = search_spurious(self.rcudata.getAll(), delta=3.0)
        for rcu in result:
            ant = rcu / 2
            ant_polarity  = rcu % 2
            self.turnOffAnt(ant)
            logger.info("RCU %d Ant %d pol %d Spurious" %(rcu, ant, ant_polarity))
            if ant_polarity == 0:
                self.lba.ant[ant].x.spurious = 1
            else:
                self.lba.ant[ant].y.spurious = 1
        
        self.lba.spurious_check_done = 1        
        self.db.tests += ',SP%d' %(mode)
        return        
        
        
    def checkSignal(self, mode, subband, min_signal, min_deviation, max_deviation):
        logger.info("Start %s RF test" %(self.lba.label))

        if self.db.rcumode != mode:
            self.db.rcumode = mode
            if mode < 3:
                swapXY(state=1)
            else:
                swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.lba.resetRcuState()
              
        self.rcudata.record(rec_time=2)
        self.rcudata.searchTestSignal(subband=subband, minsignal=min_signal, maxsignal=90.0)
        
        logger.info("For X used test subband=%d (%3.1f dB) in mode %d" %\
                     (self.rcudata.testSubband_X, self.rcudata.testSignal_X, mode))
        logger.info("For Y used test subband=%d (%3.1f dB) in mode %d" %\
                     (self.rcudata.testSubband_Y, self.rcudata.testSignal_Y, mode))             
        
        if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_Y == 0:
            logger.warn("LBA mode %d, No test signal found" %(mode))
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
                
        logger.info("used medianValX=%f" %(medianValX))
        logger.info("used medianValY=%f" %(medianValY))
        if medianValX < self.meanLimit or medianValY < self.meanLimit:
            self.lba.avg_2_low = 1
            self.lba.avg_x = medianValX
            self.lba.avg_y = medianValY
        
        self.lba.test_signal_x = medianValX
        self.lba.test_signal_y = medianValY
        self.lba.test_subband_x = self.rcudata.testSubband_X
        self.lba.test_subband_y = self.rcudata.testSubband_Y

        for ant in self.lba.ant:
            ant.x.test_signal = ssdataX[ant.nr]
            ant.y.test_signal = ssdataY[ant.nr]
            
            loginfo = False
            if ssdataX[ant.nr] < (medianValX + min_deviation):
                ant.x.too_low = 1
                if ssdataX[ant.nr] < 2.0:
                    ant.x.rcu_error = 1
                loginfo = True
            
            if ssdataX[ant.nr] > (medianValX + max_deviation):
                ant.x.too_high = 1
                loginfo = True
            
            if ssdataY[ant.nr] < (medianValY + min_deviation):
                ant.y.too_low = 1
                if ssdataY[ant.nr] < 2.0:
                    ant.y.rcu_error = 1
                loginfo = True
            
            if ssdataY[ant.nr] > (medianValY + max_deviation):
                ant.y.too_high = 1
                loginfo = True
            
            if loginfo:
                logger.info("%s %2d  RCU %3d/%3d   X=%5.1fdB  Y=%5.1fdB" %(self.lba.label, ant.nr, ant.x.rcu, ant.y.rcu, ssdataX[ant.nr], ssdataY[ant.nr]))
                     
        # mark lba as down if top of band is lower than normal and top is shifted more than 10 subbands to left or right
        
        downX = searchDown(self.rcudata.getAllX())
        downY = searchDown(self.rcudata.getAllY())
        for i in downX:
            ant, max_sb, mean_max_sb = i
            max_offset = max_sb - mean_max_sb
            
            self.lba.ant[ant].x.offset = max_offset
            self.lba.ant[ant].down = 1
            logger.info("%s %2d RCU %3d Down, offset=%d" %(self.lba.label, ant, self.lba.ant[ant].x.rcu, max_offset))
        for i in downY:
            ant = i[0]
            self.lba.ant[ant].y.offset = max_offset
            self.lba.ant[ant].down = 1
            logger.info("%s %2d RCU %3d Down, offset=%d" %(self.lba.label, ant, self.lba.ant[ant].y.rcu, max_offset))
                
        """
        minWindow = -40
        maxWindow = 40
        for ant in self.lba.ant:
            rcudataX = self.rcudata.getSubbands(ant.x.rcu)
            rcudataY = self.rcudata.getSubbands(ant.y.rcu)
            
            maxSubbandOffsetX = -1
            maxSubbandValueX = 0
            maxSubbandOffsetY = -1
            maxSubbandValueY = 0
            
            # possible down if lower than (median - 2.0dB), and higher than (median - 15dB)
            downX = ssdataX[ant.nr] < (medianValX - 2.0) and ssdataX[ant.nr] > (medianValX - 15.0)
            downY = ssdataY[ant.nr] < (medianValY - 2.0) and ssdataY[ant.nr] > (medianValY - 15.0)
            
            # search for shifted top of band
            for offset in range(minWindow, maxWindow+1, 1):
                if downX and downY:
                    if rcudataX[self.rcudata.testSubband_X+offset] > maxSubbandValueX:
                        maxSubbandValueX = rcudataX[self.rcudata.testSubband_X+offset]
                        maxSubbandOffsetX = offset
                    if rcudataY[self.rcudata.testSubband_Y+offset] > maxSubbandValueY:
                        maxSubbandValueY = rcudataY[self.rcudata.testSubband_Y+offset]
                        maxSubbandOffsetY = offset
            
            # if shifted more than 10 subbands, its probably down
            if abs(maxSubbandOffsetX - maxSubbandOffsetY) > 10:
                ant.x.offset = maxSubbandOffsetX
                ant.y.offset = maxSubbandOffsetY
                ant.down = 1
        """        
        logger.info("Done")
        self.lba.signal_check_done = 1
        self.db.tests += ',S%d' %(mode)
        return
#### end of cLBA class ####        


# class for testing HBA antennas
class cHBA:
    global logger
    def __init__(self, db, hba):
        self.db  = db
        self.hba = hba
        self.rcudata = cRCUdata(hba.nr_tiles*2)
        self.rcumode = 0
    
    def turnOnTiles(self):
        pass
    
    def turnOffTile(self, tile_nr):
        tile = self.hba.tile[tile_nr]
        tile.x.rcu_off = 1
        tile.y.rcu_off = 1
        logger.info("turned off tile %d RCU(%d,%d)" %(tile.nr, tile.x.rcu, tile.y.rcu))
        rspctl("--rcumode=0 --select=%d,%d" %(tile.x.rcu, tile.y.rcu), wait=2.0)
        return
   
    def turnOffBadTiles(self):
        for tile in self.hba.tile:
            if tile.x.rcu_off and tile.y.rcu_off:
                continue
            no_modem = 0
            modem_error = 0
            for elem in tile.element:
                if elem.no_modem:
                    no_modem += 1
                if elem.modem_error:
                    modem_error += 1
            if tile.x.osc or tile.y.osc or (no_modem >= 8) or (modem_error >= 8):
                self.turnOffTile(tile.nr)
        return
        
    def checkModem(self, mode):
        logger.info("Start HBA modem test")
        
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        ctrlstr1 = ('128,'* 16)[:-1] 
        ctrlstr2 = ('253,'* 16)[:-1]
        for ctrl in (ctrlstr1, ctrlstr2):
            rspctl('--hbadelay=%s' %(ctrl), wait=8.0)
            data = rspctl('--realdelays', wait=0.0).splitlines()
            ctrllist = ctrl.split(',')
            for line in data:
                if line[:3] == 'HBA':
                    rcu = int(line[line.find('[')+1:line.find(']')])
                    hba_nr = rcu / 2
                    ant_polarity = rcu % 2
                    realctrllist = line[line.find('=')+1:].strip().split()
                    for elem in self.hba.tile[hba_nr].element:
                        if ctrllist[elem.nr] != realctrllist[elem.nr]:
                            logger.info("Modemtest Tile=%d RCU=%d Element=%d ctrlword=%s response=%s" %\
                                         (hba_nr, rcu, elem.nr+1, ctrllist[elem.nr], realctrllist[elem.nr]))
                            
                            if realctrllist[elem.nr].count('?') == 3:
                                elem.no_modem += 1
                            else:
                                elem.modem_error += 1
        
        logger.info("Done HBA modem test")
        self.hba.modem_check_done = 1
        self.db.tests += ',M'   
        return
        
    # check for summator noise and turn off RCU 
    def checkSummatorNoise(self, mode):
        logger.info("Start HBA SummatorNoise test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
            
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        self.rcudata.record(rec_time=1)
        
        sum_noise = search_summator_noise(self.rcudata.getAllX(), start_sb=45, stop_sb=350)
        for n in sum_noise:
            tile, val, n_peaks = n
            logger.info("RCU %d Tile %d Summator-Noise val=%3.1f peaks=%3.1f" %(self.hba.tile[tile].x.rcu, tile, val, n_peaks))
            self.hba.tile[tile].x.summator_noise = 1
            self.turnOffTile(tile)
        
        sum_noise = search_summator_noise(self.rcudata.getAllY(), start_sb=45, stop_sb=350)
        for n in sum_noise:
            tile, val, n_peaks = n
            logger.info("RCU %d Tile %d Summator-Noise val=%3.1f peaks=%3.1f" %(self.hba.tile[tile].y.rcu, tile, val, n_peaks))
            self.hba.tile[tile].y.summator_noise = 1
            self.turnOffTile(tile)
            
        self.hba.summatornoise_check_done = 1    
        self.db.tests += ',SN'
        return
    
    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    def checkOscillation(self, mode):
        logger.info("Start HBA Oscillation test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        clean = False
        while not clean:
            clean = True
            self.rcudata.record(rec_time=1)
            
            # result is a sorted list on maxvalue
            result = search_oscillation(self.rcudata.getAll(), delta=10.0, start_sb=45, stop_sb=350)
            if len(result) > 0:    
                clean = False
                max_sum, n_peaks, rcu = result[0]
                tile = rcu / 2
                tile_polarity  = rcu % 2
                logger.info("RCU %d Tile %d Oscillation sum=%3.1f peaks=%d" %(rcu, tile, max_sum, n_peaks))
                self.turnOffTile(tile)
                if tile_polarity == 0:
                    self.hba.tile[tile].x.osc = 1
                else:
                    self.hba.tile[tile].y.osc = 1
        self.hba.oscillation_check_done = 1
        self.db.tests += ',O%d' %(mode)
        return
    
    def checkNoise(self, mode, record_time, min_deviation, max_deviation, max_diff):
        logger.info("Start HBA Noise test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip low-noise test for tile %d, RCUs turned off" %(tile.nr))
                
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        self.rcudata.record(rec_time=record_time)
        
        # result is a sorted list on maxvalue
        low_noise, high_noise, jitter = search_noise(self.rcudata.getAll(), min_deviation, max_deviation, max_diff)
        
        for n in low_noise:
            rcu, val, proc, ref, diff = n
            tile = rcu / 2
            if self.hba.tile[tile].x.rcu_off or self.hba.tile[tile].y.rcu_off:
                continue
            logger.info("RCU %d Tile %d Low-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, tile, val, proc, ref, diff))
            
            if rcu%2 == 0:
                tile_polarity = self.hba.tile[tile].x
            else:
                tile_polarity = self.hba.tile[tile].y
            
            if proc > tile_polarity.low_proc:
                tile_polarity.low_noise = 1
                tile_polarity.low_val   = val
                tile_polarity.low_proc  = proc
                tile_polarity.low_ref   = ref
                tile_polarity.low_diff  = diff
            f
                
        for n in high_noise:    
            rcu, val, proc, ref, diff = n
            tile = rcu / 2
            logger.info("RCU %d Tile %d High-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, tile, val, proc, ref, diff))
            
            if rcu%2 == 0:
                tile_polarity = self.hba.tile[tile].x
            else:
                tile_polarity = self.hba.tile[tile].y
                
            if proc > tile_polarity.high_proc:
                tile_polarity.high_noise = 1
                tile_polarity.high_val   = val
                tile_polarity.high_proc  = proc
                tile_polarity.high_ref   = ref
                tile_polarity.high_diff  = diff
        
        for n in jitter:
            rcu, val, ref = n
            tile = rcu / 2
            logger.info("RCU %d Tile %d Jitter diff=%3.1f ref=%3.1f" %(rcu, tile, val, ref))
            
            if rcu%2 == 0:
                tile_polarity = self.hba.tile[tile].x
            else:
                tile_polarity = self.hba.tile[tile].y
                
            tile_polarity.jitter     = 1
            tile_polarity.jitter_val = val
            tile_polarity.jitter_ref = ref
        
        self.hba.noise_check_done = 1
        self.db.tests += ',NS%d=%d' %(mode, record_time)
        return    
    
    def checkSpurious(self, mode):
        logger.info("Start HBA Spurious test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
            
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        self.rcudata.record(rec_time=1)
        
        # result is a sorted list on maxvalue
        result = search_spurious(self.rcudata.getAll(), delta=3.0)
        for rcu in result:
            tile = rcu / 2
            tile_polarity  = rcu % 2
            logger.info("RCU %d Tile %d pol %d Spurious" %(rcu, tile, tile_polarity))
            if tile_polarity == 0:
                self.hba.tile[tile].x.spurious = 1
            else:
                self.hba.tile[tile].y.spurious = 1
        
        self.hba.spurious_check_done = 1
        self.db.tests += ',SP%d' %(mode)
        return    
    
    def checkSignal(self, mode, subband, min_signal, min_deviation, max_deviation):
        logger.info("Start HBA RF test")
        
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        # check twice
        # 128 ...
        # 253 ...
        
        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip signal test for tile %d, RCUs turned off" %(tile.nr))
                    
        for ctrl in ('128,', '253,'):
            if ctrl == '128,': ctrl_nr = 0
            elif ctrl == '253,': ctrl_nr = 1
                
            logger.info("HBA signal test, ctrl word %s" %(ctrl[:-1]))
            
            rspctl('--hbadelay=%s' %((ctrl* 16)[:-1]), wait=8.0)        
            self.rcudata.record(rec_time=2)
            self.rcudata.searchTestSignal(subband=subband, minsignal=min_signal, maxsignal=150.0)
            logger.info("HBA, X used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_X, self.rcudata.testSignal_X))
            logger.info("HBA, Y used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_Y, self.rcudata.testSignal_Y))
            
            if (self.rcudata.testSubband_X != 0) and (self.rcudata.testSubband_Y != 0):
                ssdataX = self.rcudata.getSubbandX()
                ssdataY = self.rcudata.getSubbandY()
                avgX = self.rcudata.testSignal_X
                avgY = self.rcudata.testSignal_Y
                minX = ssdataX.min()
                minY = ssdataY.min()
                
                # if all elements in range
                #if minX < (avgX + self.min_dB) and minY < (avgY + self.min_dB):
                #    continue
                    
                logger.debug("X data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minX, ssdataX.max(), avgX))
                logger.debug("Y data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minY, ssdataY.max(), avgY))
                                
                if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_X == 0:
                    logger.warn("HBA, No valid test signal")
                    return
                
                for tile in self.hba.tile:
                    if tile.x.rcu_off or tile.y.rcu_off:
                        continue
                    tile.x.ref_signal[ctrl_nr] = avgX
                    tile.y.ref_signal[ctrl_nr] = avgY
                    tile.x.test_subband[ctrl_nr] = self.rcudata.testSubband_X
                    tile.y.test_subband[ctrl_nr] = self.rcudata.testSubband_Y
                    tile.x.test_signal[ctrl_nr] = ssdataX[tile.nr]
                    tile.y.test_signal[ctrl_nr] = ssdataY[tile.nr]
                    
                    loginfo = False
                    if ssdataX[tile.nr] < (avgX + min_deviation):
                        if ssdataX[tile.nr] < 2.0:
                            tile.x.no_signal = 1
                        elif ssdataX[tile.nr] > 55.0 and ssdataX[tile.nr] < 65.0:
                            tile.no_power = 1
                        else:
                            tile.x.too_low = 1
                        loginfo = True
                        
                    if ssdataX[tile.nr] > (avgX + max_deviation):
                        tile.x.too_high = 1
                        loginfo = True
    
                    if ssdataY[tile.nr] < (avgY + min_deviation):
                        if ssdataY[tile.nr] < 2.0:
                            tile.y.no_signal = 1
                        elif ssdataY[tile.nr] > 55.0 and ssdataY[tile.nr] < 65.0:
                            tile.no_power = 1
                        else:
                            tile.y.too_low = 1
                        loginfo = True
                        
                    if ssdataY[tile.nr] > (avgY + max_deviation):
                        tile.y.too_high = 1
                        loginfo = True
                        
                    if loginfo:    
                        logger.info("HBA Tile=%d  Error:  X=%3.1fdB  Y=%3.1fdB" %\
                                     (tile.nr, ssdataX[tile.nr], ssdataY[tile.nr]))
                        
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        logger.info("Done HBA signal test")
        
        self.hba.signal_check_done = 1
        self.db.tests += ',S%d' %(mode)
        return
    
    # Next tests are element based
    
    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    def checkOscillationElements(self, mode):
        logger.info("Start HBA element Oscillation test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        for elem in range(self.hba.tile[0].nr_elements):
            rsp_rcu_mode(mode=mode, n_rcus=self.db.nr_rcu)
            ctrlstring = ('2,'*elem + '253,' + '2,'*15)[:33]
            rspctl('--hbadelay=%s' %(ctrlstring), wait=8.0)
            clean = False
            while not clean:
                clean = True
                self.rcudata.record(rec_time=5)
                
                # result is a sorted list on maxvalue
                result = search_oscillation(self.rcudata.getAll(), delta=10.0, start_sb=162, stop_sb=467)
                if len(result) > 0:    
                    clean = False
                    strongest_rcu = result[0][2]
                    tile = strongest_rcu / 2
                    tile_polarity  = strongest_rcu % 2
                    logger.info("RCU %d Tile %d Element %d Oscillation sum=%3.1f peaks=%d" %(strongest_rcu, tile, elem, result[0][0], result[0][1]))
                    self.turnOffTile(tile)
                    if tile_polarity == 0:
                        self.hba.tile[tile].element[elem].x.osc = 1
                    else:
                        self.hba.tile[tile].element[elem].y.osc = 1
        
        self.hba.element_check_done = 1
        self.db.tests += ',EO'
        return
    
    def checkNoiseElements(self, mode, record_time, min_deviation, max_deviation, max_diff):
        logger.info("Start HBA Noise test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        for elem in range(self.hba.tile[0].nr_elements):
            rsp_rcu_mode(mode=mode, n_rcus=self.db.nr_rcu)
            for tile in self.hba.tile:
                bad_tile = False
                for e in tile.element:
                    if e.x.osc or e.y.osc:
                        bad_tile = True
                if bad_tile:
                    self.turnOffTile(tile.nr)
            
            ctrlstring = ('2,'*elem + '253,' + '2,'*15)[:33]
            rspctl('--hbadelay=%s' %(ctrlstring), wait=10.0)
            self.rcudata.record(rec_time=record_time)
        
            # result is a sorted list on maxvalue
            low_noise, high_noise, jitter = search_noise(self.rcudata.getAll(), min_deviation, max_deviation, max_diff)
            
            for n in low_noise:
                rcu, val, proc, ref, diff = n
                tile = rcu / 2
                logger.info("RCU %d Tile %d Element %d Low-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, tile, elem, val, proc, ref, diff))
                
                if rcu%2 == 0:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                else:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                
                elem_polarity.low_noise = 1
                elem_polarity.low_val   = val
                elem_polarity.low_proc  = proc
                elem_polarity.low_ref   = ref
                elem_polarity.low_diff  = diff
                    
            for n in high_noise:    
                rcu, val, proc, ref, diff = n
                tile = rcu / 2
                logger.info("RCU %d Tile %d Element %d High-Noise value=%3.1f badness=%3.1f%% ref=%3.1f diff=%3.1f" %(rcu, tile, elem, val, proc, ref, diff))
                
                if rcu%2 == 0:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                else:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                
                elem_polarity.high_noise = 1
                elem_polarity.high_val   = val
                elem_polarity.high_proc  = proc
                elem_polarity.high_ref   = ref
                elem_polarity.high_diff  = diff
            
            for n in jitter:
                rcu, val, ref = n
                tile = rcu / 2
                logger.info("RCU %d Tile %d Element %d Jitter %3.1f  ref=%3.1f" %(rcu, tile, elem, val, ref))
                
                if rcu%2 == 0:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                else:
                    elem_polarity = self.hba.tile[tile].element[elem].x 
                    
                elem_polarity.jitter     = 1
                elem_polarity.jitter_val = val
                elem_polarity.jitter_ref = ref
        
        self.hba.element_check_done = 1
        self.db.tests += ',EN=%d' %(record_time)
        return    
    
    def checkSpuriousElements(self, mode):
        logger.info("Start HBA Spurious test")
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
            
        for elem in range(self.hba.tile[0].nr_elements):
            rsp_rcu_mode(mode=mode, n_rcus=self.db.nr_rcu)
            for tile in self.hba.tile:
                bad_tile = False
                for e in tile.element:
                    if e.x.osc or e.y.osc:
                        bad_tile = True
                if bad_tile:
                    self.turnOffTile(tile.nr)
            
            ctrlstring = ('2,'*elem + '253,' + '2,'*15)[:33]
            rspctl('--hbadelay=%s' %(ctrlstring), wait=8.0)
        
            self.rcudata.record(rec_time=1)
            
            # result is a sorted list on maxvalue
            result = search_spurious(self.rcudata.getAll(), delta=3.0)
            for rcu in result:
                tile = rcu / 2
                tile_polarity  = rcu % 2
                logger.info("RCU %d Tile %d Element %d pol %d Spurious" %(rcu, tile, elem, tile_polarity))
                if tile_polarity == 0:
                    self.hba.tile[tile].element[elem].x.spurious = 1
                else:
                    self.hba.tile[tile].element[elem].y.spurious = 1
        
        self.hba.element_check_done = 1
        self.db.tests += ',ESP'
        return    
        
    
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
    def checkSignalElements(self, mode, subband, min_signal, min_deviation, max_deviation):
        logger.info("Start HBA RF test")
        
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            swapXY(state=0)
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.db.nr_rcu)
            self.hba.resetRcuState()
        
        nr_elements = self.hba.tile[0].nr_elements
        
        # check twice
        # 128 ...
        # 253 ...
        
        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip signal test for tile %d, RCUs are turned off" %(tile.nr))
                    
        for ctrl in ('128', '253'):
            if ctrl == '128': ctrl_nr = 0
            elif ctrl == '253': ctrl_nr = 1
                
            logger.info("HBA signal test, ctrl word %s" %(ctrl))
                    
            for elem_nr in range(nr_elements):
                logger.info("HBA signal test, element %d" %(elem_nr+1))
                ctrlstring = ('2,'*elem_nr + ctrl + ',' + '2,'*15)[:33]
                
                self.rcudata.record(rec_time=2)
                self.rcudata.searchTestSignal(subband=subband, minsignal=min_signal, maxsignal=120.0)
                logger.info("HBA, X used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_X, self.rcudata.testSignal_X))
                logger.info("HBA, Y used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_Y, self.rcudata.testSignal_Y))
                
                ssdataX = self.rcudata.getSubbandX()
                ssdataY = self.rcudata.getSubbandY()
                avgX = self.rcudata.testSignal_X
                avgY = self.rcudata.testSignal_Y
                minX = ssdataX.min()
                minY = ssdataY.min()
                
                # if all elements in range
                #if minX < (avgX + self.min_dB) and minY < (avgY + self.min_dB):
                #    continue
                    
                logger.debug("X data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minX, ssdataX.max(), avgX))
                logger.debug("Y data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minY, ssdataY.max(), avgY))
                                
                if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_X == 0:
                    logger.warn("HBA, No valid test signal")
                    return
                
                for tile in self.hba.tile:
                    if tile.x.rcu_off or tile.y.rcu_off:
                        continue
                    tile.element[elem_nr].x.ref_signal[ctrl_nr] = avgX
                    tile.element[elem_nr].y.ref_signal[ctrl_nr] = avgY
                    tile.element[elem_nr].x.test_subband[ctrl_nr] = self.rcudata.testSubband_X
                    tile.element[elem_nr].y.test_subband[ctrl_nr] = self.rcudata.testSubband_Y
                    tile.element[elem_nr].x.test_signal[ctrl_nr] = ssdataX[tile.nr]
                    tile.element[elem_nr].y.test_signal[ctrl_nr] = ssdataY[tile.nr]
                    
                    loginfo = False
                    if ssdataX[tile.nr] < (avgX + min_deviation):
                        if ssdataX[tile.nr] < 2.0:
                            tile.element[elem_nr].x.no_signal = 1
                        elif ssdataX[tile.nr] > 55.0 and ssdataX[tile.nr] < 65.0:
                            tile.element[elem_nr].no_power = 1
                        else:
                            tile.element[elem_nr].x.too_low = 1
                        loginfo = True
                        
                    if ssdataX[tile.nr] > (avgX + max_deviation):
                        tile.element[elem_nr].x.too_high = 1
                        loginfo = True

                    if ssdataY[tile.nr] < (avgY + min_deviation):
                        if ssdataY[tile.nr] < 2.0:
                            tile.element[elem_nr].y.no_signal = 1
                        elif ssdataY[tile.nr] > 55.0 and ssdataY[tile.nr] < 65.0:
                            tile.element[elem_nr].no_power = 1
                        else:
                            tile.element[elem_nr].y.too_low = 1
                        loginfo = True
                        
                    if ssdataY[tile.nr] > (avgY + max_deviation):
                        tile.element[elem_nr].y.too_high = 1
                        loginfo = True
                        
                    if loginfo:    
                        logger.info("HBA Tile=%d  Element=%d Error:  X=%3.1fdB  Y=%3.1fdB" %\
                                     (tile.nr, elem_nr+1, ssdataX[tile.nr], ssdataY[tile.nr]))
                        
        rspctl('--hbadelay=%s' %(('128,'* 16)[:-1]), wait=8.0)
        logger.info("Done HBA signal test")
        
        self.hba.element_check_done = 1
        self.db.tests += ',ES'
        return
#### end of cHBA class ####        
        
        
