## P.Donker ASTRON februari 2010
## EC control module

import socket
import struct
import time

class EC:
    # cmdIDs from TCP PROTOCOL ec-controller
    EC_NONE             = 0
    EC_STATUS           = 1
    EC_SETTINGS         = 2
    EC_CTRL_TEMP        = 3
    EC_VERSION          = 5
    EC_SET_CTRL_MODE    = 10
    EC_SET_FANS         = 11
    EC_SET_TEMP         = 15
    EC_SET_HEATER       = 17
    EC_SET_48           = 20
    EC_RESET_48         = 22
    EC_SET_230          = 25
    EC_RESET_230        = 27
    EC_RESET_TRIP       = 28
    EC_SET_DOOR_CTRL    = 50
    EC_SET_HUM_CTRL     = 52
    EC_SET_SECOND       = 115
    
    SET_MAX_CTRL_TEMP   = 150
    SET_MIN_CTRL_TEMP   = 151
    SET_WARN1_TEMP      = 152
    SET_WARN2_TEMP      = 153
    SET_TRIP_TEMP       = 154
    SET_HEATER_TEMP     = 155
    SET_MAX_HUM         = 160
    SET_TRIP_HUM        = 161
    SET_START_SIDE      = 170
    SET_BALANCE_POINT   = 171
    SET_MAX_CHANGE      = 172
    SET_SEEK_TIME       = 173
    SET_SEEK_CHANGE     = 174
                        
    SET_TEMP_OFFSET     = 180
    SET_TEMP_MULT       = 181
    SET_HUM_OFFSET      = 182
    SET_HUM_MULT        = 183
    SENSOR_SETTINGS     = 184
    SET_LOG             = 1111
    FLASH_ERASE         = 2330
    FLASH_WRITE         = 2331
    RESTART             = 2440

    MODE_OFF     = 0
    MODE_ON      = 1
    MODE_AUTO    = 2
    MODE_MANUAL  = 3
    MODE_STARTUP = 4
    MODE_SEEK    = 5
    PWR_OFF      = 0
    PWR_ON       = 1
    LCU          = 230

    printToScreen = False
    printToFile = False
    cabs = None
    nCabs = None
    host = None
    station = None
    port = 10000
    sck = None
    logger = False
    info = ''
    version = 0
    versionstr = 'V-.-.-'
    
    def __init__(self, addr='0.0.0.0', nCabs=3):
        self.nCabs = nCabs
        if nCabs == 1: self.cabs = [0]
        if nCabs == 3: self.cabs = [0,1,3]
        if nCabs == 4: self.cabs = [0,1,2,3]
        self.host = addr
        try:
            (hostname,a,b) = socket.gethostbyaddr(addr)
            self.station = hostname.split('.')[0]
        except:
            self.station = 'Unknown'
                
    def setInfo(self, info):
        self.info = info
        if self.printToScreen:
            print self.info
            self.info = ''
        else: self.info += '\n'
        return

    def addInfo(self, info):
        self.info += info
        if self.printToScreen:
            print self.info
            self.info = ''
        else: self.info += '\n'
        return
    
    def printInfo(self, state=True):
        self.printToScreen = state
        return
 
    def printFile(self, state=True):
        self.printToFile = state
        return
    
    def hex2bit(self, val=0, bits=16):
        bit = ''
        for i in range(bits-1,-1,-1):
            if val & (1 << i):
                bit += '1'
            else:
                bit += '0'
        return(bit) 

    #---------------------------------------
    def connectToHost(self):
        self.setInfo("connecting to %s on port %d" %(self.host, self.port))
        connected = False

        try:
            self.sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except socket.error:
            self.sck.close()
            return(connected)

        try:
            self.sck.settimeout(3.0)
            self.sck.connect((self.host, self.port))
            connected = True
            #time.sleep(0.1)
        except socket.error:
            self.sck.close()
        return(connected)
            
    #---------------------------------------
    def disconnectHost(self):
        self.sck.close()
        #time.sleep(0.1)
        return
        
    #---------------------------------------
    def sendCmd(self, cmdId=0, cab=-1, value=0):
        if (cmdId == self.EC_NONE):
            return (false)
        try:
            cmd = struct.pack('hhh', cmdId, cab, int(value))
            self.sck.send(cmd)
        except socket.error:
            self.setInfo("socket error, try to reconnect")
            self.disconnectHost()
            time.sleep(10.0)
            self.connectToHost()
        return
    #---------------------------------------
    def recvAck(self):
        socketError = False    
        try:
            self.sck.settimeout(1.0)
            data = self.sck.recv(6)
        except socket.error:
            socketError = True
            self.setInfo("socket error, try to reconnect")
            #self.disconnectHost()
            #self.connectToHost()
        if socketError:
            return(0,0,[])
            
        header = struct.unpack('hhh', data)
        cmdId = header[0]
        status = header[1]
        PLSize = header[2]
        if (PLSize > 0):
            data = self.sck.recv(PLSize)
            fmt = 'h' * int(PLSize / 2)
            PL = struct.unpack(fmt, data)
        else:
            PL = []
        return (cmdId, status, PL) 
    #---------------------------------------
    def recvLog(self):
        socketError = False    
        try:
            self.sck.settimeout(10.0)
            data = self.sck.recv(200)
        except socket.error:
            socketError = True
        if socketError:
            return
        print str(data).replace('\r',''),
        return 
    
    #---------------------------------------
    def setSecond(self, sec=0):
        self.sendCmd(self.EC_SET_SECOND, 0, sec)
        (cmdId, status, PL) = self.recvAck()
        return
    #---------------------------------------
    def waitForSync(self):
        while ((time.gmtime()[5] % 10) != 7):
            time.sleep(0.5)
        time.sleep(1.0)
        return
    #---------------------------------------
    def waitForUpdate(self):
        while ((time.gmtime()[5] % 10) != 3):
            time.sleep(0.5)
        time.sleep(1.0)
        return
    #---------------------------------------
    def setLogger(self, level=0):
        if (self.version >= 150):
            self.setInfo('EC set logger to level %d' %(level))
            self.sendCmd(self.SET_LOG, 0, level)
            #(cmdId, status, PL) = self.recvAck()
            return 
        else:
            self.setInfo('setting log level not possible in this EC version')
        return 
    #---------------------------------------
    
    def restart(self):
        if (self.version >= 107) or (self.version == 1):
            self.setInfo('EC restart')
            if self.version < 110:
                self.sendCmd(211)
            elif self.version < 141:
                self.sendCmd(210)
            else:
                self.sendCmd(self.RESTART)
                time.sleep(2.0)
            #(cmdId, status, PL) = self.recvAck()
            return 1
        else:
            self.setInfo('restart not possible in this EC version')
        return 0
    #---------------------------------------
    def setControlMode(self, cab=-1, mode=MODE_AUTO):
        self.sendCmd(self.EC_SET_CTRL_MODE, cab, mode)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('SetControlMode cab %d to %d' %(cab, mode))
        return
    #---------------------------------------
    def setPower(self, pwr=-1, state=PWR_ON):
        if ((pwr == 48) or (pwr == -1)):
            self.sendCmd(self.EC_SET_48, 0, state)
            (cmdId, status, PL) = self.recvAck()
            self.setInfo('Power Set 48V to %d' %(state))
        if ((pwr == self.LCU) or (pwr == -1)):
            self.sendCmd(self.EC_SET_230, 0, state)
            (cmdId, status, PL) = self.recvAck()
            self.setInfo('Power Set LCU to %d' %(state))
        return
    #---------------------------------------
    def resetPower(self, pwr=-1):
        if ((pwr == 48) or (pwr == -1)):
            self.sendCmd(self.EC_RESET_48, 0, 0)
            (cmdId, status, PL) = self.recvAck()
            self.setInfo('PowerReset 48V')
        if ((pwr == self.LCU) or (pwr == -1)):
            self.sendCmd(self.EC_RESET_230, 0, 0)
            (cmdId, status, PL) = self.recvAck()
            self.setInfo('PowerReset LCU')

    #---------------------------------------
    def resetTrip(self):
        self.sendCmd(self.EC_RESET_TRIP, -1, 0)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('Reset Trip System')

   #---------------------------------------
    ## search for new setpoint, works only in control mmode 1
    def seekNewSetpoint(self, cab=-1):
        self.sendCmd(self.EC_SET_MODE, cab, MODE_SEEK)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('Find newSetpoint cab %d' %(cab))
    #---------------------------------------
    ## set new setpoint, works only in manual mode
    def setTemperature(self, cab=-1, temp=20.0):
        temperature = int(temp * 100.)
        self.sendCmd(self.EC_SET_TEMP, cab, temperature)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('SetTemperature cab %d to %4.2f' %(cab,temp))
    #---------------------------------------
    ## set new setpoint, works only in manual mode
    def setFans(self, cab=-1, fans=0x0f):
        self.sendCmd(self.EC_SET_FANS, cab, fans)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('SetFans cab %d to %X' %(cab,fans))
    #---------------------------------------
    ## set door control to on(1) or off(0)
    def setDoorControl(self, cab=-1, state=1):
        self.sendCmd(self.EC_SET_DOOR_CTRL, cab, state)
        (cmdId, status, PL) = self.recvAck()
        self.setinfo('SetDoorControl cab %d to %d' %(cab, state))
    #---------------------------------------
    ## set hum control to on(1) or off(0)
    def setHumControl(self, cab=-1, state=1):
        self.sendCmd(self.EC_SET_HUM_CTRL, cab, state)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('SetHumidityControl cab %d to %d' %(cab, state))
    #---------------------------------------
    def setHeater(self, mode=0):
        self.sendCmd(self.EC_SET_HEATER, -1, mode)
        (cmdId, status, payload) = self.recvAck()
        
        if (mode == self.MODE_ON): self.setInfo('heater is turned ON')
        if (mode == self.MODE_OFF): self.setInfo('heater is turned OFF')
        if (mode == self.MODE_AUTO): self.setInfo('heater set to AUTO')
    #---------------------------------------
    def getVersion(self):
        self.sendCmd(self.EC_VERSION)
        (cmdId, status, PL) = self.recvAck()
        
        version = int((PL[0]*100)+(PL[1]*10)+PL[2])
        versionstr = 'V%d.%d.%d' %(PL)
        self.version = version
        self.versionstr = versionstr
        self.setInfo('EC software version %d.%d.%d' %(PL))
        return version, versionstr
    #---------------------------------------
    def getStatus(self):
        ec_mode = ('OFF','ON','AUTO','MANUAL','STARTUP','AUTO-SEEK','ABSENT')
        fan = ('.  .  .  .','.  .  .  .','.  2  .  .','1  2  .  .',\
               '.  .  3  .','.  .  .  .','.  2  3  .','1  2  3  .',\
               '.  .  .  .','.  .  .  .','.  .  .  .','.  .  .  .',\
               '.  .  3  4','.  .  .  .','.  2  3  4','1  2  3  4')
        
        door = ('CLOSED','FRONT_OPEN','BACK_OPEN','ALL_OPEN')
        fanstate = ('BAD | BAD ','GOOD| BAD ','BAD | GOOD','GOOD| GOOD')
        fanestate= ('OFF | OFF ','ON  | OFF ','OFF | ON  ','ON  | ON  ')
        onoff = ('OFF','ON')
        badok = ('BAD','OK')
       
        # get information from EC
        self.sendCmd(self.EC_CTRL_TEMP)
        (cmdId, status, PL1) = self.recvAck()
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL2) = self.recvAck()
        if len(PL1) == 0 or len(PL2) == 0: return     
        # fill lines with data    
        lines = []

        if self.nCabs == 4:
            lines.append('temperature cab3 = %5.2f' %(PL2[2]/100.))
            lines.append('humidity cab3    = %5.2f' %(PL2[3]/100.))
            lines.append('cabinet fans     = all on')
            lines.append('heater state     = %s' %(onoff[PL2[(3*7)+6]]))
            lines.append('power 48V state  = %s' %(onoff[(PL2[28] & 1)]))
            lines.append('power LCU state  = %s' %(onoff[(PL2[28] >> 1)]))
            lines.append('lightning state  = %s' %(badok[(PL2[29] & 1)]))

        else:
            lines.append('            |')
            lines.append('mode        |')
            lines.append('status      |')
            lines.append('set point   |')
            lines.append('temperature |')
            lines.append('humidity    |')
            lines.append('fans        |')
            lines.append('fane        |')
            lines.append('fans state  |')
            lines.append('doors       |')
            lines.append('heater      |')

            for nCab in range(self.nCabs):
                cab = self.cabs[nCab]
                lines[0] += '  cabinet %1d |' %(cab)
                lines[1] += '%11s |' %(ec_mode[PL2[(cab*7)]])
                lines[2] += '     %#06x |' %(PL2[(cab*7)+1])
                lines[3] += '%11.2f |' %(PL1[cab]/100.)
                lines[4] += '%11.2f |' %(PL2[(cab*7)+2]/100.)
                lines[5] += '%11.2f |' %(PL2[(cab*7)+3]/100.)
                lines[6] += '%11s |' %(fan[(PL2[(cab*7)+4]&0x0f)])
                lines[7] += '%11s |' %(fanestate[(PL2[(cab*7)+4]>>4)&0x3])  
                lines[8] += '%11s |' %(fanstate[(PL2[(cab*7)+4]>>6)&0x3])  
                lines[9] += '%11s |' %(door[(PL2[(cab*7)+5]&0x03)])
                if (cab != 3):
                   lines[10] += '%11s |' %('none')
                else:
                   lines[10] += '%11s |' %(onoff[PL2[(cab*7)+6]])

            if self.nCabs == 1: i = 7
            else: i = 28
            lines.append('power 48V state  = %s' %(onoff[(PL2[i] & 1)]))
            lines.append('power LCU state  = %s' %(onoff[(PL2[i] >> 1)]))
            lines.append('lightning state  = %s' %(badok[(PL2[i+1] & 1)]))
        
        # print lines to screen or file, see printInfo
        info = 'status %s (%s)     %s ' %(self.station, self.versionstr, time.asctime())
        self.setInfo('-' * len(info))
        self.addInfo(info)
        self.addInfo('-' * len(info))
        for line in lines:
            self.addInfo(line)
        # print data to file if selected
        if (self.printToFile == 1):
            tm = time.gmtime()
            filename = '%s_%d%02d%02d.dat' %(self.station,tm.tm_year, tm.tm_mon, tm.tm_mday)
            df = open(filename, mode='a')
            df.write('%f ' %(time.time()))
            for cab in self.cabs:
                # print cabnr, setpoint, temperature, humidity, fansstate, heaterstate
                df.write('[%d] %3.2f %3.2f %3.2f %d %d ' %\
                        ( cab, PL1[cab]/100., PL2[(cab*7)+2]/100., PL2[(cab*7)+3]/100.,
                          PL2[(cab*7)+4], PL2[(cab*7)+6]))
            df.write('\n')
            df.close()
    #---------------------------------------
    def getTripStatus(self):
        # get information from EC
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL) = self.recvAck()
        state = False
        if (PL[1] & 0x1000):
            self.addInfo('trip in cabinet 0')
            state = True
        if (PL[8] & 0x1000):
            self.addInfo('trip in cabinet 1')
            state = True
        if (PL[22] & 0x1000):
            self.addInfo('trip in cabinet 3')
            state = True
        
        if (PL[1] & 0x6000):
            self.addInfo('warning in cabinet 0')
            state = True
        if (PL[8] & 0x6000):
            self.addInfo('warning in cabinet 1')
            state = True
        if (PL[22] & 0x6000):
            self.addInfo('warning in cabinet 3')
            state = True

        if (state == False):
            self.addInfo('NO trips available')
        return(state)
    
    #---------------------------------------
    def getPowerStatus(self):
        state = ('OFF','ON')
        # get information from EC
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL) = self.recvAck()
        
        self.addInfo('Power: 48V = %s, LCU = %s' %(state[(PL[28] & 1)], state[(PL[28] >> 1)]))
        
    #---------------------------------------
    def getControlTemp(self):
        self.sendCmd(self.EC_CTRL_TEMP)
        (cmdId, status, PL) = self.recvAck()
        lines = []
        lines.append('                 |')
        lines.append('min control temp |')
        
        for cab in self.cabs:
            lines[0] += '  cab-%1d |' %(cab)
            lines[1] += '%9.2f |' %(PL[cab]/100.)
    #---------------------------------------    
    def getSettings(self):
        self.sendCmd(self.EC_SETTINGS)
        (cmdId, status, PL) = self.recvAck()
            #self.info =  len(PL))
        # fill lines with data    
        lines = []
        lines.append('                 |')
        lines.append('start side       |')
        lines.append('balance point    |') 
        lines.append('max hour change  |')
        lines.append('seek time        |')
        lines.append('max seek change  |')
        lines.append('min control temp |')
        lines.append('max control temp |')
        lines.append('heater temp      |')
        lines.append('warn1 temp       |')
        lines.append('warn2 temp       |')
        lines.append('trip temp        |')
        lines.append('max humidity     |')
        lines.append('trip humidity    |')


        for nCab in range(self.nCabs):
            cab = self.cabs[nCab]
            lines[0] += '    cab-%1d |' %(cab)
            lines[1] += '%9d |'    %(PL[(cab*13)+10])
            lines[2] += '%9d |'    %(PL[(cab*13)+9])
            lines[3] += '%9.2f |'  %(PL[(cab*13)+8]/100.)
            lines[4] += '%9d |'    %(PL[(cab*13)+11])
            lines[5] += '%9.2f |'  %(PL[(cab*13)+12]/100.)
            lines[6] += '%9.2f |'  %(PL[(cab*13)+0]/100.)
            lines[7] += '%9.2f |'  %(PL[(cab*13)+1]/100.)
            lines[8] += '%9.2f |'  %(PL[(cab*13)+2]/100.)
            lines[9] += '%9.2f |'  %(PL[(cab*13)+3]/100.)
            lines[10] += '%9.2f |'  %(PL[(cab*13)+4]/100.)
            lines[11] += '%9.2f |'  %(PL[(cab*13)+5]/100.)
            lines[12] += '%9.2f |' %(PL[(cab*13)+6]/100.)
            lines[13] += '%9.2f |' %(PL[(cab*13)+7]/100.)
        
        # print lines to screen or file, see printInfo
        self.addInfo('=== Station settings ===')
        for line in lines:
            self.addInfo(line)

    #---------------------------------------
    def setSetting(self, cmd=0, cab=-1, val=0):
        self.sendCmd(cmd, cab, val)
        (cmdId, status, PL) = self.recvAck()
    
    #---------------------------------------    
    def getSensorSettings(self):
        self.sendCmd(self.SENSOR_SETTINGS)
        (cmdId, status, PL) = self.recvAck()
        #self.info =  len(PL))
        # fill lines with data    
        lines = []
        lines.append('                     |')
        lines.append('temperature offset   |')
        lines.append('temperature multiply |')    
        lines.append('humidity offset      |')
        lines.append('humidity multiply    |')
        
        for nCab in range(self.nCabs):
            cab = self.cabs[nCab]
            lines[0] += '    cab-%1d |' %(cab)
            lines[1] += '%9.2f |' %(PL[(cab*4)+0]/100.)
            lines[2] += '%9.2f |' %(PL[(cab*4)+1]/100.)
            lines[3] += '%9.2f |' %(PL[(cab*4)+2]/100.)
            lines[4] += '%9.2f |' %(PL[(cab*4)+3]/100.)
        
        # print lines to screen or file, see printInfo
        self.addInfo('=== Station sensor settings ===')
        for line in lines:
            self.addInfo(line)
    
    #---------------------------------------
    def sendFlashEraseCmd(self):
        try:
            if (self.version < 141):
                cmd = struct.pack('hHH', 200, 0, 0)
            else:
                cmd = struct.pack('hHH', self.FLASH_ERASE, 0, 0)
            self.sck.send(cmd)
        except socket.error:
            self.setInfo("socket error, try to reconnect")
            self.disconnectHost()
            self.connectToHost()
        return
    #---------------------------------------
    def sendFlashWriteCmd(self, sector=0, offset=0, data=''):
        try:
            if (self.version < 141):
                cmd = struct.pack('hHH', 201, sector, offset)
            else:
                cmd = struct.pack('hHH', self.FLASH_WRITE, sector, offset)
            if (len(data) > 0):
                if (len(data) < 1024):
                    data = data + ('\xFF' * (1024 - len(data)))
                cmd = cmd + data
            self.sck.send(cmd)
        except socket.error:
            self.setInfo("socket error, try to reconnect")
            self.disconnectHost()
            self.connectToHost()
        return
    #---------------------------------------
    def recvFlashAck(self):
        socketError = False    
        try:
            self.sck.settimeout(5.0)
            data = self.sck.recv(4)
        except socket.error:
            socketError = True
            self.setInfo("socket error, try to reconnect")
            self.disconnectHost()
            self.connectToHost()
        if socketError:
            return(0,-1)
            
        header = struct.unpack('hh', data)
        cmdId = header[0]
        status = header[1]
        return (cmdId, status) 

