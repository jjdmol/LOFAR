#!/usr/bin/python

## LOFAR script
## for reset, turn on or off the power to 48V or LCU
## P.Donker (ASTRON) 20-10-2009


## DO NOT CHANGE THINGS BELOW THIS LINE ##
## ------------------------------------ ##


import socket
import time
import struct
import sys
import string

VERSION = '1.0.0' # version of this script
 
if len(sys.argv) < 3:
    print '--------------------------------------------'
    print 'Error: no arguments found'
    print '--------------------------------------------'
    print 'usage power.py host cmd'
    print '    host = this (get IP based on LCU)'
    print '    host = ip (set ip manualy)'
    print '    cmd  = status (show status)'
    print '    cmd  = resetTrip (reset temperature trip system)'
    print '    cmd  = reset48 (reset 48V power supply)'
    print '    cmd  = resetLCU (reset LCU power supply)'
    print '    cmd  = on48 (turn on 48V power supply)'
    print '    cmd  = off48 (turn off 48V power supply)'
    print '    cmd  = onLCU (turn on LCU power supply)'
    print '    cmd  = offLCU (turn off LCU power supply)'
    print '*only 1 cmd per power supply can be given'
    print '--------------------------------------------'
    exit(0)


PORT = 10000            # Gateway port
ecSck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

if str(sys.argv[1]) == 'this':
    # get ip-adres of LCU
    LOCAL_HOST = socket.gethostbyname(socket.gethostname())
    if LOCAL_HOST.find('10.151.1') == -1 and  LOCAL_HOST.find('10.209.1') == -1:
        print 'Error: this script can only run on a LCU'
        exit(0)

    STATION = socket.gethostname()
    # change to ec adres
    HOST = LOCAL_HOST[:LOCAL_HOST.rfind('.1')] + '.3'
else:
    if str(sys.argv[1]).count('.') == 3:
        HOST = str(sys.argv[1])
        STATION = HOST
        if HOST.rfind('.3') > 7:
           STATION = HOST[:HOST.rfind('.3')] + '.1'
        if HOST.rfind('.1') > 7:
           HOST = HOST[:HOST.rfind('.1')] + '.3'
    else:
        STATION = str(sys.argv[1])[:5]
        HOST = socket.gethostbyname(STATION+'EC')



doResetTrip   = 0
# select only 1 
doReset48V    = 0
doPowerOn48V  = 0
doPowerOff48V = 0
# select only 1
doResetLCU    = 0
doPowerOnLCU  = 0
doPowerOffLCU = 0

for arg in sys.argv[2:]:
    cmd = str(arg)
    if cmd == 'resetTrip':
        doResetTrip = 1
    if cmd == 'reset48':
        doReset48V = 1
    if cmd == 'resetLCU':
        doResetLCU = 1
    if cmd == 'on48':
        doPowerOn48V = 1
    if cmd == 'off48':
        doPowerOff48V = 1
    if cmd == 'onLCU':
        doPowerOnLCU = 1
    if cmd == 'offLCU':
        doPowerOffLCU = 1


# === TCP PROTOCOL from controller ===

EC_NONE       = 0
EC_STATUS     = 1
EC_SET_48     = 20
EC_RESET_48   = 22
EC_SET_230    = 25
EC_RESET_230  = 27
EC_RESET_TRIP = 28
EC_SET_SECOND = 115

PWR_OFF = 0
PWR_ON  = 1
LCU     = 230

#---------------------------------------
# connect to station EC_controller
def connectToHost():
    print "connecting to %s(%s)\n" %(STATION, HOST)
    ecSck.connect((HOST, PORT))
    ecSck.settimeout(5.0)
#---------------------------------------
def disconnectHost():
    ecSck.close()
#---------------------------------------
def sendCmd(cmdId=EC_NONE, cab=-1, value=0):
    if (cmdId == EC_NONE):
        return (false)
    cmd = struct.pack('hhh', cmdId, cab, int(value))
    ecSck.send(cmd)
#---------------------------------------
def recvAck():
    data = ecSck.recv(6)
    header = struct.unpack('hhh', data)
    cmdId = header[0]
    status = header[1]
    PLSize = header[2]
    if (PLSize > 0):
        data = ecSck.recv(PLSize)
        fmt = 'h' * int(PLSize / 2)
        PL = struct.unpack(fmt, data)
    else:
        PL = []
    return (cmdId, status, PL) 
#---------------------------------------
def setSecond(sec=0):
    sendCmd(EC_SET_SECOND, 0, sec)
    (cmdId, status, PL) = recvAck()
#---------------------------------------
def waitForSync():
    while ((time.gmtime()[5] % 10) != 7):
        time.sleep(0.5)
    time.sleep(1.0)
#---------------------------------------
def waitForUpdate():
    while ((time.gmtime()[5] % 10) != 3):
        time.sleep(0.5)
    time.sleep(1.0)
