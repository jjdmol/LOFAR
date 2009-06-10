#!/usr/bin/python

import socket
import time
import struct

VERSION = '1.0.2' # version of this script    

## to use other commands, see playground on the bottom of this file

##== change only next settings ====================================
printToScreen = 1
printToFile = 0
printDataToFile = 0     # save setpoint, temp, humidity to data.txt

# select test modes
doCheckRelayPanel = 0
doCheckFans = 0
doCheckDoors = 0
doChangeSettings = 0   # fill in table below

#HOST = '192.168.178.111'   # Home TempControl
#HOST = '10.151.19.2'   # CS010c TempControl
#HOST = '10.151.134.3'   # RS106c TempControl
HOST = '10.151.162.3'  # RS302c TempControl
#HOST = '10.151.66.3'  # CS030c TempControl
#HOST = '10.151.161.3'  # CS301c TempControl
#HOST = '10.87.2.239'   # ASTRON TempControl on desk PD

# settings for (cab0, cab1, cab2, cab3)
# for LOFAR NL stations cab2 is not available
# cab0 = rack with receiver 0, cab3 = always control rack
MinTemp      = (0.00, 0.00, 0.00, 10.0)
MaxTemp      = (35.0, 35.0, 35.0, 30.0)
MinMinTemp   = (0.00, 0.00, 0.00, 8.0)
MaxMaxTemp   = (40.0, 40.0, 40.0, 35.0)
MaxHum       = (90.0, 90.0, 90.0, 80.0)
MaxMaxHum    = (95.0, 95.0, 95.0, 85.0)
ControlSpan  = (2.00, 2.00, 2.00, 2.00)
MaxDayChange = (1.00, 1.00, 1.00, 1.00)
#==================================================================

PORT = 10000            # Gateway port
ecSck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# === TCP PROTOCOL from controller ===
EC_NONE             = 0
EC_STATUS           = 1
EC_SETTINGS         = 2
EC_CTRL_TEMP        = 3
EC_VERSION          = 5
EC_SET_MODE         = 10
EC_SET_TEMP         = 15

EC_SET_48           = 20
EC_RESET_48         = 22
EC_SET_230          = 25
EC_RESET_230        = 27   

EC_SET_DOOR_CTRL    = 50
EC_SET_HUM_CTRL     = 52

EC_SET_MAX_TEMP     = 100
EC_SET_MIN_TEMP     = 101
EC_SET_MAXMAX_TEMP  = 102
EC_SET_MINMIN_TEMP  = 103
EC_SET_MAX_HUM      = 105
EC_SET_MAXMAX_HUM   = 106
EC_SET_CTRL_SPAN    = 110
EC_SET_MAX_CHANGE   = 111

EC_SET_SECOND       = 115
EC_SET_FANS         = 116


MODE_OFF     = 0
MODE_ON      = 1
MODE_AUTO    = 2
MODE_MANUAL  = 3
MODE_STARTUP = 4
PWR_OFF      = 0
PWR_ON       = 1
LCU          = 230

# used variables
cabs =(0,1,3) # cabs in station
version = 0   # EC version
#---------------------------------------
# open files if needed
if (printToFile == 1):
    #(hostname, aliaslist, ipaddrlist) = socket.gethostbyaddr(HOST)
    #stationname = hostname.split('.')[0] 
    #print stationname
    filename = "tempctrl.txt"
    f = open(filename, mode='w')
#---------------------------------------
def closeFile():
    if (printToFile == 1):
        f.close()
#---------------------------------------
# print information to screen or file
def printInfo(info):
    if (printToScreen == 1):
        print info

    if (printToFile == 1):
        f.write(info)
        f.write('\n')
#---------------------------------------
# connect to station EC_controller
def connectToHost():
    info =  "connecting to %s on port %d\n" %(HOST, PORT)
    printInfo(info)
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
def setMode(cab=-1, mode=MODE_AUTO):
    sendCmd(EC_SET_MODE, cab, mode)
    (cmdId, status, PL) = recvAck()
    printInfo('SetMode cab %d to %d' %(cab, mode))
#---------------------------------------
def setPower(pwr=-1, state=PWR_ON):
    if ((pwr == 48) or (pwr == -1)):
        sendCmd(EC_SET_48, 0, state)
        (cmdId, status, PL) = recvAck()
        printInfo('Power Set 48V to %d' %(state))
    if ((pwr == LCU) or (pwr == -1)):
        sendCmd(EC_SET_230, 0, state)
        (cmdId, status, PL) = recvAck()
        printInfo('Power Set LCU to %d' %(state))
