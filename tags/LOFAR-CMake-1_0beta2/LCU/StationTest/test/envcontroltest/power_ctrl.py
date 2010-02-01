#!/usr/bin/python

import socket
import time
import struct

VERSION = '1.0.0' # version of this script

## select HOST and action

#HOST = '192.168.178.111'   # Home EC
#HOST = '10.151.19.2'   # CS010c EC
#HOST = '10.151.134.3'   # RS106c EC
#HOST = '10.151.162.3'  # RS302c EC
#HOST = '10.151.66.3'  # CS030c EC
#HOST = '10.151.161.3'  # CS301c EC
HOST = '10.87.2.239'   # ASTRON EC on desk PD


## to see active state set all to 0
# select only 1
doReset48V    = 0
doPowerOn48V  = 0
doPowerOff48V = 0
# select only 1
doResetLCU    = 0
doPowerOnLCU  = 0
doPowerOffLCU = 0



## DO NOT CHANGE THINGS BELOW THIS LINE ##
## ------------------------------------ ##



# === TCP PROTOCOL from controller ===
PORT = 10000            # Gateway port
ecSck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

EC_NONE       = 0
EC_STATUS     = 1
EC_SET_48     = 20
EC_RESET_48   = 22
EC_SET_230    = 25
EC_RESET_230  = 27
EC_SET_SECOND = 115

PWR_OFF = 0
PWR_ON  = 1
LCU     = 230

#---------------------------------------
# connect to station EC_controller
def connectToHost():
    print "connecting to %s on port %d\n" %(HOST, PORT)
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

##=======================================================================
## start of main program
##=======================================================================
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
if (doReset48V == 1):
    resetPower(48)
if (doResetLCU == 1):
    resetPower(LCU)
if (doPowerOn48V == 1):
    setPower(48,PWR_ON)
if (doPowerOff48V == 1):
    setPower(48,PWR_OFF)
if (doPowerOnLCU == 1):
    setPower(LCU,PWR_ON)
if (doPowerOffLCU == 1):
    setPower(LCU,PWR_OFF)

disconnectHost()
