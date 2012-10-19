## P.Donker ASTRON februari 2011
## EC IS status module

import socket
import struct
import time

def getIP():
    # get ip-adres of LCU
    local_host = socket.gethostbyname(socket.gethostname())
    ip = local_host.split('.')
    if ip[0] == '10' and ip[1] == '209':
        # if LCU adress make it EC adress
        return(local_host[:local_host.rfind('.')+1]+'3')
    return(None)

class EC:
    # cmdIDs from TCP PROTOCOL ec-controller
    EC_NONE             = 0
    EC_STATUS           = 1
    EC_CTRL_TEMP        = 3
    EC_VERSION          = 5
    EC_SET_HEATER       = 17
    EC_RESET_48         = 22
    EC_RESET_LCU        = 27

    PWR_OFF      = 0
    PWR_ON       = 1
    P48		     = 48
    LCU          = 230

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
    def resetPower(self):
        self.sendCmd(self.EC_RESET_48, 0, 0)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('PowerReset 48V')
    
    #---------------------------------------
    def resetLCU(self):
        self.sendCmd(self.EC_RESET_LCU, 0, 0)
        (cmdId, status, PL) = self.recvAck()
        self.setInfo('PowerReset LCU')
        
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

    #---------------------------------------
    def getStatusData(self):
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL2) = self.recvAck()
        return PL2

    def getStatus(self):
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
        lines.append('temperature cab3 = %5.2f' %(PL2[2]/100.))
        lines.append('humidity cab3    = %5.2f' %(PL2[3]/100.))
        lines.append('heater state     = %s' %(onoff[PL2[(3*7)+6]]))
        lines.append('power 48V state  = %s' %(onoff[(PL2[28] & 1)]))
        lines.append('power LCU state  = %s' %(onoff[(PL2[28] >> 1)]))
        lines.append('lightning state  = %s' %(badok[(PL2[29] & 1)]))

        # print lines to screen or file, see printInfo
        info1 = ' %s      (EC %s)' %(self.station, self.versionstr)
        info2 = ' %s ' %(time.asctime())
        self.setInfo('-' * len(info2))
        self.addInfo(info1)
        self.addInfo(info2)
        self.addInfo('-' * len(info2))
        for line in lines:
            self.addInfo(line)
   
    #---------------------------------------
    def getPowerStatus(self):
        state = ('OFF','ON')
        # get information from EC
        self.sendCmd(self.EC_STATUS)
        (cmdId, status, PL) = self.recvAck()
        
        self.addInfo('Power: 48V = %s, LCU = %s' %(state[(PL[28] & 1)], state[(PL[28] >> 1)]))

