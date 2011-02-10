#!/usr/bin/python

from eccontrol import *
from stations import *
import sys
import time

VERSION = '1.2.4' # version of this script    

## to use other commands, see playground on the bottom of this file

##== change only next settings ====================================

printToScreen = 1
printToFile = 0
#printDataToFile = 0     # save setpoint, temp, humidity to data.txt

# select test modes
doCheckRelayPanel = 0
doCheckFans = 0
doCheckDoors = 0
doChangeSettings = 0   # fill in table below

# settings for (cab0, cab1, cab2, cab3)
# for LOFAR NL stations cab2 is not available
# cab0 = rack with receiver 0, cab3 = always control rack
MaxHourChange= (0.50, 0.50, 0.50, 0.50) 
StartSide    = (1   , 1   , 1   , 0   ) 
BalancePoint = (20  , 20  , 20  , 20  ) 
SeekTime     = (15  , 15  , 15  , 15  )
SeekChange   = (10.0, 10.0, 10.0, 10.0)
MinCtrlTemp  = (10.0, 10.0, 10.0, 10.0)
MaxCtrlTemp  = (35.0, 35.0, 35.0, 35.0)
HeaterTemp   = (8.00, 8.00, 8.00, 8.00)
WarnTemp1    = (39.0, 39.0, 39.0, 39.0)
WarnTemp2    = (39.5, 39.5, 39.5, 39.5)
TripTemp     = (40.0, 40.0, 40.0, 40.0)
MaxHum       = (90.0, 90.0, 90.0, 80.0)
TripHum      = (95.0, 95.0, 95.0, 85.0)
##==================================================================


if len(sys.argv) == 1:
    print '--------------------------------------------'
    print 'Error: no arguments found'
    print '--------------------------------------------'
    print ''
    print 'usage power.py host'
    print '    host = this (get IP based on LCU)'
    print '    host = ip (set ip manualy)'
    print ''
    print '--------------------------------------------'
    print ''
    exit(0)

Station = str(sys.argv[1])

station = STATION()
stations = station.ec(Station)


# used variables
version = 0   # EC version
versionstr = 'V-.-.-'


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
def setSettings():
    for i in range(4):
        ec.setSetting(ec.SET_MAX_CHANGE, i, int(MaxHourChange[i]*100))
        ec.setSetting(ec.SET_START_SIDE, i, int(StartSide[i]))
        ec.setSetting(ec.SET_BALANCE_POINT, i, int(BalancePoint[i]))
        ec.setSetting(ec.SET_SEEK_TIME, i, int(SeekTime[i]))
        ec.setSetting(ec.SET_SEEK_CHANGE, i, int(SeekChange[i]*100))
        ec.setSetting(ec.SET_MIN_CTRL_TEMP, i, int(MinCtrlTemp[i]*100))
        ec.setSetting(ec.SET_MAX_CTRL_TEMP, i, int(MaxCtrlTemp[i]*100))
        ec.setSetting(ec.SET_HEATER_TEMP, i, int(HeaterTemp[i]*100))
        ec.setSetting(ec.SET_WARN1_TEMP, i, int(WarnTemp1[i]*100))
        ec.setSetting(ec.SET_WARN2_TEMP, i, int(WarnTemp2[i]*100))
        ec.setSetting(ec.SET_TRIP_TEMP, i, int(TripTemp[i]*100))
        ec.setSetting(ec.SET_MAX_HUM, i, int(MaxHum[i]*100))
        ec.setSetting(ec.SET_TRIP_HUM, i, int(TripHum[i]*100))
        
        
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
    # heater test
    setHeater(1)
    printInfo("heater test")
    waitForUpdate()
    waitForUpdate()
    setHeater(0)
    # power switch test
    resetPower(48)
    waitForUpdate()
    waitForUpdate()
    resetPower(230)
    waitForUpdate()
    waitForUpdate()
    printInfo('Relay panel check done\n')
    
   

##=======================================================================
## start of main program
##=======================================================================
## do not delete next lines
if __name__ == '__main__':
    for station in stations:
        host = station
