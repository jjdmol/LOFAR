#!/usr/bin/python

from copy import deepcopy
from general_lib import *
from lofar_lib import *
import logging

db_version = '1013c'

logger = None
def init_test_db():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger test_db")

class cDB:
    global logger
    def __init__(self, StID, nRSP, nTBB, nLBL, nLBH, nHBA):
        self.StID   = StID
        self.nr_rsp = nRSP
        self.nr_rcu = nRSP * 8
        self.nr_lbl = nLBL
        self.nr_lbh = nLBH
        self.nr_hba = nHBA
        self.nr_tbb = nTBB
        
        self.script_versions = ''
        
        self.board_errors = list()
        self.rcumode = -1
        self.tests   = ''
        self.check_start_time = 0
        self.check_stop_time  = 0
        self.rsp_driver_down  = False 
        self.tbb_driver_down  = False 
        
        self.station_error     = 0
        self.rspdriver_version = "ok"
        self.rspctl_version    = "ok"
        self.tbbdriver_version = "ok"
        self.tbbctl_version    = "ok"
        
        
        self.rsp = list()
        for i in range(nRSP):
            self.rsp.append(self.cRSP(i))
        
        self.tbb = list()
        for i in range(nTBB):
            self.tbb.append(self.cTBB(i))
        
        self.rcu_state = list()
        for i in range(self.nr_rcu):
            self.rcu_state.append(0)
        
        self.lbl = deepcopy(self.cLBA_DB(label='LBL', nr_antennas=nLBL, nr_offset=48))
        self.lbh = deepcopy(self.cLBA_DB(label='LBH', nr_antennas=nLBH, nr_offset=0))
        self.hba = deepcopy(self.cHBA_DB(nr_tiles=nHBA))
    
    # add only ones
    def addTestDone(self, name):
        if self.tests.find(name) == -1:
            self.tests += ','
            self.tests += name
    
    # check if already done
    def isTestDone(self, name):
        if self.tests.find(name) == -1:
            return (False)
        return (True)
        
    # test
    def test(self, logdir):
        if self.rspdriver_version != "ok" or self.rspctl_version != "ok":
            self.station_error = 1
            
        if self.tbbdriver_version != "ok" or self.tbbctl_version != "ok":
            self.station_error = 1
            
        for _rsp in self.rsp:
            ok = _rsp.test()
            if not ok:
                self.station_error = 1
        
        for _tbb in self.tbb:
            ok = _tbb.test()
            if not ok:
                self.station_error = 1
        
        # test rcu's first
        for _rcu in range(self.nr_rcu):
            error_count = 0
            
            ant_nr = _rcu / 2
            pol_nr = _rcu % 2  # 0=X, 1=Y
            
            if pol_nr == 0:
                if self.nr_lbl > 0 and self.lbl.ant[ant_nr].x.error: error_count += 1
                if self.lbh.ant[ant_nr].x.error: error_count += 1
                if self.hba.tile[ant_nr].x.rcu_error: error_count += 1
            else:
                if self.nr_lbl > 0 and self.lbl.ant[ant_nr].y.error: error_count += 1
                if self.lbh.ant[ant_nr].y.error: error_count += 1
                if self.hba.tile[ant_nr].y.rcu_error: error_count += 1
                    
            if error_count >= 2:
                self.rcu_state[_rcu] = 1
           
        self.station_error = max(self.station_error, self.lbl.test(), self.lbh.test(), self.hba.test())
        
        self.makeLogFile(logdir)
        return (self.station_error)
    
    
    # make standard log file
    def makeLogFile(self, logdir):
        #print logdir
        date = getShortDateStr(self.check_start_time)
        log = cTestLogger(logdir)
        
        log.addLine("%s,NFO,---,VERSIONS,%s" %(date, self.script_versions))
        
        log.addLine("%s,NFO,---,STATION,NAME=%s" %(date, getHostName()))   
        
        log.addLine("%s,NFO,---,RUNTIME,START=%s,STOP=%s" %(date, getDateTimeStr(self.check_start_time), getDateTimeStr(self.check_stop_time)))
        
        info = ""
        bad = ""
        for ant in self.lbl.ant:
            if ant.on_bad_list == 1:
                bad += "%d " %(ant.nr_pvss)
        if len(bad) > 0:
            info += "LBL=%s," %(bad[:-1])
        
        bad = ""
        for ant in self.lbh.ant:
            if ant.on_bad_list == 1:
                bad += "%d " %(ant.nr_pvss)        
        if len(bad) > 0:
            info += "LBH=%s," %(bad[:-1])
            
        bad = ""
        for tile in self.hba.tile:
            if tile.on_bad_list == 1:
                bad += "%d " %(tile.nr)    
        if len(bad) > 0:
            info += "HBA=%s," %(bad[:-1])
        if len(info) > 0:
            log.addLine("%s,NFO,---,BADLIST,%s" %(date, info[:-1]))
        
        if self.rsp_driver_down:
            log.addLine("%s,NFO,---,DRIVER,RSPDRIVER=DOWN" %(date))
        
        if self.tbb_driver_down:
            log.addLine("%s,NFO,---,DRIVER,TBBDRIVER=DOWN" %(date))
        
        if len(self.board_errors):
            boardstr = ''
            for board in self.board_errors:
                boardstr += "RSP-%d=DOWN," %(board)
            log.addLine("%s,NFO,---,BOARD,%s" %(date, boardstr[:-1]))
        
        log.addLine("%s,NFO,---,CHECKS%s" %(date, self.tests))
        
        log.addLine("%s,NFO,---,STATISTICS,BAD_LBL=%d,BAD_LBH=%d,BAD_HBA=%d" %(date, self.lbl.nr_bad_antennas, self.lbh.nr_bad_antennas, self.hba.nr_bad_tiles))
        
        
        if self.rspdriver_version != "ok" or self.rspctl_version != "ok":
            log.addLine("%s,RSP,---,VERSION,RSPDRIVER=%s,RSPCTL=%s" %\
                       (date, self.rspdriver_version, self.rspctl_version))
        
        for rsp in self.rsp:
            rsp.test()
            if not rsp.version_ok:
                log.addLine("%s,RSP,%03d,VERSION,BP=%s,AP=%s" %\
                           (date, rsp.nr, rsp.bp_version, rsp.ap_version))
        
        if self.tbbdriver_version != "ok" or self.tbbctl_version != "ok":
            log.addLine("%s,TBB,---,VERSION,TBBDRIVER=%s,TBBCTL=%s" %\
                       (date, self.tbbdriver_version, self.tbbctl_version))
        
        for tbb in self.tbb:
            tbb.test()
            if not tbb.version_ok:
                log.addLine("%s,TBB,%03d,VERSION,TP=%s,MP=%s" %\
                           (date, tbb.nr, tbb.tp_version, tbb.mp_version))
            if not tbb.memory_ok:
                log.addLine("%s,TBB,%03d,MEMORY" %(date, tbb.nr))
                           
        for rcu in range(self.nr_rcu):
            if self.rcu_state[rcu]:
                log.addLine("%s,RCU,%03d,BROKEN" % (date, rcu))
        
        # lbl/lbh        
        for lba in (self.lbl, self.lbh):
            
            if lba.signal_check_done:
                if lba.test_signal_x == 0 or lba.test_signal_y == 0:
                    log.addLine("%s,%s,---,NOSIGNAL" %(date, lba.label))
    
                elif lba.avg_2_low:
                    log.addLine("%s,%s,---,TOOLOW ,AVGX=%3.1f,AVGY=%3.1f" %(date, lba.label, lba.avg_x, lba.avg_y))
    
                else:
                    if lba.error:
                        log.addLine("%s,%s,---,TESTSIGNAL,SUBBANDX=%d,SIGNALX=%3.1f,SUBBANDY=%d,SIGNALY=%3.1f" %\
                                   (date, lba.label, lba.test_subband_x, lba.test_signal_x, lba.test_subband_y, lba.test_signal_y))
                   
             
            if lba.noise_check_done or lba.oscillation_check_done or lba.spurious_check_done or lba.signal_check_done:                               
                for ant in lba.ant:
                    if ant.down:
                            log.addLine("%s,%s,%03d,DOWN  ,X=%3.1f,Y=%3.1f,Xoff=%d,Yoff=%d" %\
                                       (date, lba.label, ant.nr_pvss, ant.x.test_signal, ant.y.test_signal, ant.x.offset, ant.y.offset))
                    else:
                        if lba.signal_check_done:
                            valstr = ''
                            if ant.x.too_low or ant.x.too_high: valstr += ",X=%3.1f" %(ant.x.test_signal)
                            if ant.y.too_low or ant.y.too_high: valstr += ",Y=%3.1f" %(ant.y.test_signal)    
                            if len(valstr):
                                log.addLine("%s,%s,%03d,RF_FAIL%s" %(date, lba.label, ant.nr_pvss, valstr))
                        
                        if lba.oscillation_check_done:
                            valstr = ''
                            if ant.x.osc: valstr += ',X=1'
                            if ant.y.osc: valstr += ',Y=1'
                            if len(valstr):
                                log.addLine("%s,%s,%03d,OSCILLATION%s" %(date, lba.label, ant.nr_pvss, valstr))
                        
                        if lba.spurious_check_done:
                            valstr = ''
                            if ant.x.spurious: valstr += ',X=1'
                            if ant.y.spurious: valstr += ',Y=1'
                            if len(valstr):
                                log.addLine("%s,%s,%03d,SPURIOUS%s" %(date, lba.label, ant.nr_pvss, valstr))
                            
                        if lba.noise_check_done:
                            noise = False
                            valstr = ''
                            if ant.x.low_noise: 
                                proc = (100.0 / ant.x.low_seconds) * ant.x.low_bad_seconds
                                valstr += ',Xproc=%5.3f,Xval=%3.1f,Xdiff=%5.3f,Xref=%3.1f' %(proc, ant.x.low_val, ant.x.low_diff, ant.x.low_ref)
                            if ant.y.low_noise: 
                                proc = (100.0 / ant.y.low_seconds) * ant.y.low_bad_seconds
                                valstr += ',Yproc=%5.3f,Yval=%3.1f,Ydiff=%5.3f,Yref=%3.1f' %(proc, ant.y.low_val, ant.y.low_diff, ant.y.low_ref)
                            if len(valstr):
                                log.addLine("%s,%s,%03d,LOW_NOISE%s" %(date, lba.label, ant.nr_pvss, valstr))
                                noise = True
                        
                            valstr = ''
                            if ant.x.high_noise:
                                proc = (100.0 / ant.x.high_seconds) * ant.x.high_bad_seconds
                                valstr += ',Xproc=%5.3f,Xval=%3.1f,Xdiff=%5.3f,Xref=%3.1f' %(proc, ant.x.high_val, ant.x.high_diff, ant.x.high_ref)
                            if ant.y.high_noise:
                                proc = (100.0 / ant.y.high_seconds) * ant.y.high_bad_seconds
                                valstr += ',Yproc=%5.3f,Yval=%3.1f,Ydiff=%5.3f,Yref=%3.1f' %(proc, ant.y.high_val, ant.y.high_diff, ant.y.high_ref)
                            if len(valstr):
                                log.addLine("%s,%s,%03d,HIGH_NOISE%s" %(date, lba.label, ant.nr_pvss, valstr))
                                noise = True
                            
                            valstr = ''
                            if not noise and ant.x.jitter:
                                proc = (100.0 / ant.x.jitter_seconds) * ant.x.jitter_bad_seconds
                                valstr += ',Xproc=%5.3f,Xdiff=%5.3f,Xref=%3.1f' %(proc, ant.x.jitter_val, ant.x.jitter_ref)
                            if not noise and ant.y.jitter:
                                proc = (100.0 / ant.y.jitter_seconds) * ant.y.jitter_bad_seconds
                                valstr += ',Xproc=%5.3f,Ydiff=%5.3f,Yref=%3.1f' %(proc, ant.y.jitter_val, ant.y.jitter_ref)
                            if len(valstr):
                                log.addLine("%s,%s,%03d,JITTER%s" %(date, lba.label, ant.nr_pvss, valstr))
            lba = None
        # end lbl/lbh                
        
        
        # hba
        for tile in self.hba.tile:
            if tile.x.error or tile.y.error:
                if self.hba.noise_check_done:
                    valstr = ''
                    noise = False
                    
                    if tile.x.low_noise:
                        proc = (100.0 / tile.x.low_seconds) * tile.x.low_bad_seconds
                        valstr += ',Xproc=%5.3f,Xval=%3.1f,Xdiff=%5.3f,Xref=%3.1f' %(proc, tile.x.low_val, tile.x.low_diff, tile.x.low_ref)
                    if tile.y.low_noise:
                        proc = (100.0 / tile.y.low_seconds) * tile.y.low_bad_seconds
                        valstr += ',Yproc=%5.3f,Yval=%3.1f,Ydiff=%5.3f,Yref=%3.1f' %(proc, tile.y.low_val, tile.y.low_diff, tile.y.low_ref)
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,LOW_NOISE%s" %(date, tile.nr, valstr))
                        noise = True
                
                    valstr = ''
                    if tile.x.high_noise:
                        proc = (100.0 / tile.x.high_seconds) * tile.x.high_bad_seconds
                        valstr += ',Xproc=%5.3f,Xval=%3.1f,Xdiff=%5.3f,Xref=%3.1f' %(proc, tile.x.high_val, tile.x.high_diff, tile.x.high_ref)
                    if tile.y.high_noise: 
                        proc = (100.0 / tile.y.high_seconds) * tile.y.high_bad_seconds
                        valstr += ',Yproc=%5.3f,Yval=%3.1f,Ydiff=%5.3f,Yref=%3.1f' %(proc, tile.y.high_val, tile.y.high_diff, tile.y.high_ref)
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,HIGH_NOISE%s" %(date, tile.nr, valstr))
                        noise = True

                    valstr = ''
                    if (not noise) and tile.x.jitter:
                        proc = (100.0 / tile.x.jitter_seconds) * tile.x.jitter_bad_seconds
                        valstr += ',Xproc=%5.3f,Xdiff=%5.3f,Xref=%3.1f' %(proc, tile.x.jitter_val, tile.x.jitter_ref)
                    if (not noise) and tile.y.jitter:
                        proc = (100.0 / tile.y.jitter_seconds) * tile.y.jitter_bad_seconds
                        valstr += ',Yproc=%5.3f,Ydiff=%5.3f,Yref=%3.1f' %(proc, tile.y.jitter_val, tile.y.jitter_ref)
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,JITTER%s" %(date, tile.nr, valstr))
                            
                if self.hba.oscillation_check_done:
                    valstr = ''
                    if tile.x.osc: valstr += ',X=1'
                    if tile.y.osc: valstr += ',Y=1'
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,OSCILLATION%s" %(date, tile.nr, valstr))
                
                if tile.p_summator_error:
                    log.addLine("%s,HBA,%03d,P_SUMMATOR" %(date, tile.nr))     
                
                # check for broken summators
                if self.hba.modem_check_done and tile.c_summator_error:
                    log.addLine("%s,HBA,%03d,C_SUMMATOR" %(date, tile.nr))
                
                if self.hba.summatornoise_check_done:
                    valstr = ''    
                    if tile.x.summator_noise: valstr += ',X=1'
                    if tile.y.summator_noise: valstr += ',Y=1'
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,SUMMATOR_NOISE%s" %(date, tile.nr, valstr))
                    
                if self.hba.spurious_check_done:
                    valstr = ''    
                    if tile.x.spurious: valstr += ',X=1'
                    if tile.y.spurious: valstr += ',Y=1'
                    if len(valstr):    
                        log.addLine("%s,HBA,%03d,SPURIOUS%s" %(date, tile.nr, valstr))
                
                if self.hba.signal_check_done:
                    valstr = ''    
                    if tile.x.too_low or tile.x.too_high:
                        valstr += ",X=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                  (tile.x.test_signal[0], tile.x.test_subband[0], tile.x.ref_signal[0],\
                                   tile.x.test_signal[1], tile.x.test_subband[1], tile.x.ref_signal[1])
                    if tile.y.too_low or tile.y.too_high:
                        valstr += ",Y=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                  (tile.y.test_signal[0], tile.y.test_subband[0], tile.y.ref_signal[0],\
                                   tile.y.test_signal[1], tile.y.test_subband[1], tile.y.ref_signal[1])
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,RF_FAIL%s" %(date, tile.nr, valstr))
                                       
                valstr = ''
                if self.hba.element_check_done:
                    for elem in tile.element:
                        #if tile.x.rcu_off or tile.y.rcu_off:
                        #    continue
                        
                        if elem.no_modem or elem.modem_error:
                            if elem.no_modem:
                                valstr += ",M%d=??" %(elem.nr)
                            
                            elif elem.modem_error:
                                valstr += ",M%d=error" %(elem.nr)
                        else:
                            if elem.x.osc or elem.y.osc:
                                if elem.x.osc:
                                    valstr += ",OX%d=1" %(elem.nr)
                                if elem.y.osc:
                                    valstr += ",OY%d=1" %(elem.nr)
                            
                            elif elem.x.spurious or elem.y.spurious:
                                if elem.x.spurious:
                                    valstr += ",SPX%d=1" %(elem.nr)
                                if elem.y.spurious:
                                    valstr += ",SPY%d=1" %(elem.nr)
                            
                            elif elem.x.low_noise or elem.x.high_noise or elem.y.low_noise or elem.y.high_noise or elem.x.jitter or elem.y.jitter:
                                if elem.x.low_noise:
                                    valstr += ",LNX%d=%3.1f %5.3f" %(elem.nr, elem.x.low_val, elem.x.low_diff)
                                
                                if elem.x.high_noise:
                                    valstr += ",HNX%d=%3.1f %5.3f" %(elem.nr, elem.x.high_val, elem.x.high_diff)
                                
                                if (not elem.x.low_noise) and (not elem.x.high_noise) and (elem.x.jitter > 0.0):
                                    valstr += ",JX%d=%5.3f" %(elem.nr, elem.x.jitter)
                                    
                                if elem.y.low_noise:
                                    valstr += ",LNY%d=%3.1f %5.3f" %(elem.nr, elem.y.low_val, elem.y.low_diff)               
                                
                                if elem.y.high_noise:
                                    valstr += ",HNY%d=%3.1f %5.3f" %(elem.nr, elem.y.high_val, elem.y.high_diff)
                                
                                if (not elem.y.low_noise) and (not elem.y.high_noise) and (elem.y.jitter > 0.0):
                                    valstr += ",JY%d=%5.3f" %(elem.nr, elem.y.jitter)
                            
                            elif elem.x.ref_signal[0] == 0 or elem.y.ref_signal[0] == 0:
                                log.addLine("%s,HBA,%03d,NOSIGNAL" %(date, tile.nr))
                            else:
                                if elem.x.error:
                                    valstr += ",X%d=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                              (elem.nr, elem.x.test_signal[0], elem.x.test_subband[0], elem.x.ref_signal[0],\
                                                          elem.x.test_signal[1], elem.x.test_subband[1], elem.x.ref_signal[1])
                                
                                if elem.y.error:
                                    valstr += ",Y%d=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                              (elem.nr, elem.y.test_signal[0], elem.y.test_subband[0], elem.y.ref_signal[0],\
                                                          elem.y.test_signal[1], elem.y.test_subband[1], elem.y.ref_signal[1])
                
                    if len(valstr):
                        log.addLine("%s,HBA,%03d,E_FAIL%s" %(date, tile.nr, valstr))
        # end HBA 
        return
        