#---------------------------------------
def resetPower(pwr=-1):
    if ((pwr == 48) or (pwr == -1)):
        sendCmd(EC_RESET_48, 0, 0)
        (cmdId, status, PL) = recvAck()
        printInfo('PowerReset 48V')
    if ((pwr == LCU) or (pwr == -1)):
        sendCmd(EC_RESET_230, 0, 0)
        (cmdId, status, PL) = recvAck()
        printInfo('PowerReset LCU')

#---------------------------------------
## mode 1 = moving setpoint
## mode 2 = constant setpoint, preset to 25.0 C    
def setControlMode(cab=-1, mode=1):
    sendCmd(EC_SET_CTRL_MODE, cab, mode)
    (cmdId, status, PL) = recvAck()
    printInfo('SetControlMode cab %d to %d' %(cab, mode))
#---------------------------------------
## search for new setpoint, works only in control mmode 1
def seekNewSetpoint(cab=-1):
    if (version < 102):
        printInfo('to use this function update firmware')
        return
    sendCmd(EC_SET_MODE, cab, MODE_STARTUP)
    (cmdId, status, PL) = recvAck()
    printInfo('Find newSetpoint cab %d' %(cab))
#---------------------------------------
## set seek time in minutes
def setSeekTime(cab=-1, time=60):
    sendCmd(EC_SET_SEEK_TIME, cab, time)
    (cmdId, status, PL) = recvAck()
    printInfo('SetTemperature cab %d to %4.2f' %(cab,temp))
#---------------------------------------
## set max temperature change in given seek time
def setSeekChange(cab=-1, temp=5.0):
    temperature = int(temp * 100.)
    sendCmd(EC_SET_SEEK_CHANGE, cab, temperature)
    (cmdId, status, PL) = recvAck()
    printInfo('SetTemperature cab %d to %4.2f' %(cab,temp))
#---------------------------------------
## set new setpoint, works only in manual mode
def setTemperature(cab=-1, temp=20.0):
    temperature = int(temp * 100.)
    sendCmd(EC_SET_TEMP, cab, temperature)
    (cmdId, status, PL) = recvAck()
    printInfo('SetTemperature cab %d to %4.2f' %(cab,temp))
#---------------------------------------
## set new setpoint, works only in manual mode
def setFans(cab=-1, fans=0x0f):
    sendCmd(EC_SET_FANS, cab, fans)
    (cmdId, status, PL) = recvAck()
    printInfo('SetFans cab %d to %X' %(cab,fans))
#---------------------------------------
## set door control to on(1) or off(0)
def setDoorControl(cab=-1, state=1):
    sendCmd(EC_SET_DOOR_CTRL, cab, state)
    (cmdId, status, PL) = recvAck()
    printInfo('SetDoorControl cab %d to %d' %(cab, state))
#---------------------------------------
## set hum control to on(1) or off(0)
def setHumControl(cab=-1, state=1):
    sendCmd(EC_SET_HUM_CTRL, cab, state)
    (cmdId, status, PL) = recvAck()
    printInfo('SetHumidityControl cab %d to %d' %(cab, state))
#---------------------------------------
def getVersion():
    sendCmd(EC_VERSION)
    (cmdId, status, PL) = recvAck()
    version = int((PL[0]*100)+(PL[1]*10)+PL[2])
    printInfo('EC software version %d.%d.%d' %(PL))
    return version
#---------------------------------------
def getStatus():
    ec_mode = ('OFF','ON','AUTO','MANUAL','STARTUP','ABSENT')
    fan = ('.  .  .  .','.  .  .  .','.  2  .  .','1  2  .  .',\
           '.  .  3  .','.  .  .  .','.  2  3  .','1  2  3  .',\
           '.  .  .  .','.  .  .  .','.  .  .  .','.  .  .  .',\
           '.  .  3  4','.  .  .  .','.  2  3  4','1  2  3  4')
    
    door = ('CLOSED','FRONT_OPEN','BACK_OPEN','ALL_OPEN')
    onoff = ('OFF','ON')
    badok = ('BAD','OK')

    # get information from EC
    sendCmd(EC_CTRL_TEMP)
    (cmdId, status, PL1) = recvAck()
        
    sendCmd(EC_STATUS)
    (cmdId, status, PL2) = recvAck()

    # fill lines with data    
    lines = []
    lines.append('            |')
    lines.append('mode        |')
    lines.append('status      |')
    lines.append('set point   |')
    lines.append('temperature |')
    lines.append('humidity    |')
    lines.append('fans        |')
    lines.append('doors       |')
    lines.append('heater      |')

    for cab in cabs:
        lines[0] += '  cabinet %1d |' %(cab)
        lines[1] += '%11s |' %(ec_mode[PL2[(cab*7)]])
        lines[2] += '%11X |' %(PL2[(cab*7)+1])
        lines[3] += '%11.2f |' %(PL1[cab]/100.)
        lines[4] += '%11.2f |' %(PL2[(cab*7)+2]/100.)
        lines[5] += '%11.2f |' %(PL2[(cab*7)+3]/100.)
        lines[6] += '%11s |' %(fan[(PL2[(cab*7)+4]&0x0f)])
        lines[7] += '%11s |' %(door[(PL2[(cab*7)+5]&0x03)])
        if (cab != 3):
           lines[8] += '%11s |' %('none')
        else:
           lines[8] += '%11s |' %(onoff[PL2[(cab*7)+6]])

    lines.append('power 48V state = %s' %(onoff[(PL2[28] & 1)]))
    lines.append('power LCU state = %s' %(onoff[(PL2[28] >> 1)]))
    lines.append('lightning state = %s' %(badok[(PL2[29] & 1)]))
        
    # print lines to screen or file, see printInfo
    printInfo('=== Station status ===')
    for line in lines:
        printInfo(line)
    printInfo(' ')
    # print data to df if selected
    if (printDataToFile == 1):
        df = open("data.txt", mode='a')
        for cab in cabs:
            df.write(' %3.2f %3.2f %3.2f' %\
                    (PL1[cab]/100., PL2[(cab*7)+2]/100., PL2[(cab*7)+3]/100.))
        df.write('\n')
        df.close()
    
