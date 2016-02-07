#!/usr/bin/python
# test lib

from general_lib import *
from lofar_lib import *
from search_lib import *
from data_lib import *
import os
import numpy as np
import logging

test_version = '0815'

logger = None
def init_test_lib():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger test_lib")


#HBASubband = dict( DE601C=155, DE602C=155, DE603C=284, DE604C=474, DE605C=479, FR606C=155, SE607C=287, UK608C=155 )
#DefaultLBASubband = 301
#DefaultHBASubband = 155

class cSPU:
    def __init__(self, db):
        self.db = db
        self.board_info_str = []
        self.board_info_val = [-1, 0.0, 0.0, 0.0, 0.0, 0]
    
    def extract_board_info(self, line):
        li = line.split("|")
        if not li[0].strip().isdigit():
            return False
            
        self.board_info_str = [i.strip() for i in li]
                
        if li[0].strip().isdigit():
            self.board_info_val[0] = int(li[0].strip())
        else:
            self.board_info_val[0] = -1
            
        for i in xrange(1, 5, 1):
            if li[i].strip().replace('.','').isdigit():
                self.board_info_val[i] = float(li[i].strip())
            else:
                self.board_info_val[i] = 0.0
                                
        if li[5].strip().isdigit():
            self.board_info_val[5] = int(li[5].strip())
        else:
            self.board_info_val[5] = 0
        return True
        
        
    def checkStatus(self):
        """
        check PSU if boards idle and fully loaded
        """
        # in future get all settings from configuration file
        max_3_3 = 3.4
        min_3_3 = 3.1
        max_drop_3_3 = 0.3
        
        max_5_0 = 5.0
        min_5_0 = 4.5
        max_drop_5_0 = 0.3
        
        max_8_0 = 8.0
        min_8_0 = 7.4
        max_drop_8_0 = 0.3
        
        max_48  = 48.0
        min_48  = 43.0
        max_drop_48 = 2.0
          
        logger.info("=== SPU status check ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        
        noload = []
        fullload_3 = []
        fullload_5 = []
        logger.debug("check spu no load")
        answer = rspctl('--spustatus')

        # check if Driver is available
        if answer.find('No Response') > 0:
            logger.warn("No RSPDriver")
            self.db.rspdriver_version = 0
        else:
            infolines = answer.splitlines()
            for line in infolines:
                if self.extract_board_info(line):
                    bi = self.board_info_val
                    noload.append([bi[0], bi[1], bi[2], bi[3], bi[4]])
                    self.db.spu[bi[0]].temp = bi[5]
                    bi = self.board_info_str
                    logger.debug("Subrack %s voltages: rcu=%s  lba=%s  hba=%s  spu=%s  temp: %s" % (
                                 bi[0], bi[1], bi[2], bi[3], bi[4], bi[5]))

            # turn on all hbas
            logger.debug("check spu full load mode 3")
            rsp_rcu_mode(3, self.db.lbh.selectList())
            answer = rspctl('--spustatus')
            infolines = answer.splitlines()
            for line in infolines:
                if self.extract_board_info(line):
                    bi = self.board_info_val
                    fullload_3.append([bi[0], bi[1], bi[2], bi[3], bi[4]])
                    bi = self.board_info_str
                    logger.debug("Subrack %s voltages: rcu=%s  lba=%s  hba=%s  spu=%s  temp: %s" % (
                                 bi[0], bi[1], bi[2], bi[3], bi[4], bi[5]))

            # turn on all hbas
            logger.debug("check spu full load mode 5")
            rsp_rcu_mode(5, self.db.hba.selectList())
            answer = rspctl('--spustatus')
            infolines = answer.splitlines()
            for line in infolines:
                if self.extract_board_info(line):
                    bi = self.board_info_val
                    fullload_5.append([bi[0], bi[1], bi[2], bi[3], bi[4]])
                    bi = self.board_info_str
                    logger.debug("Subrack %s voltages: rcu=%s  lba=%s  hba=%s  spu=%s  temp: %s" % (
                                 bi[0], bi[1], bi[2], bi[3], bi[4], bi[5]))
                                
            for sr in range(self.db.nr_spu):
                # calculate mean of noload, fullload_3, fullload_5
                self.db.spu[sr].rcu_5_0V = (noload[sr][1] + fullload_3[sr][1] + fullload_5[sr][1]) / 3.0
                self.db.spu[sr].lba_8_0V = fullload_3[sr][2]
                self.db.spu[sr].hba_48V  = fullload_5[sr][3]
                self.db.spu[sr].spu_3_3V = (noload[sr][4] + fullload_3[sr][4] + fullload_5[sr][4]) / 3.0
                
                if (self.db.spu[sr].temp > 35.0):
                    self.db.spu[sr].temp_ok = 0
                
                if (not (min_5_0 <= noload[sr][1] <= max_5_0)
                or not (min_5_0 < fullload_3[sr][1] <= max_5_0)
                or not (min_5_0 <= fullload_5[sr][1] <= max_5_0)
                or (noload[sr][1] - fullload_3[sr][1]) > max_drop_5_0):
                    self.db.spu[sr].rcu_ok = 0
                    logger.info("SPU voltage 5.0V out of range")
                    
                if (not (min_8_0 <= noload[sr][2] <= max_8_0)
                or not (min_8_0 <= fullload_3[sr][2] <= max_8_0)
                or (noload[sr][2] - fullload_3[sr][2]) > max_drop_8_0):
                    self.db.spu[sr].lba_ok = 0   
                    logger.info("SPU voltage 8.0V out of range")
                
                if (not (min_48 <= noload[sr][3] <= max_48)
                or not (min_48 <= fullload_5[sr][3] <= max_48)
                or (noload[sr][3] - fullload_5[sr][3]) > max_drop_48):
                    self.db.spu[sr].hba_ok = 0   
                    logger.info("SPU voltage 48V out of range")
                
                if (not (min_3_3 <= noload[sr][4] <= max_3_3)
                or not (min_3_3 <= fullload_3[sr][4] <= max_3_3)
                or not (min_3_3 <= fullload_5[sr][4] <= max_3_3)
                or (noload[sr][4] - fullload_5[sr][4]) > max_drop_3_3):
                    self.db.spu[sr].spu_ok = 0   
                    logger.info("SPU voltage 3.3V out of range")
                    
        logger.info("=== Done SPU check ===")
        self.db.addTestDone('SPU')
        return

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
        logger.info("=== TBB Version check ===")
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
        logger.info("=== Done TBB Version check ===")
        self.db.addTestDone('TV')
        return

    # Check memory address and data lines
    def checkMemory(self):
        logger.info("=== TBB Memory check ===")
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
        # turn on recording again
        tbbctl('--alloc')
        tbbctl('--record')
        rspctl('--tbbmode=transient')
        logger.info("=== Done TBB Memory check ===")
        self.db.addTestDone('TM')
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
        logger.info("=== RSP Version check ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
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

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return (False)

        logger.info("=== Done RSP Version check ===")
        self.db.addTestDone('RV')
        return (images_ok)

    def checkBoard(self):
        max_1_2 = 1.3
        min_1_2 = 1.1
        
        max_2_5 = 2.6
        min_2_5 = 2.4
        
        max_3_3 = 3.4
        min_3_3 = 3.1
        
        ok = True
        logger.info("=== RSP Board check ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return (False)
        answer = rspctl('--status')
        p2 = 0
        for rsp in self.db.rsp:
            p1 = answer.find("RSP[%2d]" %(rsp.nr), p2)
            p2 = answer.find("\n", p1)
            d = [float(i.split(":")[1].strip()) for i in answer[p1+7:p2].split(',')]
            if len(d) == 3:
                logger.debug("RSP board %d: [1.2V]=%3.2fV, [2.5V]=%3.2fV, [3.3V]=%3.2fV" %(rsp.nr, d[0],  d[1],  d[2]))
                rsp.voltage1_2 = d[0]
                if not (min_1_2 <= d[0] <= max_1_2):
                    rsp.voltage_ok = 0
                    logger.info("RSP board %d [1.2V]=%3.2fV" %(rsp.nr, d[0]))
                rsp.voltage2_5 = d[1]
                if not (min_2_5 <= d[1] <= max_2_5):
                    rsp.voltage_ok = 0
                    logger.info("RSP board %d [2.5V]=%3.2fV" %(rsp.nr, d[1]))
                rsp.voltage3_3 = d[2]
                if not (min_3_3 <= d[2] <= max_3_3):
                    rsp.voltage_ok = 0
                    logger.info("RSP board %d [3.3V]=%3.2fV" %(rsp.nr, d[2]))

        for rsp in self.db.rsp:
            p1 = answer.find("RSP[%2d]" %(rsp.nr), p2)
            p2 = answer.find("\n", p1)
            d = [float(i.split(":")[1].strip()) for i in answer[p1+7:p2].split(',')]
            if len(d) == 6:
                logger.debug("RSP board %d temperatures: pcb=%3.1f, bp=%3.1f, ap0=%3.1f, ap1=%3.1f, ap2=%3.1f, ap3=%3.1f" %(
                             rsp.nr, 
                             d[0], d[1], d[2], d[3], d[4], d[5]))
                rsp.pcb_temp   = d[0]
                if d[0] > 45.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [pcb_temp]=%2.0f" %(rsp.nr, d[0]))
                rsp.bp_temp    = d[1]
                if d[1] > 75.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [bp_temp]=%2.0f" %(rsp.nr, d[1]))
                rsp.ap0_temp   = d[2]
                if d[2] > 75.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [ap0_temp]=%2.0f" %(rsp.nr, d[2]))
                rsp.ap1_temp   = d[3]
                if d[3] > 75.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [ap1_temp]=%2.0f" %(rsp.nr, d[3]))
                rsp.ap2_temp   = d[4]
                if d[4] > 75.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [ap2_temp]=%2.0f" %(rsp.nr, d[4]))
                rsp.ap3_temp   = d[5]
                if d[5] > 75.0:
                    rsp.temp_ok = 0
                    logger.info("RSP board %d [ap3_temp]=%2.0f" %(rsp.nr, d[5]))
        logger.info("=== Done RSP Board check ===")
        self.db.addTestDone('RBC')
        return (ok)

#### end of cRSP class ####


# class for testing LBA antennas
class cLBA:
    #global logger
    # mode='lba_low' or 'lba_high'
    def __init__(self, db, lba):
        self.db  = db
        self.lba = lba
        self.rcudata = cRCUdata(self.lba.nr_antennas*2)
        self.rcudata.setActiveRcus(self.lba.selectList())
        
        # Average normal value = 150.000.000 (81.76 dBm) -3dB +3dB
        # LOW/HIGH LIMIT is used for calculating mean value
        self.lowLimit = -3.0 #dB
        self.highLimit = 3.0 #dB

        # MEAN LIMIT is used to check if mean of all antennas is ok
        self.meanLimit = 66.0 #dB
    
    def reset(self):
        self.rcudata.reset()
        self.rcudata.reset_masks()
        self.rcudata.setActiveRcus(self.lba.selectList())
    
    def turnOffAnt(self, ant_nr):
        ant = self.lba.ant[ant_nr]
        ant.x.rcu_off = 1
        ant.y.rcu_off = 1
        self.rcudata.setInActiveRcu(ant.x.rcu)
        self.rcudata.setInActiveRcu(ant.y.rcu)
        logger.info("turned off antenna %d RCU(%d,%d)" %(ant.nr_pvss, ant.x.rcu, ant.y.rcu))
        rspctl("--rcumode=0 --select=%d,%d" %(ant.x.rcu, ant.y.rcu), wait=2.0)
        rspctl("--rcuenable=0 --select=%d,%d" %(ant.x.rcu, ant.y.rcu), wait=2.0)
        return

    def set_mode(self, mode):
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.lba.selectList())
            self.lba.resetRcuState()
            self.rcudata.reset()
        
    def record_data(self, rec_time, new_data=False):
        if new_data or self.rcudata.isActiveRcusChanged() or self.rcudata.getRecTime() < rec_time:
            logger.debug('record info changed')
            self.rcudata.resetActiveRcusChanged()
            self.rcudata.record(rec_time=rec_time)
        
    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    def checkOscillation(self, mode):
        logger.info("=== Start %s oscillation test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=28.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        clean = False
        while not clean:
            if self.db.checkEndTime(duration=18.0) == False:
                logger.warn("check stopped, end time reached")
                return

            clean = True
            self.record_data(rec_time=3)
            
            for pol_nr, pol in enumerate(('X', 'Y')):
                # result is a sorted list on maxvalue
                result = search_oscillation(data=self.rcudata, pol=pol, delta=6.0)
                if len(result) > 1:
                    clean = False
                    ant, peaks_sum, n_peaks, ant_low  = sorted(result[1:], reverse=True)[0] #result[1]
                    #ant = rcu / 2
                    #ant_polarity = rcu % 2
                    rcu = (ant * 2) + pol_nr
                    logger.info("RCU %d LBA %d Oscillation sum=%3.1f peaks=%d low=%3.1fdB" %\
                               (rcu, self.lba.ant[ant].nr_pvss, peaks_sum, n_peaks, ant_low))
                    self.turnOffAnt(ant)
                    if pol_nr == 0:
                        self.lba.ant[ant].x.osc = 1
                    else:
                        self.lba.ant[ant].y.osc = 1

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.oscillation_check_done = 1
        self.db.addTestDone('O%d' %(mode))
        logger.info("=== Done %s oscillation test ===" %(self.lba.label))
        return

    def checkNoise(self, mode, record_time, low_deviation, high_deviation, max_diff):
        logger.info("=== Start %s noise test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=(record_time+100.0)) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        for ant in self.lba.ant:
            if ant.x.rcu_off or ant.y.rcu_off:
                logger.info("skip low-noise test for antenna %d, RCUs turned off" %(ant.nr))
        
        self.record_data(rec_time=record_time)

        # result is a sorted list on maxvalue
        low_noise, high_noise, jitter = search_noise(self.rcudata, 'XY', low_deviation, high_deviation, max_diff)

        for n in low_noise:
            rcu, val, bad_secs, ref, diff = n
            ant = rcu / 2
            if self.lba.ant[ant].x.rcu_off or self.lba.ant[ant].y.rcu_off:
                continue
            #self.turnOffAnt(ant)
            logger.info("RCU %d Ant %d Low-Noise value=%3.1f bad=%d(%d) limit=%3.1f diff=%3.3f" %\
                       (rcu, self.lba.ant[ant].nr_pvss, val, bad_secs, self.rcudata.frames, ref, diff))
            
            self.rcudata.add_to_rcu_mask(rcu)
            if rcu%2 == 0:
                antenna = self.lba.ant[ant].x
            else:
                antenna = self.lba.ant[ant].y

            antenna.low_seconds     += self.rcudata.frames
            antenna.low_bad_seconds += bad_secs
            if val < self.lba.ant[ant].x.low_val:
                antenna.low_noise = 1
                antenna.low_val   = val
                antenna.low_ref   = ref
                antenna.low_diff  = diff

        for n in high_noise:
            rcu, val, bad_secs, ref, diff = n
            ant = rcu / 2
            #self.turnOffAnt(ant)
            logger.info("RCU %d Ant %d High-Noise value=%3.1f bad=%d(%d) ref=%3.1f diff=%3.1f" %\
                       (rcu, self.lba.ant[ant].nr_pvss, val, bad_secs, self.rcudata.frames, ref, diff))

            self.rcudata.add_to_rcu_mask(rcu)
            if rcu%2 == 0:
                antenna = self.lba.ant[ant].x
            else:
                antenna = self.lba.ant[ant].y

            antenna.high_seconds     += self.rcudata.frames
            antenna.high_bad_seconds += bad_secs
            if val > self.lba.ant[ant].x.high_val:
                antenna.high_noise = 1
                antenna.high_val   = val
                antenna.high_ref   = ref
                antenna.high_diff  = diff

        for n in jitter:
            rcu, val, ref, bad_secs = n
            ant = rcu / 2
            logger.info("RCU %d Ant %d Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %(rcu, self.lba.ant[ant].nr_pvss, val, ref))

            self.rcudata.add_to_rcu_mask(rcu)
            if rcu%2 == 0:
                antenna = self.lba.ant[ant].x
            else:
                antenna = self.lba.ant[ant].y

            antenna.jitter_seconds     += self.rcudata.frames
            antenna.jitter_bad_seconds += bad_secs
            if val > antenna.jitter_val:
                antenna.jitter     = 1
                antenna.jitter_val = val
                antenna.jitter_ref = ref

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.noise_check_done = 1
        self.db.addTestDone('NS%d=%d' %(mode, record_time))
        logger.info("=== Done %s noise test ===" %(self.lba.label))
        return

    def checkSpurious(self, mode):
        logger.info("=== Start %s spurious test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=12.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)
        self.record_data(rec_time=3)
            
        # result is a sorted list on maxvalue
        result = search_spurious(self.rcudata, 'XY', delta=3.0)
        for rcu in result:
            ant = rcu / 2
            ant_polarity  = rcu % 2
            #self. turnOffAnt(ant)
            logger.info("RCU %d Ant %d pol %d Spurious" %(rcu, self.lba.ant[ant].nr_pvss, ant_polarity))
            
            self.rcudata.add_to_rcu_mask(rcu)
            if ant_polarity == 0:
                self.lba.ant[ant].x.spurious = 1
            else:
                self.lba.ant[ant].y.spurious = 1

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.spurious_check_done = 1
        self.db.addTestDone('SP%d' %(mode))
        logger.info("=== Done %s spurious test ===" %(self.lba.label))
        return

    def checkShort(self, mode):
        logger.info("=== Start %s Short test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=15.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)
        self.record_data(rec_time=3)
            
        # search for shorted cable (input), mean signal all subbands between 55 and 61 dB
        logger.debug("Check Short")
        short = searchShort(self.rcudata)
        for i in short:
            rcu, mean_val = i
            ant = rcu / 2
            pol = rcu % 2
            
            logger.info("%s %2d RCU %3d Short, mean value band=%5.1fdB" %\
                       (self.lba.label, self.lba.ant[ant].nr_pvss, rcu, mean_val)) 
            
            self.rcudata.add_to_rcu_mask(rcu)
            if pol == 0:
                self.lba.ant[ant].x.short = 1;
                self.lba.ant[ant].x.short_val = mean_val;
            else:    
                self.lba.ant[ant].y.short = 1;
                self.lba.ant[ant].y.short_val = mean_val;

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.short_check_done = 1
        self.db.addTestDone('SH%d' %(mode))
        logger.info("=== Done %s Short test ===" %(self.lba.label))
        return

    def checkFlat(self, mode):
        logger.info("=== Start %s Flat test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=15.0) == False:
            logger.warn("check stopped, end time reached")
            return
            
        self.set_mode(mode)
        self.record_data(rec_time=3)
        
        # search for flatliners, mean signal all subbands between 63 and 65 dB
        logger.debug("Check Flat")
        flat = searchFlat(self.rcudata)
        for i in flat:
            rcu, mean_val = i
            ant = rcu / 2
            pol = rcu % 2
            
            logger.info("%s %2d RCU %3d Flat, mean value band=%5.1fdB" %\
                       (self.lba.label, self.lba.ant[ant].nr_pvss, rcu, mean_val)) 
            
            self.rcudata.add_to_rcu_mask(rcu)
            if pol == 0:
                self.lba.ant[ant].x.flat = 1;
                self.lba.ant[ant].x.flat_val = mean_val;
            else:    
                self.lba.ant[ant].y.flat = 1;
                self.lba.ant[ant].y.flat_val = mean_val;
                
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.flat_check_done = 1
        self.db.addTestDone('F%d' %(mode))
        logger.info("=== Done %s Flat test ===" %(self.lba.label))
        return
        
        
    def checkDown(self, mode, subband):
        logger.info("=== Start %s Down test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=15.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)
        self.record_data(rec_time=3)
        
        # mark lba as down if top of band is lower than normal and top is shifted more than 10 subbands to left or right
        logger.debug("Check Down")
        down, shifted = searchDown(self.rcudata, subband)
        for i in down:
            ant, max_x_sb, max_y_sb, mean_max_sb = i
            max_x_offset = max_x_sb - mean_max_sb
            max_y_offset = max_y_sb - mean_max_sb
            
            if self.lba.ant[ant].x.flat or self.lba.ant[ant].x.short or self.lba.ant[ant].y.flat or self.lba.ant[ant].y.short:
                continue
            
            self.lba.ant[ant].x.offset = max_x_offset
            self.lba.ant[ant].y.offset = max_y_offset
            self.lba.ant[ant].down = 1
            logger.info("%s %2d RCU %3d/%3d Down, offset-x=%d offset-y=%d" %\
                       (self.lba.label, self.lba.ant[ant].nr_pvss, self.lba.ant[ant].x.rcu, self.lba.ant[ant].y.rcu, max_x_offset, max_y_offset))
            self.rcudata.add_to_rcu_mask(self.lba.ant[ant].x.rcu)
            self.rcudata.add_to_rcu_mask(self.lba.ant[ant].y.rcu)
        for i in shifted:
            rcu, max_sb, mean_max_sb = i
            ant = rcu / 2
            logger.info("%s %2d RCU %3d shifted top on sb=%d, normal=sb%d" %(self.lba.label, self.lba.ant[ant].nr_pvss, rcu, max_sb, mean_max_sb))

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.down_check_done = 1
        self.db.addTestDone('D%d' %(mode))
        logger.info("=== Done %s Down test ===" %(self.lba.label))
        return
        
    
    def checkSignal(self, mode, subband, min_signal, low_deviation, high_deviation):
        logger.info("=== Start %s RF test ===" %(self.lba.label))
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=15.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)
        self.record_data(rec_time=3)

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
        
        logger.debug("Check RF signal")
        for ant in self.lba.ant:
            ant.x.test_signal = ssdataX[ant.nr]
            ant.y.test_signal = ssdataY[ant.nr]
            
            loginfo = False
            if ssdataX[ant.nr] < (medianValX + low_deviation):
                if not max(ant.x.flat, ant.x.short, ant.down):
                    ant.x.too_low = 1
                if ssdataX[ant.nr] < 2.0:
                    ant.x.rcu_error = 1
                loginfo = True

            if ssdataX[ant.nr] > (medianValX + high_deviation):
                ant.x.too_high = 1
                loginfo = True

            if ssdataY[ant.nr] < (medianValY + low_deviation):
                if not max(ant.y.flat, ant.y.short, ant.down):
                    ant.y.too_low = 1
                if ssdataY[ant.nr] < 2.0:
                    ant.y.rcu_error = 1
                loginfo = True

            if ssdataY[ant.nr] > (medianValY + high_deviation):
                ant.y.too_high = 1
                loginfo = True

            if loginfo:
                logger.info("%s %2d  RCU %3d/%3d   X=%5.1fdB  Y=%5.1fdB" %(self.lba.label, ant.nr_pvss, ant.x.rcu, ant.y.rcu, ssdataX[ant.nr], ssdataY[ant.nr]))

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.lba.signal_check_done = 1
        self.db.addTestDone('S%d' %(mode))
        logger.info("=== Done %s RF test ===" %(self.lba.label))
        return
#### end of cLBA class ####


# class for testing HBA antennas
class cHBA:
    #global logger
    def __init__(self, db, hba):
        self.db  = db
        self.hba = hba
        self.rcudata = cRCUdata(hba.nr_tiles*2)
        self.rcudata.setActiveRcus(self.hba.selectList())
        self.rcumode = 0

    def reset(self):
        self.rcudata.reset()
        self.rcudata.reset_masks()
        self.rcudata.setActiveRcus(self.hba.selectList())
        
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

    def set_mode(self, mode):
        if self.db.rcumode != mode:
            self.db.rcumode = mode
            turnoffRCUs()
            turnonRCUs(mode=mode, rcus=self.hba.selectList())
            self.hba.resetRcuState()
            self.rcudata.reset()
        
    def record_data(self, rec_time, new_data=False):
        if new_data or self.rcudata.isActiveRcusChanged() or self.rcudata.getRecTime() < rec_time:
            logger.debug('record info changed')
            self.rcudata.resetActiveRcusChanged()
            self.rcudata.record(rec_time=rec_time)

    def checkModem(self, mode):
        # setup internal test db
        n_elements = 16
        n_tests    = 7
        modem_tst = list()
        for tile_nr in range(self.db.nr_hba):
            tile = list()
            for elem_nr in range(n_elements):
                test = list()
                for tst_nr in range(n_tests):
                    test.append([0, 0])
                tile.append(test)
            modem_tst.append(tile)
        # done

        logger.info("=== Start HBA modem test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=50.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        time.sleep(4.0)
        ctrlstr = list()
        ctrlstr.append(('129,'* 16)[:-1]) # 0ns
        ctrlstr.append(('133,'* 16)[:-1]) # 0.5ns
        ctrlstr.append(('137,'* 16)[:-1]) # 1ns
        ctrlstr.append(('145,'* 16)[:-1]) # 2ns
        ctrlstr.append(('161,'* 16)[:-1]) # 4ns
        ctrlstr.append(('193,'* 16)[:-1]) # 8ns
        ctrlstr.append(('253,'* 16)[:-1]) # 15.5ns
        #rsp_hba_delay(delay=ctrlstr[6], rcus=self.hba.selectList(), discharge=False)
        tst_nr = 0
        for ctrl in ctrlstr:

            rsp_hba_delay(delay=ctrl, rcus=self.hba.selectList(), discharge=False)
            data = rspctl('--realdelays', wait=1.0).splitlines()

            ctrllist = ctrl.split(',')
            for line in data:
                if line[:3] == 'HBA':
                    rcu = int(line[line.find('[')+1:line.find(']')])
                    hba_nr = rcu / 2
                    if self.hba.tile[hba_nr].on_bad_list:
                        continue
                    ant_polarity = rcu % 2
                    realctrllist = line[line.find('=')+1:].strip().split()
                    for elem in self.hba.tile[hba_nr].element:
                        if ctrllist[elem.nr-1] != realctrllist[elem.nr-1]:
                            logger.info("Modemtest Tile=%d RCU=%d Element=%d ctrlword=%s response=%s" %\
                                         (hba_nr, rcu, elem.nr, ctrllist[elem.nr-1], realctrllist[elem.nr-1]))

                            if realctrllist[elem.nr-1].count('?') == 3:
                                #elem.no_modem += 1
                                modem_tst[hba_nr][elem.nr-1][tst_nr][0] = 1
                            else:
                                #elem.modem_error += 1
                                modem_tst[hba_nr][elem.nr-1][tst_nr][1] = 1
            tst_nr += 1
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        # analyse test results and add to DB
        no_modem    = dict()
        modem_error = dict()
        for tile_nr in range(self.db.nr_hba):
            n_no_modem    = dict()
            n_modem_error = dict()
            for elem_nr in range(n_elements):
                n_no_modem[elem_nr] = 0
                n_modem_error[elem_nr] = 0
                for tst_nr in range(n_tests):
                    if modem_tst[tile_nr][elem_nr][tst_nr][0]:
                        n_no_modem[elem_nr] += 1
                    if modem_tst[tile_nr][elem_nr][tst_nr][1]:
                        n_modem_error[elem_nr] += 1
                no_modem[tile_nr]  = n_no_modem
                modem_error[tile_nr] = n_modem_error

        n_tile_err = 0
        for tile in no_modem:
            n_elem_err = 0
            for elem in no_modem[tile]:
                if no_modem[tile][elem] == n_tests:
                    n_elem_err += 1
            if n_elem_err == n_elements:
                n_tile_err += 1

        if n_tile_err < (self.db.nr_hba / 2):
            for tile_nr in range(self.db.nr_hba):
                for elem_nr in range(n_elements):
                    if no_modem[tile_nr][elem_nr] >= 2: # 2 or more ctrl values went wrong
                        self.db.hba.tile[tile_nr].element[elem_nr].no_modem = 1

        n_tile_err = 0
        for tile in modem_error:
            n_elem_err = 0
            for elem in modem_error[tile]:
                if modem_error[tile][elem] == n_tests:
                    n_elem_err += 1
            if n_elem_err == n_elements:
                n_tile_err += 1

        if n_tile_err < (self.db.nr_hba / 2):
            for tile_nr in range(self.db.nr_hba):
                for elem_nr in range(n_elements):
                    if no_modem[tile_nr][elem_nr] >= 2: # 2 or more ctrl values went wrong
                        self.db.hba.tile[tile_nr].element[elem_nr].modem_error = 1

        self.hba.modem_check_done = 1
        self.db.addTestDone('M%d' % mode)
        logger.info("=== Done HBA modem test ===")
        #self.db.rcumode = 0
        return

    # check for summator noise and turn off RCU
    def checkSummatorNoise(self, mode):
        logger.info("=== Start HBA tile based summator-noise test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=25.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        delay_str = ('253,'* 16)[:-1]
        rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())
        
        self.record_data(rec_time=12)

        for pol_nr, pol in enumerate(('X', 'Y')):
            sum_noise, cable_reflection = search_summator_noise(data=self.rcudata, pol=pol, min_peak=0.8)
            for n in sum_noise:
                bin_nr, cnt, n_peaks = n
                tile = bin_nr
                logger.info("RCU %d Tile %d Summator-Noise cnt=%3.1f peaks=%3.1f" %(self.hba.tile[tile].x.rcu, tile, cnt, n_peaks))
                if pol == 'X':
                    self.hba.tile[tile].x.summator_noise = 1
                else:
                    self.hba.tile[tile].y.summator_noise = 1
                self.turnOffTile(tile)

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.summatornoise_check_done = 1
        self.db.addTestDone('SN%d' %(mode))
        logger.info("=== Done HBA tile based summator-noise test ===")
        return

    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    def checkOscillation(self, mode):
        logger.info("=== Start HBA tile based oscillation test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=35.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        delay_str = ('253,'* 16)[:-1]
        get_new_data = rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())

        clean = False
        while not clean:
            if self.db.checkEndTime(duration=25.0) == False:
                logger.warn("check stopped, end time reached")
                return

            clean = True
            
            self.record_data(rec_time=12, new_data=get_new_data)
            
            for pol_nr, pol in enumerate(('X', 'Y')):
                # result is a sorted list on maxvalue
                result = search_oscillation(data=self.rcudata, pol=pol, delta=6.0) # start_sb=45, stop_sb=350
                if len(result) > 1:
                    if len(result) == 2:
                        tile, max_sum, n_peaks, rcu_low = result[1]
                    else:
                        ref_low = result[0][3]
                        max_low_tile = (-1, -1)
                        max_sum_tile = (-1, -1)
                        for i in result[1:]:
                            tile, max_sum, n_peaks, tile_low = i
                            #rcu = (tile * 2) + pol_nr
                            if max_sum > max_sum_tile[0]:
                                max_sum_tile = (max_sum, tile)
                            if (tile_low - ref_low) > max_low_tile[0]:
                                max_low_tile = (tile_low, tile)

                        rcu_low, tile = max_low_tile
                    
                    clean = False
                    get_new_data = True
                    #tile = rcu / 2
                    #tile_polarity  = rcu % 2
                    rcu = (tile * 2) + pol_nr
                    logger.info("RCU %d Tile %d Oscillation sum=%3.1f peaks=%d low=%3.1f" %\
                               (rcu, tile, max_sum, n_peaks, tile_low))
                    self.turnOffTile(tile)
                    if pol_nr == 0:
                        self.hba.tile[tile].x.osc = 1
                    else:
                        self.hba.tile[tile].y.osc = 1

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.oscillation_check_done = 1
        self.db.addTestDone('O%d' %(mode))
        logger.info("=== Done HBA tile based oscillation test ===")
        return

    def checkNoise(self, mode, record_time, low_deviation, high_deviation, max_diff):
        logger.info("=== Start HBA tile based noise test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=(record_time+60.0)) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip low-noise test for tile %d, RCUs turned off" %(tile.nr))

        delay_str = ('253,'* 16)[:-1]
        get_new_data = rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())
        
        self.record_data(rec_time=record_time, new_data=get_new_data)

        for pol_nr, pol in enumerate(('X', 'Y')):
            # result is a sorted list on maxvalue
            low_noise, high_noise, jitter = search_noise(self.rcudata, pol, low_deviation, high_deviation, max_diff)

            for n in low_noise:
                bin_nr, val, bad_secs, ref, diff = n
                tile = bin_nr
                rcu = (tile * 2) + pol_nr
                if self.hba.tile[tile].x.rcu_off or self.hba.tile[tile].y.rcu_off:
                    continue
                logger.info("RCU %d Tile %d Low-Noise value=%3.1f bad=%d(%d) limit=%3.1f diff=%3.3f" %\
                           (rcu, tile, val, bad_secs, self.rcudata.frames, ref, diff))

                if pol == 'X':
                    tile_polarity = self.hba.tile[tile].x
                else:
                    tile_polarity = self.hba.tile[tile].y

                tile_polarity.low_seconds     += self.rcudata.frames
                tile_polarity.low_bad_seconds += bad_secs
                if val < tile_polarity.low_val:
                    tile_polarity.low_noise = 1
                    tile_polarity.low_val   = val
                    tile_polarity.low_ref   = ref
                    tile_polarity.low_diff  = diff

            for n in high_noise:
                bin_nr, val, bad_secs, ref, diff = n
                tile = bin_nr
                rcu = (tile * 2) + pol_nr
                logger.info("RCU %d Tile %d High-Noise value=%3.1f bad=%d(%d) limit=%3.1f diff=%3.1f" %\
                           (rcu, tile, val, bad_secs, self.rcudata.frames, ref, diff))

                if pol == 'X':
                    tile_polarity = self.hba.tile[tile].x
                else:
                    tile_polarity = self.hba.tile[tile].y

                tile_polarity.high_seconds     += self.rcudata.frames
                tile_polarity.high_bad_seconds += bad_secs
                if val > tile_polarity.high_val:
                    tile_polarity.high_noise = 1
                    tile_polarity.high_val   = val
                    tile_polarity.high_ref   = ref
                    tile_polarity.high_diff  = diff

            for n in jitter:
                bin_nr, val, ref, bad_secs = n
                tile = bin_nr
                rcu = (tile * 2) + pol_nr
                logger.info("RCU %d Tile %d Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %(rcu, tile, val, ref))

                if pol == 'X':
                    tile_polarity = self.hba.tile[tile].x
                else:
                    tile_polarity = self.hba.tile[tile].y

                tile_polarity.jitter_seconds     += self.rcudata.frames
                tile_polarity.jitter_bad_seconds += bad_secs
                if val > tile_polarity.jitter_val:
                    tile_polarity.jitter     = 1
                    tile_polarity.jitter_val = val
                    tile_polarity.jitter_ref = ref

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.noise_check_done = 1
        self.db.addTestDone('NS%d=%d' %(mode, record_time))
        logger.info("=== Done HBA tile based noise test ===")
        return

    def checkSpurious(self, mode):
        logger.info("=== Start HBA tile based spurious test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=12.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        delay_str = ('253,'* 16)[:-1]
        get_new_data = rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())
        
        self.record_data(rec_time=12, new_data=get_new_data)

        for pol_nr, pol in enumerate(('X', 'Y')):
            # result is a sorted list on maxvalue
            result = search_spurious(self.rcudata, pol, delta=3.0)
            for bin_nr in result:
                tile = bin_nr
                rcu = (tile * 2) + pol_nr
                logger.info("RCU %d Tile %d pol %c Spurious" %(rcu, tile, pol))
                if pol == 'X':
                    self.hba.tile[tile].x.spurious = 1
                else:
                    self.hba.tile[tile].y.spurious = 1

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.spurious_check_done = 1
        self.db.addTestDone('SP%d' %(mode))
        logger.info("=== Done HBA spurious test ===")
        return

    def checkSignal(self, mode, subband, min_signal, low_deviation, high_deviation):
        logger.info("=== Start HBA tile based RF test ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return

        if self.db.checkEndTime(duration=37.0) == False:
            logger.warn("check stopped, end time reached")
            return

        self.set_mode(mode)

        # check twice
        # 2   ... check if all elements are turned off, normal value between 60.0 and 62.0
        # 128 ...
        # 253 ...
        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip signal test for tile %d, RCUs turned off" %(tile.nr))

        logger.info("start test")
        for ctrl in ('2,', '128,', '253,'):
            if self.db.checkEndTime(duration=20.0) == False:
                logger.warn("check stopped, end time reached")
                return

            if ctrl == '128,': ctrl_nr = 0
            elif ctrl == '253,': ctrl_nr = 1

            logger.info("HBA signal test, ctrl word %s" %(ctrl[:-1]))

            delay_str = (ctrl*16)[:-1]
            rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())
            
            self.record_data(rec_time=2, new_data=True)
            self.rcudata.searchTestSignal(subband=subband, minsignal=min_signal, maxsignal=150.0)
            logger.info("HBA, X used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_X, self.rcudata.testSignal_X))
            logger.info("HBA, Y used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_Y, self.rcudata.testSignal_Y))
            
            if ctrl == '2,':
                ssdataX = self.rcudata.getSubbandX(subband=subband)
                ssdataY = self.rcudata.getSubbandY(subband=subband)
            else:
                ssdataX = self.rcudata.getSubbandX()
                ssdataY = self.rcudata.getSubbandY()
                
            for tile in self.hba.tile:
                if tile.x.rcu_off or tile.y.rcu_off:
                    continue
                logger.debug("HBA Tile=%d :  X=%3.1fdB  Y=%3.1fdB" %\
                            (tile.nr, ssdataX[tile.nr], ssdataY[tile.nr]))
            if ctrl == '2,':
                continue
            
            if (self.rcudata.testSignal_X != -1) and (self.rcudata.testSignal_Y != -1):

                self.hba.ref_signal_x[ctrl_nr]   = self.rcudata.testSignal_X
                self.hba.ref_signal_y[ctrl_nr]   = self.rcudata.testSignal_Y
                self.hba.test_subband_x[ctrl_nr] = self.rcudata.testSubband_X
                self.hba.test_subband_y[ctrl_nr] = self.rcudata.testSubband_Y
                
                avgX = self.rcudata.testSignal_X
                avgY = self.rcudata.testSignal_Y
                minX = ssdataX.min()
                minY = ssdataY.min()

                # if all elements in range
                #if minX < (avgX + self.min_dB) and minY < (avgY + self.min_dB):
                #    continue

                logger.debug("X data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minX, ssdataX.max(), avgX))
                logger.debug("Y data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minY, ssdataY.max(), avgY))

                for tile in self.hba.tile:
                    if tile.x.rcu_off or tile.y.rcu_off:
                        continue

                    tile.x.test_signal[ctrl_nr] = ssdataX[tile.nr]
                    tile.y.test_signal[ctrl_nr] = ssdataY[tile.nr]

                    loginfo = False
                    if ssdataX[tile.nr] < (avgX + low_deviation):
                        if ssdataX[tile.nr] < 2.0:
                            tile.x.no_signal = 1
                        elif ssdataX[tile.nr] > 55.0 and ssdataX[tile.nr] < 65.0:
                            tile.no_power = 1
                        else:
                            tile.x.too_low = 1
                        loginfo = True

                    if ssdataX[tile.nr] > (avgX + high_deviation):
                        tile.x.too_high = 1
                        loginfo = True

                    if ssdataY[tile.nr] < (avgY + low_deviation):
                        if ssdataY[tile.nr] < 2.0:
                            tile.y.no_signal = 1
                        elif ssdataY[tile.nr] > 55.0 and ssdataY[tile.nr] < 65.0:
                            tile.no_power = 1
                        else:
                            tile.y.too_low = 1
                        loginfo = True

                    if ssdataY[tile.nr] > (avgY + high_deviation):
                        tile.y.too_high = 1
                        loginfo = True

                    if loginfo:
                        logger.info("HBA Tile=%d  Error:  X=%3.1fdB  Y=%3.1fdB" %\
                                     (tile.nr, ssdataX[tile.nr], ssdataY[tile.nr]))
            else:
                logger.warn("HBA, No valid test signal")

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.signal_check_done = 1
        self.db.addTestDone('S%d' %(mode))
        logger.info("=== Done HBA signal test ===")
        return

    # Next tests are element based
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
    # control word = 0 (signal - 30 db)
    # control word = 2 (signal - 40 db)
    #
    def checkElements(self, mode, record_time, subband,
                      noise_low_deviation, noise_high_deviation, noise_max_diff,
                      rf_min_signal, rf_low_deviation, rf_high_deviation):

        logger.info("=== Start HBA element based tests ===")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        
        self.set_mode(mode)

        n_rcus_off  = 0
        for ctrl in ('128', '253'):
            if ctrl == '128': ctrl_nr = 0
            elif ctrl == '253': ctrl_nr = 1

            for elem in range(self.hba.tile[0].nr_elements):
                logger.info("check elements %d, ctrlword=%s" %(elem+1, ctrl))

                if self.db.checkEndTime(duration=45.0) == False:
                    logger.warn("check stopped, end time reached")
                    return

                if n_rcus_off > 0:
                    rsp_rcu_mode(mode=mode, rcus=self.hba.selectList())
                    n_rcus_off = 0
                for tile in self.hba.tile:
                    if tile.element[elem].no_modem or tile.element[elem].modem_error:
                        self.turnOffTile(tile.nr)
                        n_rcus_off += 1
                        logger.info("skip tile %d, modem error" %(tile.nr))

                delay_str = ('2,'*elem + ctrl + ',' + '2,'*15)[:33]
                rsp_hba_delay(delay=delay_str, rcus=self.hba.selectList())

                clean = False
                while not clean:
                    if self.db.checkEndTime(duration=(record_time+45.0)) == False:
                        logger.warn("check stopped, end time reached")
                        return

                    clean = True
                    self.record_data(rec_time=record_time, new_data=True)

                    clean, n_off = self.checkOscillationElements(elem)
                    n_rcus_off += n_off
                    if n_off > 0: continue
                    n_off = self.checkSpuriousElements(elem)
                    n_rcus_off += n_off
                    if n_off > 0: continue
                    self.checkNoiseElements(elem, noise_low_deviation, noise_high_deviation, noise_max_diff)
                    self.checkSignalElements(elem, ctrl_nr, subband, rf_min_signal, rf_low_deviation, rf_high_deviation)

        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down while testing, skip result")
            return

        self.hba.element_check_done = 1
        self.db.addTestDone('E%d' %(mode))
        logger.info("=== Done HBA element tests ===")
        return

    # check for oscillating tiles and turn off RCU
    # stop one RCU each run
    # elem counts from 0..15 (for user output use 1..16)
    def checkOscillationElements(self, elem):
        logger.info("--- oscillation test --")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        clean = True
        n_rcus_off = 0
        # result is a sorted list on maxvalue
        result = search_oscillation(data=self.rcudata, pol='XY', delta=3.0)
        if len(result) > 1:
            clean = False
            rcu, peaks_sum, n_peaks, rcu_low  = sorted(result[1:], reverse=True)[0] #result[1]
            tile = rcu / 2
            if self.hba.tile[tile].element[elem].no_modem or self.hba.tile[tile].element[elem].modem_error:
                return(True, 0)
            tile_polarity  = rcu % 2
            logger.info("RCU %d Tile %d Element %d Oscillation sum=%3.1f peaks=%d, low=%3.1f" %\
                       (rcu, tile, elem+1, peaks_sum, n_peaks, rcu_low))
            self.turnOffTile(tile)
            n_rcus_off += 1
            if tile_polarity == 0:
                self.hba.tile[tile].element[elem].x.osc = 1
            else:
                self.hba.tile[tile].element[elem].y.osc = 1
        return (clean, n_rcus_off)

    def checkSpuriousElements(self, elem):
        logger.info("--- spurious test ---")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        n_rcus_off = 0
        # result is a sorted list on maxvalue
        result = search_spurious(data=self.rcudata, pol='XY', delta=3.0)
        for rcu in result:
            tile = rcu / 2
            tile_polarity  = rcu % 2
            logger.info("RCU %d Tile %d Element %d pol %d Spurious" %(rcu, tile, elem+1, tile_polarity))
            self.turnOffTile(tile)
            n_rcus_off += 1
            if tile_polarity == 0:
                self.hba.tile[tile].element[elem].x.spurious = 1
            else:
                self.hba.tile[tile].element[elem].y.spurious = 1
        return (n_rcus_off)

    def checkNoiseElements(self, elem, low_deviation, high_deviation, max_diff):
        logger.info("--- noise test ---")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        # result is a sorted list on maxvalue
        low_noise, high_noise, jitter = search_noise(self.rcudata, 'XY', low_deviation, high_deviation, max_diff)

        for n in low_noise:
            rcu, val, bad_secs, ref, diff = n
            tile = rcu / 2
            logger.info("RCU %d Tile %d Element %d Low-Noise value=%3.1f bad=%d(%d) limit=%3.1f diff=%3.3f" %\
                       (rcu, tile, elem+1, val, bad_secs, self.rcudata.frames, ref, diff))

            if rcu%2 == 0:
                elem_polarity = self.hba.tile[tile].element[elem].x
            else:
                elem_polarity = self.hba.tile[tile].element[elem].y

            elem_polarity.low_seconds     += self.rcudata.frames
            elem_polarity.low_bad_seconds += bad_secs
            if val < elem_polarity.low_val:
                elem_polarity.low_noise = 1
                elem_polarity.low_val   = val
                elem_polarity.low_ref   = ref
                elem_polarity.low_diff  = diff

        for n in high_noise:
            rcu, val, bad_secs, ref, diff = n
            tile = rcu / 2
            logger.info("RCU %d Tile %d Element %d High-Noise value=%3.1f bad=%d(%d) ref=%3.1f diff=%3.1f" %\
                       (rcu, tile, elem+1, val, bad_secs, self.rcudata.frames, ref, diff))

            if rcu%2 == 0:
                elem_polarity = self.hba.tile[tile].element[elem].x
            else:
                elem_polarity = self.hba.tile[tile].element[elem].y

            elem_polarity.high_seconds     += self.rcudata.frames
            elem_polarity.high_bad_seconds += bad_secs
            if val > elem_polarity.high_val:
                elem_polarity.high_noise = 1
                elem_polarity.high_val   = val
                elem_polarity.high_ref   = ref
                elem_polarity.high_diff  = diff

        for n in jitter:
            rcu, val, ref, bad_secs = n
            tile = rcu / 2
            logger.info("RCU %d Tile %d Element %d Jitter, fluctuation=%3.1fdB  normal=%3.1fdB" %\
                       (rcu, tile, elem+1, val, ref))

            if rcu%2 == 0:
                elem_polarity = self.hba.tile[tile].element[elem].x
            else:
                elem_polarity = self.hba.tile[tile].element[elem].y

            elem_polarity.jitter_seconds     += self.rcudata.frames
            elem_polarity.jitter_bad_seconds += bad_secs
            if val > elem_polarity.jitter_val:
                elem_polarity.jitter     = 1
                elem_polarity.jitter_val = val
                elem_polarity.jitter_ref = ref
        return

    def checkSignalElements(self, elem, ctrl_nr, subband, min_signal, low_deviation, high_deviation):

        logger.info("--- RF test ---")
        if not checkActiveRSPDriver():
            logger.warn("RSPDriver down, skip test")
            return
        self.rcudata.searchTestSignal(subband=subband, minsignal=min_signal, maxsignal=120.0)
        logger.info("HBA, X used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_X, self.rcudata.testSignal_X))
        logger.info("HBA, Y used test subband=%d  avg_signal=%3.1f" %(self.rcudata.testSubband_Y, self.rcudata.testSignal_Y))

        ssdataX = self.rcudata.getSubbandX()
        ssdataY = self.rcudata.getSubbandY()
        avgX = self.rcudata.testSignal_X
        avgY = self.rcudata.testSignal_Y
        minX = ssdataX.min()
        minY = ssdataY.min()

        logger.debug("X data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minX, ssdataX.max(), avgX))
        logger.debug("Y data:  min=%5.3f  max=%5.3f  avg=%5.3f" %(minY, ssdataY.max(), avgY))

        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                logger.info("skip signal test for tile %d, RCUs are turned off" %(tile.nr))

        if self.rcudata.testSubband_X == 0 or self.rcudata.testSubband_Y == 0:
            logger.warn("HBA, No valid test signal")
            for tile in self.hba.tile:
                tile.element[elem].x.ref_signal[ctrl_nr] = 0
                tile.element[elem].y.ref_signal[ctrl_nr] = 0
            return

        for tile in self.hba.tile:
            if tile.x.rcu_off or tile.y.rcu_off:
                continue
            tile.element[elem].x.ref_signal[ctrl_nr] = avgX
            tile.element[elem].y.ref_signal[ctrl_nr] = avgY
            tile.element[elem].x.test_subband[ctrl_nr] = self.rcudata.testSubband_X
            tile.element[elem].y.test_subband[ctrl_nr] = self.rcudata.testSubband_Y
            tile.element[elem].x.test_signal[ctrl_nr] = ssdataX[tile.nr]
            tile.element[elem].y.test_signal[ctrl_nr] = ssdataY[tile.nr]

            #logger.debug("HBA Tile=%d  Element=%d:  X=%3.1fdB  Y=%3.1fdB" %\
            #            (tile.nr, elem+1, ssdataX[tile.nr], ssdataY[tile.nr]))

            loginfo = False
            if ssdataX[tile.nr] < (avgX + low_deviation):
                if ssdataX[tile.nr] < 2.0:
                    tile.element[elem].x.no_signal = 1
                elif ssdataX[tile.nr] > 55.0 and ssdataX[tile.nr] < 65.0:
                    tile.element[elem].no_power = 1
                else:
                    tile.element[elem].x.too_low = 1
                loginfo = True

            if ssdataX[tile.nr] > (avgX + high_deviation):
                tile.element[elem].x.too_high = 1
                loginfo = True

            if ssdataY[tile.nr] < (avgY + low_deviation):
                if ssdataY[tile.nr] < 2.0:
                    tile.element[elem].y.no_signal = 1
                elif ssdataY[tile.nr] > 55.0 and ssdataY[tile.nr] < 65.0:
                    tile.element[elem].no_power = 1
                else:
                    tile.element[elem].y.too_low = 1
                loginfo = True

            if ssdataY[tile.nr] > (avgY + high_deviation):
                tile.element[elem].y.too_high = 1
                loginfo = True

            if loginfo:
                logger.info("HBA Tile=%d  Element=%d Error:  X=%3.1fdB  Y=%3.1fdB" %\
                             (tile.nr, elem+1, ssdataX[tile.nr], ssdataY[tile.nr]))
        return
#### end of cHBA class ####