#    (hostname, aliaslist, ipaddrlist) = socket.gethostbyaddr(host)
#    stationname = hostname.split('.')[0] 
#    print stationname
    print host
    nCabs = 3
    if host.find('10.209') != -1:
        nCabs = 4
    
    ec = EC(host,nCabs)
    ec.connectToHost()
    
    ec.printInfo(True)    
    ec.printFile(False)
   
    time.sleep(1.0)
    ## synchronize EC and PC
    #ec.setSecond(int(time.gmtime()[5]))
    # version is used to check if function is available in firmware
    version,versionstr  = ec.getVersion()  
    ## do not change if statements
    if (doCheckFans == 1):
        checkFans()
    if (doCheckDoors == 1):
        checkDoors()
    if (doCheckRelayPanel == 1):
        checkRelayPanel()
    if (doChangeSettings == 1):
        printInfo('Change control settings\nold settings')
        ec.getSettings()
        setSettings()
        printInfo('new settings')
        ec.getSettings()
        printInfo('Done\n')
    ## end if statements
    ##-----------------------------------------------------------------------
    ## playground
    ## cab = -1(all) or 0,1,3

    ## set cab to mode, default=MODE_AUTO 
    ## mode = MODE_OFF, MODE_ON, MODE_AUTO, MODE_MANUAL, MODE_STARTUP, MODE_SEEK
    ## MODE_STARTUP and MODE_SEEK will automatically return to MODE_AUTO
    #ec.setControlMode(cab=-1, mode=ec.MODE_MANUAL)
    #ec.setControlMode(cab=-1, mode=ec.MODE_OFF)
    #ec.setControlMode(cab=-1, mode=ec.MODE_ON)
       
    ## set cab to given temp, only posible in MODE_MANUAL
    #ec.setTemperature(cab=0,temp=33.00)
    #ec.setTemperature(cab=1,temp=28.50)
    #ec.setTemperature(cab=3,temp=28.50)
    #ec.setTemperature(cab=-1,temp=35.00)

    ## search for new temperature setpoint
    #ec.seekNewSetpoint()

    ## turn on fans of cab, only possible in MODE_ON
    ## fans=bitfield(0000,0010,0011,0100,0110,0111,1100,1110,1111)
    ## lsb = fan1
    #ec.setFans(cab=-1,fans=0x0c)
    #ec.setFans(cab=3,fans=0x0f)

    ## set door control on(1) or off(0)
    #ec.setDoorControl(cab=-1,state=1)

    ## set humidity control on(1) or off(0)
    #ec.setHumControl(cab=-1,state=1)

    ## reset or set power for 48V or LCU
    #ec.resetPower(48)
    #ec.resetPower(LCU)
    #ec.setPower(48,1)
    #ec.setPower(LCU,1)

    ## set Heater mode default=MODE_AUTO
    ## mode = MODE_OFF | MODE_ON | MODE_AUTO
    #ec.setHeater(ec.MODE_AUTO)

    ## restart controller, works from EC version 1.0.7
    #if ec.restart() == 1: stop = True

    ## reset trip system
    #ec.resetTrip()

    ## set cab to mode, default=MODE_AUTO 
    ## mode = MODE_OFF, MODE_ON, MODE_AUTO, MODE_MANUAL, MODE_STARTUP, MODE_SEEK
    ## MODE_STARTUP and MODE_SEEK will automatically return to MODE_AUTO
#    ec.setControlMode(cab=-1, mode=ec.MODE_AUTO)

    ## get controller settings
#    ec.getSettings()
#    setSettings()
#    ec.getSettings()
#    ec.setSetting(EC_SET_HUM_MULT,0,1.165)
#    ec.setSetting(EC_SET_HUM_MULT,1,1.165)
#    ec.setSetting(EC_SET_HUM_MULT,3,1.165)
#    ec.setSetting(EC_SET_HUM_MULT,0,1.0)
#    ec.setSetting(EC_SET_HUM_MULT,1,1.0)
#    ec.setSetting(EC_SET_HUM_MULT,3,1.0)
#    ec.getStatus()
#    ec.getSensorSettings()
    stop = False
    #stop = True
    while (not stop):
        #ec.waitForUpdate()
        ec.getStatus()
        time.sleep(10.0)

    ##----------------------------------------------------------------------
    ## do not delete next lines
    ec.disconnectHost()
    if (printToFile == 1):
        closeFile()