#---------------------------------------
def getControlTemp():
    sendCmd(EC_CTRL_TEMP)
    (cmdId, status, PL) = recvAck()
    lines = []
    lines.append('                 |')
    lines.append('min control temp |')
    
    for cab in cabs:
        lines[0] += '  cab-%1d |' %(cab)
        lines[1] += '%9.2f |' %(PL[cab]/100.)
#---------------------------------------    
def getSettings():
    sendCmd(EC_SETTINGS)
    (cmdId, status, PL) = recvAck()

    # fill lines with data    
    lines = []
    lines.append('                 |')
    lines.append('min control temp |')
    lines.append('max control temp |')
    lines.append('minmin temp      |')
    lines.append('maxmax temp      |')
    lines.append('max humidity     |')
    lines.append('maxmax humidity  |')
    lines.append('max day change   |')
    lines.append('control span     |')

    for cab in cabs:
        lines[0] += '    cab-%1d |' %(cab)
        lines[1] += '%9.2f |' %(PL[(cab*8)+0]/100.)
        lines[2] += '%9.2f |' %(PL[(cab*8)+1]/100.)
        lines[3] += '%9.2f |' %(PL[(cab*8)+2]/100.)
        lines[4] += '%9.2f |' %(PL[(cab*8)+3]/100.)
        lines[5] += '%9.2f |' %(PL[(cab*8)+4]/100.)
        lines[6] += '%9.2f |' %(PL[(cab*8)+5]/100.)
        lines[7] += '%9.2f |' %(PL[(cab*8)+6]/100.)
        lines[8] += '%9.2f |' %(PL[(cab*8)+7]/100.)

    # print lines to screen or file, see printInfo
    printInfo('=== Station settings ===')
    for line in lines:
        printInfo(line)
    printInfo(' ')