#---------------------------------------
def printPowerState():
    state = ('OFF','ON')
    sendCmd(EC_STATUS)
    (cmdId, status, PL) = recvAck()
    print 'power 48V state = %s' %(state[(PL[28] & 1)])
    print 'power LCU state = %s' %(state[(PL[28] >> 1)])
    tripState = False
    if (PL[1] & 0x1000):
        print 'max trip in cabinet 0'
        tripState = True
    if (PL[8] & 0x1000):
        print 'max trip in cabinet 1'
        tripState = True
    if (PL[22] & 0x1000):
        print 'max trip in cabinet 3'
        tripState = True
    if (tripState == False):
        print 'NO trips available'
  
#---------------------------------------
def setPower(pwr=-1, state=PWR_ON):
    waitForSync()
    if ((pwr == 48) or (pwr == -1)):
        sendCmd(EC_SET_48, 0, state)
        (cmdId, status, PL) = recvAck()
        print 'Power Set 48V to %d' %(state)
    if ((pwr == LCU) or (pwr == -1)):
        sendCmd(EC_SET_230, 0, state)
        (cmdId, status, PL) = recvAck()
        print 'Power Set LCU to %d' %(state)
    waitForUpdate()
    printPowerState()
    print ''
#---------------------------------------
def resetPower(pwr=-1):
    waitForSync()
    if ((pwr == 48) or (pwr == -1)):
        sendCmd(EC_RESET_48, 0, 0)
        (cmdId, status, PL) = recvAck()
        print 'PowerReset 48V'
    if ((pwr == LCU) or (pwr == -1)):
        sendCmd(EC_RESET_230, 0, 0)
        (cmdId, status, PL) = recvAck()
        print 'PowerReset LCU'
    waitForUpdate()
    printPowerState()
    waitForUpdate()
    printPowerState()
    print ''
#---------------------------------------
def printTripState():
    state = False
    sendCmd(EC_STATUS)
    (cmdId, status, PL) = recvAck()
    if (PL[1] & 0x1000):
        print 'max trip in cabinet 0'
        state = True
    if (PL[8] & 0x1000):
        print 'max trip in cabinet 1'
        state = True
    if (PL[22] & 0x1000):
        print 'max trip in cabinet 3'
        state = True
    if (state == False):
        print 'NO trips available'
    return (state)

#---------------------------------------
def resetTrip():
    if (printTripState() == True):
        waitForSync()
        sendCmd(EC_RESET_TRIP, -1, 0)
        (cmdId, status, PL) = recvAck()
        print 'Reset trip system'
        waitForUpdate()
        printTripState()
    print ''


##=======================================================================
## start of main program
##=======================================================================
if __name__ == "__main__":
    
    logfile = open('ecPowerActions.log','a')
    
    if doResetTrip or doReset48V or doResetLCU: 
        name = raw_input('Enter your personal name: ')
    if doPowerOn48V or doPowerOff48V or doPowerOnLCU or doPowerOffLCU:
        name = raw_input('Enter your personal name: ')

    if ((doReset48V + doPowerOn48V + doPowerOff48V) > 1):
        print 'error more than 1 48V cmd selected'
        exit(-1)
    if ((doResetLCU + doPowerOnLCU + doPowerOffLCU) > 1):
        print 'error more than 1 LCU cmd selected'
        exit(-1)
   
    connectToHost()
    time.sleep(1.0)
    setSecond(int(time.gmtime()[5]))
    
    ## do not change if statements
    printPowerState()
    print ''
    if (doResetTrip == 1):
        logfile.write('%sc, %s, ecResetTrip by %s\n' %(string.upper(STATION), time.asctime(), name))
        resetTrip()
    if (doReset48V == 1):
        logfile.write('%sc, %s, ecReset48 by %s\n' %(string.upper(STATION), time.asctime(), name))
        resetPower(48)
    if (doResetLCU == 1):
        logfile.write('%sc, %s, ecResetLCU by %s\n' %(string.upper(STATION), time.asctime(), name))
        resetPower(LCU)
    if (doPowerOn48V == 1):
        logfile.write('%sc, %s, ec48On by %s\n' %(string.upper(STATION), time.asctime(), name))
        setPower(48,PWR_ON)
    if (doPowerOff48V == 1):
        logfile.write('%sc, %s, ec48Off by %s\n' %(string.upper(STATION), time.asctime(), name))
        setPower(48,PWR_OFF)
    if (doPowerOnLCU == 1):
        logfile.write('%sc, %s, ecLcuOn by %s\n' %(string.upper(STATION), time.asctime(), name))
        setPower(LCU,PWR_ON)
    if (doPowerOffLCU == 1):
        logfile.write('%sc, %s, ecLcuOff by %s\n' %(string.upper(STATION), time.asctime(), name))
        setPower(LCU,PWR_OFF)

    logfile.close()
    disconnectHost()