#=======================================================================================================================
# database from here                
    class cRSP:
        def __init__(self, nr):
            self.nr = nr
            
            self.test_done  = 0
            self.board_ok   = 1
            self.ap_version = 'ok'
            self.bp_version = 'ok'
            self.version_ok = 1
        
        def test(self):
            if self.ap_version != 'ok' or self.bp_version != 'ok':
                self.version_ok = 0
            return
    # used by LBA and HBA antenna class        
    class cPolarity:
        def __init__(self, rcu=None):
            
            self.rcu            = rcu
            self.rcu_off        = 0 # 0 = RCU on, 1 = RCU off
            self.rcu_error      = 0
            
            # status variables 0|1       
            self.error          = 0    #
            self.too_low        = 0    #
            self.too_high       = 0    #
            self.low_noise      = 0    #
            self.high_noise     = 0    #
            self.jitter         = 0    #
            self.osc            = 0    #
            self.no_signal      = 0    # signal below 2dB
            self.summator_noise = 0    #
            self.spurious       = 0    #

            # test result of signal test, 
            # if HBA test firt value ctrl=128 second value ctrl=253
            # if LBA test only first values are used
            self.test_subband   = [0, 0]
            self.ref_signal     = [-1, -1]
            self.test_signal    = [0.0, 0.0]
            self.offset         = 0
            
            # measured values filled on error
            # proc : bad time in meausured time 0..100%        
            # val  : max or min meausured value
            self.low_seconds     = 0
            self.low_bad_seconds = 0
            self.low_val         = 100.0  #
            self.low_diff        = 0.0
            self.low_ref         = 0.0  #
    
            self.high_seconds     = 0
            self.high_bad_seconds = 0
            self.high_val         = 0.0  #
            self.high_diff        = 0.0
            self.high_ref         = 0.0  #
            
            self.jitter_seconds     = 0
            self.jitter_bad_seconds = 0
            self.jitter_val         = 0.0
            self.jitter_ref         = 0.0
    
    class cLBA_DB:
        def __init__(self, label, nr_antennas, nr_offset=0):
            self.rsp_driver_down         = False
            self.noise_check_done        = 0
            self.signal_check_done       = 0
            self.spurious_check_done     = 0
            self.oscillation_check_done  = 0
            
            self.noise_low_deviation     = 0.0
            self.noise_high_deviation    = 0.0
            self.noise_max_fluctuation   = 0.0
            
            self.rf_low_deviation        = 0.0
            self.rf_high_deviation       = 0.0
            self.rf_subband              = 0
            
            self.check_time_noise        = 0
            self.nr_antennas             = nr_antennas
            self.nr_offset               = nr_offset
            self.label                   = label
            self.error                   = 0
            self.avg_2_low               = 0
            self.avg_x                   = 0
            self.avg_y                   = 0
            self.test_subband_x          = 0
            self.test_subband_y          = 0
            self.test_signal_x           = 0
            self.test_signal_y           = 0
            self.nr_bad_antennas         = -1
            self.ant                     = list()
            for i in range(self.nr_antennas):
                self.ant.append(self.cAntenna(i, self.nr_offset))
            return
        
        def test(self):
            if self.rsp_driver_down:
                return (self.error)
            if self.noise_check_done or self.signal_check_done or self.spurious_check_done or self.oscillation_check_done:
                self.nr_bad_antennas = 0
            
            for ant in self.ant:
                ant.test()
                ant_error = max(ant.x.error, ant.y.error)
                self.error = max(self.error, ant_error)
                if ant_error:
                    self.nr_bad_antennas += 1
            return (self.error) 
        
        # return select string for rspctl command
        def selectList(self):
            select = list()
            for ant in self.ant:
                if ant.on_bad_list == 0:
                    select.append(ant.x.rcu)
                    select.append(ant.y.rcu)
            return (select)
            
        def resetRcuState(self):
            for ant in self.ant:
                ant.x.rcu_off = 0
                ant.y.rcu_off = 0
            
        class cAntenna:
            def __init__(self, nr, nr_offset):
                self.nr          = nr
                self.nr_pvss     = self.nr + nr_offset
                self.on_bad_list = 0
                self.x           = cDB.cPolarity(rcu=(self.nr * 2))
                self.y           = cDB.cPolarity(rcu=((self.nr * 2) + 1))
                
                self.down = 0
                return
                
            def test(self):
                self.x.error = max(self.x.too_low, self.x.too_high, self.x.osc, self.x.high_noise, self.x.low_noise, self.x.jitter, self.x.spurious, self.down)
                self.y.error = max(self.y.too_low, self.y.too_high, self.y.osc, self.y.high_noise, self.y.low_noise, self.y.jitter, self.y.spurious, self.down)
                return
            
    class cHBA_DB:
        def __init__(self, nr_tiles):
            self.rsp_driver_down           = False
            self.modem_check_done          = 0
            self.noise_check_done          = 0
            self.signal_check_done         = 0
            self.spurious_check_done       = 0
            self.oscillation_check_done    = 0
            self.summatornoise_check_done  = 0
            self.element_check_done        = 0
            
            self.check_time_noise          = 0
            self.check_time_noise_elements = 0
            self.nr_tiles                  = nr_tiles
            self.error                     = 0
            self.avg_2_low                 = 0
            self.tile                      = list()
            self.nr_bad_tiles              = -1
            for i in range(self.nr_tiles):
                self.tile.append(self.cTile(i))
            return
        
        def test(self):
            if self.rsp_driver_down:
                return (self.error)
            if self.modem_check_done or self.noise_check_done or self.signal_check_done or self.spurious_check_done \
               or self.oscillation_check_done or self.summatornoise_check_done or self.element_check_done :
                self.nr_bad_tiles = 0
                    
            for tile in self.tile:
                tile.test()
                tile_error = max(tile.x.error, tile.y.error)
                self.error = max(self.error, tile_error)
                
                if tile_error:
                    self.nr_bad_tiles += 1

            return (self.error)   
        
        # return select string for rspctl command
        def selectList(self):
            select = list()
            for tile in self.tile:
                if tile.on_bad_list == 0:
                    select.append(tile.x.rcu)
                    select.append(tile.y.rcu)
            return (select)
            
        def resetRcuState(self):
            for tile in self.tile:
                tile.x.rcu_off = 0
                tile.y.rcu_off = 0
        
        class cTile:
            def __init__(self, nr):
                self.nr          = nr
                self.on_bad_list = 0
                self.x           = cDB.cPolarity(rcu=(nr*2))
                self.y           = cDB.cPolarity(rcu=(nr*2+1))
            
                self.noise_low_deviation     = 0.0
                self.noise_high_deviation    = 0.0
                self.noise_max_fluctuation   = 0.0
                
                self.rf_low_deviation        = 0.0
                self.rf_high_deviation       = 0.0
                self.rf_subband              = 0
            
                self.no_power         = 0    # signal around 60dB  
                self.p_summator_error = 0
                self.c_summator_error = 0
                self.nr_elements      = 16
                self.element = list()
                for i in range(1,self.nr_elements+1,1):
                    self.element.append(self.cElement(i))
                return
            
            def test(self):
                no_modem_cnt = 0
                modem_err_cnt = 0
                no_power_cnt = 0
                x_no_signal_cnt = 0
                y_no_signal_cnt = 0
                for elem in self.element:
                    elem.test()
                    if elem.x.no_signal:
                        x_no_signal_cnt += 1
                    if elem.y.no_signal:
                        y_no_signal_cnt += 1
                    if elem.no_power:
                        no_power_cnt += 1
                    if elem.no_modem:
                        no_modem_cnt += 1
                    if elem.modem_error:
                        modem_err_cnt += 1
    
                    self.x.error = max(self.x.error, elem.x.error)
                    self.y.error = max(self.y.error, elem.y.error)
                    
                if (no_modem_cnt >= 8) or (modem_err_cnt >= 8):
                    self.c_summator_error = 1
                if no_power_cnt >= 15:
                    self.p_summator_error = 1
                if x_no_signal_cnt == 16:
                    self.x.rcu_error = 1
                if y_no_signal_cnt == 16:
                    self.y.rcu_error = 1
                
                self.x.error = max(self.x.error, self.x.too_low, self.x.too_high, self.x.low_noise, self.x.no_signal, self.x.high_noise, self.x.jitter, self.x.osc,
                                   self.x.summator_noise, self.x.spurious, self.p_summator_error, self.c_summator_error) 
                
                self.y.error = max(self.y.error, self.y.too_low, self.y.too_high, self.y.low_noise, self.y.no_signal, self.y.high_noise, self.y.jitter, self.y.osc,
                                   self.y.summator_noise, self.y.spurious, self.p_summator_error, self.c_summator_error) 
                return
            
            
            class cElement:
                def __init__(self, nr):
                    self.nr = nr
                    self.x = cDB.cPolarity()
                    self.y = cDB.cPolarity()

                    self.noise_low_deviation     = 0.0
                    self.noise_high_deviation    = 0.0
                    self.noise_max_fluctuation   = 0.0
                    
                    self.rf_low_deviation        = 0.0
                    self.rf_high_deviation       = 0.0
                    self.rf_subband              = 0

                    self.no_power                = 0    # signal around 60dB
                    self.no_modem                = 0    # modem reponse = ??
                    self.modem_error             = 0    # wrong response from modem
                    
                    return
                
                def test(self):
                    self.x.error = max(self.x.too_low, self.x.too_high, self.x.low_noise, self.x.high_noise, self.x.no_signal,
                                       self.x.jitter, self.no_power, self.x.spurious, self.x.osc, self.no_modem, self.modem_error)
                    
                    self.y.error = max(self.y.too_low, self.y.too_high, self.y.low_noise, self.y.high_noise, self.y.no_signal, 
                                       self.y.jitter, self.no_power, self.y.spurious, self.y.osc, self.no_modem, self.modem_error)
                    return
         
                
    class cTDS:
        def __init__(self):
            self.test_done = 0
            self.ok = 1
    
    class cTBB:
        def __init__(self, nr):
            self.nr = nr
            self.test_done = 0
            self.tp_version = 'ok'
            self.mp_version = 'ok'
            self.memory_size = 0
            self.version_ok = 1
            self.memory_ok = 1
        
        def test(self):
            if self.tp_version != 'ok' or self.mp_version != 'ok':
                self.version_ok = 0
            if self.memory_size != 0:
                self.memory_ok = 0
                
            return 
            
            