#---------------------------------------
def setSettings():
    for i in range(4):
        sendCmd(EC_SET_MIN_TEMP, i, int(MinTemp[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MAX_TEMP, i, int(MaxTemp[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MINMIN_TEMP, i, int(MinMinTemp[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MAXMAX_TEMP, i, int(MaxMaxTemp[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MAX_HUM, i, int(MaxHum[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MAXMAX_HUM, i, int(MaxMaxHum[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_CTRL_SPAN, i, int(ControlSpan[i]*100))
        (cmdId, status, payload) = recvAck()
        sendCmd(EC_SET_MAX_CHANGE, i, int(MaxDayChange[i]*100))
        (cmdId, status, payload) = recvAck()
#---------------------------------------
def checkFans():
    setMode(cab=-1, mode=MODE_ON)
    # fans and vane check
    printInfo('\nFan and Vane check')
    setFans(-1,0) # turn off all fans
    time.sleep(15.0)
    
    # check all cabinets
    for cab in cabs:
        # turn on fans in front door
        waitForSync()
        setFans(cab,2)
        waitForSync()
        setFans(cab,3)
               
        # check vane state
        waitForUpdate()
        waitForUpdate()
        sendCmd(EC_STATUS)
        (cmdId, status, PL) = recvAck()
        if (PL[(cab * 7) + 4] & 0x10):
            printInfo('Fans and vane of cab-%d front OK' %(cab))
        else:
            printInfo('Fans or vane of cab-%d front NOT OK' %(cab))
        setFans(-1,0) # turn off all fans
        
        # turn on fans in back door
        waitForSync()
        setFans(cab,4)
        waitForSync()
        setFans(cab,12)
        
        # check vane state
        waitForUpdate()
        waitForUpdate()
        sendCmd(EC_STATUS)
        (cmdId, status, PL) = recvAck()
        if (PL[(cab * 7) + 4] & 0x20):
             printInfo('Fans and vane of cab-%d back OK\n' %(cab))
        else:
            printInfo('Fans or vane of cab-%d back NOT OK\n' %(cab))
        setFans(-1,0) # turn off all fans
    setMode(cab=0, mode=MODE_AUTO)
#---------------------------------------
def checkDoors():
    printInfo('\nDoor contact check')
    printInfo('Close doors and press last doorcontact')

    doorOpenCheck = 0
    doorCloseCheck = 0
            
    doorsOpen = 6
    cycles = 0
    while ((doorsOpen > 0) and (cycles < 6)):
        doorsOpen = 0
        waitForUpdate()
        sendCmd(EC_STATUS)
        (cmdId, status, PL) = recvAck()
        for cab in range(4):
            if (cab == 2):
                continue
            if (PL[(cab * 7) + 5] & 1):
                doorsOpen += 1
            if (PL[(cab * 7) + 5] & 2):
                doorsOpen += 1
        cycles += 1
        printInfo('doors closed=%d' %(6 - doorsOpen))
    if (doorsOpen == 0):
        printInfo('all doors are closed, Open all doors')
        doorCloseCheck = 1
    else:
        printInfo('not all doors are closed')

    doorsOpen = 0
    cycles = 0
    while ((doorsOpen < 6) and (cycles < 6)):
        doorsOpen = 0
        waitForUpdate()
        sendCmd(EC_STATUS)
        (cmdId, status, PL) = recvAck()
        for cab in range(4):
            if (cab == 2):
                continue
            if (PL[(cab * 7) + 5] & 1):
                doorsOpen += 1
            if (PL[(cab * 7) + 5] & 2):
                doorsOpen += 1

        cycles += 1
        printInfo('doors open=%d' %(doorsOpen))
    if (doorsOpen == 6):
        printInfo('all doors are open')
        doorOpenCheck = 1
    else:
        printInfo('not all doors are open')

    
    if (doorOpenCheck and doorCloseCheck):
        printInfo('all door contacs OK\n')
    else:
        printInfo('door contact failure\n')
#---------------------------------------
def checkRelayPanel():
    printInfo('\nRelay panel check')
    resetPower(48)
    waitForUpdate()
    waitForUpdate()
    resetPower(230)
    waitForUpdate()
    waitForUpdate()
    # heater test
    sendCmd(EC_SET_MINMIN_TEMP, 3, int(30.0*100))
    (cmdId, status, payload) = recvAck()
    printInfo("heater test")
    waitForUpdate()
    waitForUpdate()
    sendCmd(EC_SET_MINMIN_TEMP, 3, int(8.0*100))
    (cmdId, status, payload) = recvAck()
    printInfo('Relay panel check done\n')
   

##=======================================================================
## start of main program
##=======================================================================
## do not delete next lines
connectToHost()
time.sleep(1.0)
setSecond(int(time.gmtime()[5]))
# version is used to check if function is available in firmware
version = getVersion()  
## do not change if statements
if (doCheckRelayPanel == 1):
    checkRelayPanel()
if (doCheckFans == 1):
    checkFans()
if (doCheckDoors == 1):
    checkDoors()
if (doChangeSettings == 1):
    printInfo('Change control settings\nold settings')
    getSettings()
    setSettings()
    printInfo('new settings')
    getSettings()
    printInfo('Done\n')
## end if statements
##-----------------------------------------------------------------------
## playground
## cab = -1(all) or 0,1,3
   
## search for new temperature setpoint
#seekNewSetpoint()

## set cab to mode 
## mode = MODE_OFF, MODE_ON, MODE_AUTO, MODE_MANUAL, MODE_STARTUP
#setMode(cab=-1, mode=MODE_MANUAL)

## set cab to given temp, only posible in manual mode
#setTemperature(cab=-1,temp=25.0)

## set cab to auto
#setMode(cab=-1, mode=MODE_AUTO)

## turn on fans of cab
## fans=bitfield(0000,0010,0011,0100,0110,0111,1100,1110,1111)
## lsb = fan1
#setFans(cab=-1,fans=0x0c)

## set door control on(1) or off(0)
#setDoorControl(cab=-1,state=1)

## get controller settings
#getSettings()

## reset or set power for 48V or LCU
#resetPower(48)
#resetPower(LCU)
#setPower(48,0)
#setPower(LCU,0)

stop = False
while (not stop):
    waitForUpdate()
    printInfo('====== %s ====================' %(time.asctime()) )
    getStatus()

##----------------------------------------------------------------------
## do not delete next lines
disconnectHost()
closeFile()



