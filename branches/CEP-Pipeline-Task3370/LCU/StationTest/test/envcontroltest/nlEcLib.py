## P.Donker ASTRON februari 2011
## EC NL status module

import socket
import struct
import time

def getIP():
    # get ip-adres of LCU
    local_host = socket.gethostbyname(socket.gethostname())
    ip = local_host.split('.')
    if ip[0] == '10' and ip[1] == '151':
        # if LCU adress make it EC adress
        return(local_host[:local_host.rfind('.')+1]+'3')
    return(None)

class EC:
    # cmdIDs from TCP PROTOCOL ec-controller
    EC_NONE             = 0
    EC_STATUS           = 1
    EC_CTRL_TEMP        = 3
    EC_VERSION          = 5
    EC_STATION_INFO     = 6

    printToScreen = False
    host = None
    station = None
    port = 10000
    sck = None
    logger = False
    info = ''
    version = 0
    versionstr = 'V-.-.-'
    
    def __init__(self, addr='0.0.0.0'):
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
        self.setInfo("closing %s" %(self.host))
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
    def getVersion(self):
        self.sendCmd(self.EC_VERSION)
        (cmdId, status, PL) = self.recvAck()
        try:
            version = int((PL[0]*100)+(PL[1]*10)+PL[2])
            versionstr = 'V%d.%d.%d' %(PL)
            self.version = version
            self.versionstr = versionstr
            self.setInfo('EC software version %d.%d.%d' %(PL))
        except:
            version = 0
            versionstr = 'V0.0.0'
            self.version = version
            self.versionstr = versionstr
            self.setInfo('EC software version 0.0.0')

        return version, versionstr

    def getStationInfo(self):
        stationType = ('Unknown','NAA','Unknown','LOFAR NL','LOFAR IS','Unknown')
        wxt520Text = ('not available','available')
        self.sendCmd(self.EC_STATION_INFO)
        (cmdId, status, PL) = self.recvAck()
        
        type = int(PL[0])
        wxt520 = int(PL[1])
        self.stationtype = type
        self.wxt520present = wxt520
        self.setInfo('station type: %s,   wxt520: %s' %\
            (stationType[type], wxt520Text[wxt520]))
        return type, wxt520
    #---------------------------------------
    def getStatusData(self):
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL2) = self.recvAck()
        return PL2
    
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
        badok = ('N.A.','OK')
       
        # get information from EC
        self.sendCmd(self.EC_CTRL_TEMP)
        (cmdId, status, PL1) = self.recvAck()
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL2) = self.recvAck()
        if len(PL1) == 0 or len(PL2) == 0: return     
        # fill lines with data    
        lines = []

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
        cabs = [0,1,3]
        for nCab in range(3):
            cab = cabs[nCab]
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

        i = 28
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
