#!/usr/bin/python

from general_lib import *
from lofar_lib import *

# PVSS states
State = dict({'OFF':0, 'OPERATIONAL':1, 'MAINTENANCE':2, 'TEST':3, 'SUSPICIOUS':4, 'BROKEN':5})
    
class cDB:
    def __init__(self, StID, nRSP, nTBB, nLBL, nLBH, nHBA, clean=False):
        self.StID   = StID
        self.nr_rsp = nRSP
        self.nr_rcu = nRSP * 8
        self.nr_lbl = nLBL
        self.nr_lbh = nLBH
        self.nr_hba = nHBA
        self.nr_tbb = nTBB
        
        self.tests = ''
        
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
        
        self.lbl = self.cLBA(label='LBL', nr_antennas=nLBL, nr_offset=48)
        self.lbh = self.cLBA(label='LBH', nr_antennas=nLBH)
        self.hba = self.cHBA(nr_tiles=nHBA)
                
        # try to read in from last test file LBL, LBH and HBA status
        try:
            if not clean:
                print "Import latest log file"
                f = open("%s_StationTest.csv" %(self.StID), 'r')
                data = f.readlines()
                f.close()
                for line in data:
                    d = line.split(',')
                    if d[1] == 'LBL':
                        if d[2] != '---':
                            nr = int(d[2])
                            msg = d[3].strip()
                            if msg in ('DOWN', 'FAIL', 'LOW_NOISE', 'HIGH_NOISE'):
                                self.lbl.ant[nr].last_state = State['BROKEN']
                    elif d[1] == 'LBH':
                        if d[2] != '---':
                            nr = int(d[2])
                            msg = d[3].strip()
                            if msg in ('DOWN', 'FAIL', 'LOW_NOISE', 'HIGH_NOISE'):
                                self.lbh.ant[nr].last_state = State['BROKEN']
                    elif d[1] == 'HBA':
                        if d[2] != '---':
                            nr = int(d[2])
                            msg = d[3].strip()
                            if msg in ('FAIL', 'LOW_NOISE', 'HIGH_NOISE'):
                                self.hba.tile[nr].las_state = State['BROKEN']
                                for elem in hba.tile[nr].element:
                                    nr = elem.nr
                                    if line.find('M%d=' %(nr)) > 0 or line.find('X%d=' %(nr)) > 0 or line.find('Y%d=' %(nr)) > 0:
                                        elem.last_state = State['BROKEN']
        except:
            pass
            
    
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
                if self.nr_lbl > 0 and self.lbl.ant[ant_nr].x_error: error_count += 1
                if self.lbh.ant[ant_nr].x_error: error_count += 1
                if self.hba.tile[ant_nr].x_rcu_error: error_count += 1
            else:
                if self.nr_lbl > 0 and self.lbl.ant[ant_nr].y_error: error_count += 1
                if self.lbh.ant[ant_nr].y_error: error_count += 1
                if self.hba.tile[ant_nr].y_rcu_error: error_count += 1
                    
            if error_count >= 2:
                self.rcu_state[_rcu] = 1
           
        self.station_error = max(self.station_error, self.lbl.test(), self.lbh.test(), self.hba.test())
        
        self.makeLogFile(logdir)
        self.makePVSSfile(logdir)      
        return (self.station_error)
    
    
    # make standard log file
    def makeLogFile(self, logdir):
        print logdir
        date = getShortDateStr()
        log = cTestLogger(logdir)
        
        log.addLine("%s,NFO,---,TESTS%s" %(date, self.tests))

        if self.rspdriver_version != "ok" or self.rspctl_version != "ok":
            log.addLine("%s,RSP,---,VERSION,RSPDRIVER=%s,RSPCTL=%s" %(date, self.rspdriver_version, self.rspctl_version))
        
        for rsp in self.rsp:
            rsp.test()
            if not rsp.version_ok:
                log.addLine("%s,RSP,%03d,VERSION,BP=%s,AP=%s" %\
                           (date, rsp.nr, rsp.bp_version, rsp.ap_version))
        
        if self.tbbdriver_version != "ok" or self.tbbctl_version != "ok":
            log.addLine("%s,TBB,---,VERSION,TBBDRIVER=%s,TBBCTL=%s" %(date, self.tbbdriver_version, self.tbbctl_version))
        
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
            if lba.check_done:
                if lba.test_signal_x == 0 or lba.test_signal_y == 0:
                    log.addLine("%s,%s,---,NOSIGNAL" %(date, lba.label))
    
                elif lba.avg_2_low:
                    log.addLine("%s,%s,---,TOOLOW ,AVGX=%3.1f,AVGY=%3.1f" %(date, lba.label, lba.avg_x, lba.avg_y))
    
                else:
                    if lba.error:
                        log.addLine("%s,%s,---,TESTSIGNAL,SUBBANDX=%d,SIGNALX=%3.1f,SUBBANDY=%d,SIGNALY=%3.1f" %\
                                   (date, lba.label, lba.test_subband_x, lba.test_signal_x, lba.test_subband_y, lba.test_signal_y))
                   
                    for ant in lba.ant:
                        if ant.down:
                            log.addLine("%s,%s,%03d,DOWN  ,X=%3.1f,Y=%3.1f,Xoff=%d,Yoff=%d" %\
                                       (date, lba.label, ant.nr, ant.x_signal, ant.y_signal, ant.x_offset, ant.y_offset))
                        else:
                            valstr = ''
                            if ant.x_low_noise or ant.y_low_noise:
                                if ant.x_low_noise: valstr += ',Xproc=%3.1f,Xval=%3.1f' %(ant.x_low_proc, ant.x_low_val)
                                if ant.y_low_noise: valstr += ',Yproc=%3.1f,Yval=%3.1f' %(ant.y_low_proc, ant.y_low_val)
                                log.addLine("%s,%s,%03d,LOW_NOISE%s" %(date, lba.label, ant.nr, valstr))
                            
                            valstr = ''
                            if ant.x_high_noise or ant.y_high_noise:
                                if ant.x_high_noise: valstr += ',Xproc=%3.1f,Xval=%3.1f' %(ant.x_high_proc, ant.x_high_val)
                                if ant.y_high_noise: valstr += ',Yproc=%3.1f,Yval=%3.1f' %(ant.y_high_proc, ant.y_high_val)
                                log.addLine("%s,%s,%03d,HIGH_NOISE%s" %(date, lba.label, ant.nr, valstr))
                            
                            valstr = ''
                            if ant.x_too_low or ant.x_too_high:
                                valstr += ",X=%3.1f" %(ant.x_signal)
                            if ant.y_too_low or ant.y_too_high:
                                valstr += ",Y=%3.1f" %(ant.y_signal)    
                            if len(valstr) > 0:    
                                log.addLine("%s,%s,%03d,FAIL%s" %\
                                           (date, lba.label, ant.nr, valstr))
        
        # hba
        for tile in self.hba.tile:
            if tile.x_error or tile.y_error:
                valstr = ''
                if tile.x_low_noise or tile.y_low_noise:
                    if tile.x_low_noise: valstr += ',Xproc=%3.1f,Xval=%3.1f' %(tile.x_low_proc, tile.x_low_val)
                    if tile.y_low_noise: valstr += ',Yproc=%3.1f,Yval=%3.1f' %(tile.y_low_proc, tile.y_low_val)
                    log.addLine("%s,HBA,%03d,LOW_NOISE%s" %(date, tile.nr, valstr))

                if tile.x_high_noise or tile.y_high_noise:
                    if tile.x_high_noise: valstr += ',Xproc=%3.1f,Xval=%3.1f' %(tile.x_high_proc, tile.x_high_val)
                    if tile.y_high_noise: valstr += ',Yproc=%3.1f,Yval=%3.1f' %(tile.y_high_proc, tile.y_high_val)
                    log.addLine("%s,HBA,%03d,HIGH_NOISE%s" %(date, tile.nr, valstr))
                
                # check for broken summators
                if tile.c_summator_error:
                    log.addLine("%s,HBA,%03d,C_SUMMATOR" %(date, tile.nr))
                
                if tile.p_summator_error:
                    log.addLine("%s,HBA,%03d,P_SUMMATOR" %(date, tile.nr))     
                
                valstr = ''
                for elem in tile.element:
                    if elem.x_test_signal == 0 or elem.y_test_signal == 0:
                        log.addLine("%s,HBA,---,NOSIGNAL" %(date))
                    else:
                        if elem.no_modem:
                            valstr += ",M%d=??" %(elem.nr+1)
                        
                        elif elem.modem_error:
                            valstr += ",M%d=error" %(elem.nr+1)
                        
                        else:
                            if elem.x_low_noise:
                                valstr += ",LNX%d=1" %(elem.nr+1)
                            
                            if elem.x_high_noise:
                                valstr += ",HNX%d=1" %(elem.nr+1)
                            
                            if elem.y_low_noise:
                                valstr += ",LNY%d=1" %(elem.nr+1)               
                            
                            if elem.y_high_noise:
                                valstr += ",HNY%d=1" %(elem.nr+1)
                                
                            if elem.x_error:
                                valstr += ",X%d=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                          (elem.nr+1, elem.x_signal[0], elem.x_test_subband[0], elem.x_test_signal[0],\
                                                      elem.x_signal[1], elem.x_test_subband[1], elem.x_test_signal[1])
                            
                            if elem.y_error:
                                valstr += ",Y%d=%3.1f %d %3.1f %3.1f %d %3.1f" %\
                                          (elem.nr+1, elem.y_signal[0], elem.y_test_subband[0], elem.y_test_signal[0],\
                                                      elem.y_signal[1], elem.y_test_subband[1], elem.y_test_signal[1])
                
                if len(valstr):
                    log.addLine("%s,HBA,%03d,FAIL%s" %(date, tile.nr, valstr)) 
        return


    # make log file for PVSS logging, use setObjectState to enter settings in PVSS              
    """       
    Syntax: /opt/lofar/sbin/setObjectState who datapoint stateNr
    OR    /opt/lofar/sbin/setObjectState who filename
    who        : Identification who changes the state, name of program or user
    datapoint  : PVSS datapoint name including database name
    stateNr    : 0 - Off
                 1 - Operational
                 2 - Maintenance
                 3 - Test
                 4 - Suspicious
                 5 - Broken
    """
    def makePVSSfile(self, logdir):
        pvss = cPVSSLogger(logdir)
        
        for ant in self.lbh.ant:
            ant.testPVSS()
            pvss.addLine("LOFAR_PIC_LBA%03d %d" %(ant.nr_total, ant.state))     
            pvss.addLine("LOFAR_PIC_LBA%03d.X %d" %(ant.nr_total, ant.x_state))     
            pvss.addLine("LOFAR_PIC_LBA%03d.Y %d" %(ant.nr_total, ant.y_state))     
        
        for ant in self.lbl.ant:
            ant.testPVSS()
            pvss.addLine("LOFAR_PIC_LBA%03d %d" %(ant.nr_total, ant.state))                
            pvss.addLine("LOFAR_PIC_LBA%03d.X %d" %(ant.nr_total, ant.x_state))                
            pvss.addLine("LOFAR_PIC_LBA%03d.Y %d" %(ant.nr_total, ant.y_state))                
        
        for tile in self.hba.tile:
            tile.testPVSS()
            pvss.addLine("LOFAR_PIC_HBA%02d %d" %(tile.nr, tile.state))
            for elem in tile.element:
                pvss.addLine("LOFAR_PIC_HBA%02d.element%02d %d" %(tile.nr, elem.nr, elem.state))
                pvss.addLine("LOFAR_PIC_HBA%02d.element%02d.comm %d" %(tile.nr, elem.nr, elem.modem_state))
                pvss.addLine("LOFAR_PIC_HBA%02d.element%02d.X %d" %(tile.nr, elem.nr, elem.x_state))
                pvss.addLine("LOFAR_PIC_HBA%02d.element%02d.Y %d" %(tile.nr, elem.nr, elem.y_state))
        return
                
    
    class cRSP:
        def __init__(self, nr):
            self.nr = nr
            self.test_done = 0
            self.board_ok = 1
            self.ap_version = 'ok'
            self.bp_version = 'ok'
            self.version_ok = 1
        
        def test(self):
            if self.ap_version != 'ok' or self.bp_version != 'ok':
                self.version_ok = 0
            return
            
    class cLBA:
        def __init__(self, label, nr_antennas, nr_offset=0):
            self.check_done = 0
            self.check_time_noise = 0
            self.nr_antennas = nr_antennas
            self.nr_offset = nr_offset
            self.label = label
            self.error   = 0
            self.avg_2_low = 0
            self.avg_x    = 0
            self.avg_y    = 0
            self.test_subband_x = 0
            self.test_subband_y = 0
            self.test_signal_x  = 0
            self.test_signal_y  = 0
            self.ant = list()
            for i in range(self.nr_antennas):
                self.ant.append(self.cAntenna(i, nr_offset))
            return
        
        def test(self):
            for ant in self.ant:
                ant.test()
                self.error = max(self.error, ant.x_error, ant.y_error)
            return (self.error) 
            
        class cAntenna:
            def __init__(self, nr, nr_offset):
                self.nr = nr
                self.nr_total = nr + nr_offset
                self.x_rcu        = nr * 2 
                self.y_rcu        = nr * 2 + 1
                # test result
                self.x_signal     = 0
                self.x_offset     = 0
                self.y_signal     = 0
                self.y_offset     = 0
                # test status
                self.x_error      = 0
                self.y_error      = 0
                self.x_too_low    = 0
                self.y_too_low    = 0
                self.x_too_high   = 0
                self.y_too_high   = 0
                self.x_rcu_error  = 0
                self.y_rcu_error  = 0
                
                # noise: state 0|1
                # proc : bad time in meausured time 0..100%        
                # val  : max or min meausured value
                self.x_low_noise  = 0  
                self.x_low_proc   = 0.0
                self.x_low_val    = 0.0
                self.x_high_noise = 0  
                self.x_high_proc  = 0.0
                self.x_high_noise = 0.0
                self.y_low_noise  = 0  
                self.y_low_proc   = 0.0
                self.y_low_val    = 0.0
                self.y_high_noise = 0  
                self.y_high_proc  = 0.0
                self.y_high_val   = 0.0

                self.down = 0
                
                self.last_state = State['OFF']
                # states for PVSS
                self.x_state = State['OFF']
                self.y_state = State['OFF']
                self.state = State['OFF']
                return
                
            def test(self):
                self.x_error = max(self.x_too_low, self.x_too_high, self.x_high_noise, self.x_low_noise, self.down)
                self.y_error = max(self.y_too_low, self.y_too_high, self.y_high_noise, self.y_low_noise, self.down)
                return
            
            def testPVSS(self):
                if self.x_error:
                    self.x_state = State['BROKEN']
                if self.y_error:
                    self.y_state = State['BROKEN']
                self.state = max(self.last_state, self.state, self.x_state, self.y_state)
                return
            
    class cHBA:
        def __init__(self, nr_tiles):
            self.check_done = 0
            self.check_time_noise = 0
            self.check_time_noise_elements = 0
            self.nr_tiles = nr_tiles
            self.error   = 0
            self.avg_2_low = 0
            self.tile = list()
            for i in range(self.nr_tiles):
                self.tile.append(self.cTile(i))
            return
        
        def test(self):
            for tile in self.tile:
                tile.test()
                self.error = max(self.error, tile.x_error, tile.y_error)
            return (self.error)   
        
        class cTile:
            def __init__(self, nr):
                self.nr = nr
                self.x_rcu_off  = 0 # 0 = RCU on, 1 = RCU off
                self.y_rcu_off  = 0
                self.x_rcu        = nr * 2 
                self.y_rcu        = nr * 2 + 1
                self.x_rcu_error  = 0
                self.y_rcu_error  = 0
                self.x_error      = 0
                self.y_error      = 0

                # noise: state 0|1
                # proc : bad time in meausured time 0..100%        
                # val  : max or min meausured value
                self.x_low_noise  = 0  
                self.x_low_proc   = 0.0
                self.x_low_val    = 0.0
                self.x_high_noise = 0  
                self.x_high_proc  = 0.0
                self.x_high_noise = 0.0
                self.y_low_noise  = 0  
                self.y_low_proc   = 0.0
                self.y_low_val    = 0.0
                self.y_high_noise = 0  
                self.y_high_proc  = 0.0
                self.y_high_val   = 0.0
                
                self.p_summator_error = 0
                self.c_summator_error = 0
                self.nr_elements      = 16
                self.element = list()
                for i in range(self.nr_elements):
                    self.element.append(self.cElement(i))
                
                # states for PVSS
                self.state = State['OFF']
                return
            
            def test(self):
                no_modem_cnt = 0
                modem_err_cnt = 0
                no_power_cnt = 0
                x_no_signal_cnt = 0
                y_no_signal_cnt = 0
                for elem in self.element:
                    elem.test()
                    if elem.x_no_signal:
                        x_no_signal_cnt += 1
                    if elem.y_no_signal:
                        y_no_signal_cnt += 1
                    if elem.no_power:
                        no_power_cnt += 1
                    if elem.no_modem:
                        no_modem_cnt += 1
                    if elem.modem_error:
                        modem_err_cnt += 1
    
                    #self.x_low_noise  = max(self.x_low_noise, elem.x_low_noise)
                    #self.x_high_noise = max(self.x_high_noise, elem.x_high_noise)
                    #self.y_low_noise  = max(self.y_low_noise, elem.y_low_noise)
                    #self.y_high_noise = max(self.y_high_noise, elem.y_high_noise)
                    self.x_error = max(self.x_error, elem.x_error)
                    self.y_error = max(self.y_error, elem.y_error)
                    
                if (no_modem_cnt >= 8) or (modem_err_cnt >= 8):
                    self.c_summator_error = 1
                if no_power_cnt >= 15:
                    self.p_summator_error = 1
                if x_no_signal_cnt == 16:
                    self.x_rcu_error = 1
                if y_no_signal_cnt == 16:
                    self.y_rcu_error = 1
                
                self.x_error = max(self.x_error, self.x_low_noise, self.x_high_noise, self.p_summator_error, self.c_summator_error) 
                self.y_error = max(self.y_error, self.y_low_noise, self.y_high_noise, self.p_summator_error, self.c_summator_error) 
                return
            
            def testPVSS(self):
                for elem in self.element:
                    elem.testPVSS()
                    self.state = max(self.state, elem.state)
                return       
                
            class cElement:
                def __init__(self, nr):
                    self.nr = nr
                    # test result, firt value ctrl=128 second value ctrl=253
                    self.x_test_subband = [0, 0]
                    self.y_test_subband = [0, 0]
                    self.x_test_signal = [0, 0]
                    self.y_test_signal = [0, 0]
                    self.x_signal = [0.0, 0.0]
                    self.y_signal = [0.0, 0.0]
                    # test status
                    self.x_error = 0
                    self.y_error = 0
                    self.x_too_low = 0
                    self.y_too_low = 0
                    self.x_too_high = 0
                    self.y_too_high = 0
                    self.x_low_noise  = 0
                    self.x_high_noise = 0
                    self.y_low_noise  = 0
                    self.y_high_noise = 0
                    self.x_no_signal = 0 # signal below 2dB
                    self.y_no_signal = 0 # signal below 2dB
                    self.no_power = 0 # signal around 60dB
                    self.no_modem = 0 # modem reponse = ??
                    self.modem_error = 0 # wrong response from modem
                    
                    self.last_state = State['OFF']
                    # states for PVSS
                    self.modem_state = State['OFF']
                    self.x_state = State['OFF']
                    self.y_state = State['OFF']
                    self.state = State['OFF']
                    return
                
                def test(self):
                    self.x_error = max(self.x_too_low, self.x_too_high, self.x_low_noise, self.x_high_noise, self.x_no_signal,
                                       self.no_power, self.no_modem, self.modem_error)
                    
                    self.y_error = max(self.y_too_low, self.y_too_high, self.y_low_noise, self.y_high_noise, self.y_no_signal, 
                                       self.no_power, self.no_modem, self.modem_error)
                    return
                
                # test for PVSS settings    
                def testPVSS(self):
                    self.state = self.last_state
    
                    if self.modem_error:
                        self.modem_state = State['SUSPICIOUS']
                    
                    if self.no_modem:
                        self.modem_state = State['BROKEN']
                    
                    if self.x_no_signal or self.x_too_low or self.x_low_noise or self.x_high_noise or self.no_power or self.no_modem or self.modem_error:
                        self.x_state = State['BROKEN']
                    
                    if self.y_no_signal or self.y_too_low or self.y_low_noise or self.y_high_noise or self.no_power or self.no_modem or self.modem_error:
                        self.y_state = State['BROKEN']
                        
                    self.state = max(self.state, self.x_state, self.y_state, self.modem_state)
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
            
            
