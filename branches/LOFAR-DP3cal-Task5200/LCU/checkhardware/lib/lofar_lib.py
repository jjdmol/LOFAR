# lofar_lib

import os
import sys
import time
import logging
import socket
import struct
import string
from general_lib import sendCmd

os.umask(001)
lofar_version = '0514'

CoreStations          = ('CS001C','CS002C','CS003C','CS004C','CS005C','CS006C','CS007C','CS011C',\
                         'CS013C','CS017C','CS021C','CS024C','CS026C','CS028C','CS030C','CS031C',\
                         'CS032C','CS101C','CS103C','CS201C','CS301C','CS302C','CS401C','CS501C')

RemoteStations        = ('RS106C','RS205C','RS208C','RS210C','RS305C','RS306C','RS307C','RS310C',\
                         'RS406C','RS407C','RS409C','RS503C','RS508C','RS509C')

InternationalStations =	('DE601C','DE602C','DE603C','DE604C','DE605C','FR606C','SE607C','UK608C')


StationType = dict( CS=1, RS=2, IS=3 )

logger           = None
rcumode          = -1
active_delay_str = ('2,'*16)[:-1]


def init_lofar_lib():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger lofar_lib")
    if os.access(dataDir(), os.F_OK):
        removeAllDataFiles()
    else:
        os.mkdir(dataDir())


def dataDir():
    return (r'/localhome/stationtest/sb_data')

# remove all *.dat 
def removeAllDataFiles():
    if os.access(dataDir(), os.F_OK):
        files = os.listdir(dataDir())
        #print files
        for f in files:
            if f[-3:] == 'dat' or f[-3:] == 'nfo':
                os.remove(os.path.join(dataDir(),f))


# return station type
def getStationType(StID):
    if StID in CoreStations:
        return (StationType[CS])
    if StID in RemoteStations:
        return (StationType[RS])
    if StID in InternationalStations:
        return (StationType[IS])


# read from RemoteStation.conf file number of RSP and TB Boards
"""
#
# THIS FILE IS GENERATED, DO NOT MODIFY IT.
#
# RemoteStation.conf for CS002
#
# Describes the amount of available hardware on the station.
#

RS.STATION_ID  = 2
RS.N_RSPBOARDS = 12
RS.N_TBBOARDS  = 6
RS.N_LBAS      = 96
RS.N_HBAS      = 48
RS.HBA_SPLIT   = Yes
RS.WIDE_LBAS   = Yes
"""
def readStationConfig():
    f = open('/opt/lofar/etc/RemoteStation.conf', 'r')
    lines = f.readlines()
    f.close()
    
    ID = nRSP = nTBB = nLBA = nLBL = nLBH = nHBA = HBA_SPLIT = 0
    
    for line in lines:
        if (line[0] == '#') or (len(line) < 2):
            continue
        key, val = line.split('=')
        key = key.strip()
        val = val.strip()
        if key == "RS.STATION_ID":
            ID = int(val)
            continue
        if key == "RS.N_RSPBOARDS":
            nRSP = int(val)
            continue
        if key == "RS.N_TBBOARDS":
            nTBB = int(val)
            continue    
        if key == "RS.N_LBAS":
            nLBA = int(val)
            if nLBA == nRSP * 8:
                nLBL = nLBA / 2
                nLBH = nLBA / 2
            else:
                nLBL = 0
                nLBH = nLBA       
            continue
        if key == "RS.N_HBAS":
            nHBA = int(val)
            continue
        if key == "RS.HBA_SPLIT":
            if string.upper(val) == "YES":
                HBA_SPLIT = 1
                continue
    return(ID, nRSP, nTBB, nLBL, nLBH, nHBA, HBA_SPLIT)

    
# [lofarsys@RS306C stationtest]$ swlevel 2
# Going to level 2
# Starting RSPDriver
# Loading image 4 on RSPboard 0 ...
# Loading image 4 on RSPboard 1 ...
# Loading image 4 on RSPboard 2 ...
# Loading image 4 on RSPboard 3 ...
# Loading image 4 on RSPboard 4 ...
# Loading image 4 on RSPboard 5 ...
# Loading image 4 on RSPboard 6 ...
# Loading image 4 on RSPboard 7 ...
# RSPboard 8: Error requesting active firmware version (communication error)
# Loading image 4 on RSPboard 9 ...
# Loading image 4 on RSPboard 10 ...
# Loading image 4 on RSPboard 11 ...
# One or more boards have a communication problem; try reset the 48V
# root     21470     1  1 10:41 pts/2    00:00:00 /opt/lofar/bin/RSPDriver
# Starting TBBDriver
# root     21492     1  0 10:41 pts/2    00:00:00 /opt/lofar/bin/TBBDriver
# 
# Status of all software level:
# 1 : PVSS00pmon                16177
# 1 : SoftwareMonitor           16227
# 1 : LogProcessor              16248
# 1 : ServiceBroker             16278
# 1 : SASGateway                16299
# ---
# 2 : RSPDriver                 21470
# 2 : TBBDriver                 21492
# ---
# 3 : CalServer                 DOWN
# 3 : BeamServer                DOWN
# ---
# 4 : HardwareMonitor           DOWN
# ---
# 5 : SHMInfoServer             DOWN
# ---
# 6 : CTStartDaemon             DOWN
# 6 : StationControl            DOWN
# 6 : ClockControl              DOWN
# 6 : CalibrationControl        DOWN
# 6 : BeamControl               DOWN
# 6 : TBBControl                DOWN
# ---

def swlevel(level=None):
    global logger
    _level = level
    board_errors = list()
    if (level != None):
        if _level < 0:
            _level *= -1
        answer = sendCmd('swlevel', str(_level))
    else:
        answer = sendCmd('swlevel')
        
    current_level = 0
    for line in answer.splitlines():
        if line.find("Going to level") > -1:
            current_level = int(line.split()[-1])
        
        elif line.find("Currently set level") > -1:
            current_level = int(line.split()[-1])
            if current_level < 0:
                logger.warn("Current swlevel is %d" %(current_level))
        if line.find("Error requesting active firmware version") > -1:
            endpos = line.find(":")
            board_errors.append(int(line[:endpos].split()[1]))
            logger.warn(line)
    return (current_level, board_errors)

def reset48V():
    global logger
    logger.info("Try to reset 48V power")
    ec_name = socket.gethostname()[:-1]+"ec"
    ec_ip = socket.gethostbyname(ec_name)
    logger.debug("EC to connect = %s" %(ec_ip))
    
    connected = False

    try:
        sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error:
        sck.close()
        return

    try:
        sck.settimeout(4.0)
        sck.connect((ec_ip, 10000))
        connected = True
        time.sleep(0.5)
        cmd = struct.pack('hhh', 22, 0, 0)
        logger.debug("send cmd")
        sck.send(cmd)
        sck.settimeout(4.0)
        logger.debug("recv cmd")
        data = sck.recv(6)
        sck.close()
        logger.debug("reset done")
    except socket.error:
        print "ec socket connect error"
        sck.close()


# Run rspctl command with given args and return response
def rspctl(args='', wait=0.0):
    global logger
    if args != '':
        logger.debug("rspctl %s" %(args))
        response = sendCmd('rspctl', args)
        if wait > 0.0:
            time.sleep(wait)
        return (response)
    return ('No args given')


# Run tbbctl command with given args and return response
def tbbctl(args=''):
    global logger
    if args != '':
        logger.debug("tbbctl %s" %(args))
        return (sendCmd('tbbctl', args))
    return ('No args given')

def checkActiveTBBDriver():
    answer = sendCmd('swlevel').strip().splitlines()
    for line in answer:
        if line.find('TBBDriver') > -1:
            if line.find('DOWN') != -1:
                return (False)
    return (True)

# wait until all boards have a working image loaded
# returns 1 if ready or 0 if timed_out
def waitTBBready(n_boards=6):
    global logger
    timeout = 90
    logger.info("wait for working TBB boards ")
    sys.stdout.flush()
    while timeout > 0:
        answer = tbbctl('--version')
        #print answer
        if answer.find('TBBDriver is NOT responding') > 0:
            if timeout < 10:
                logger.info("TBBDriver is NOT responding, try again in every 5 seconds")
            time.sleep(5.0)
            timeout -= 5
            if timeout < 60:
                return (0)
            continue
        # check if image_nr > 0 for all boards
        if answer.count('V') == (n_boards * 4):
            logger.info("All boards in working image")
            return (1)
        time.sleep(1.0)
        timeout -= 1
    logger.warn("Not all TB boards in working image")
    return (0)

def checkActiveRSPDriver():
    answer = sendCmd('swlevel').strip().splitlines()
    for line in answer:
        if line.find('RSPDriver') > -1:
            if line.find('DOWN') != -1:
                return (False)
    return (True)

# wait until all boards have a working image loaded
# returns 1 if ready or 0 if timed_out
#
# [lofarsys@RS306C ~]$ rspctl --version
# RSP[ 0] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 1] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 2] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 3] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 4] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 5] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 6] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 7] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 8] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[ 9] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[10] RSP version = 0, BP version = 0.0, AP version = 0.0
# RSP[11] RSP version = 0, BP version = 0.0, AP version = 0.0

def waitRSPready():
    global logger
    timeout = 60
    logger.info("wait for working RSP boards ")
    sys.stdout.flush()
    while timeout > 0:
        answer = rspctl('--version')
        #print answer
        if answer.count('No Response') > 0:
            time.sleep(5.0)
            timeout -= 5
            if timeout < 60:
                return (0)
            continue
        # check if image_nr > 0 for all boards
        if answer.count('0.0') == 0:
            logger.info("All boards in working image")
            return (1)
        else:
            logger.warn("Not all RSP boards in working image")
            logger.debug(answer)
        time.sleep(5.0)
        timeout -= 1
    return (0)

# convert select-list to select-string 
def selectStr(sel_list):
    last_sel = -2
    set = False
    select = ""
    for sel in sorted(sel_list):
        if sel == last_sel+1:
            set = True
        else:
            if set:
                set = False
                select += ':%d' %(last_sel)
            select += ",%d" %(sel)
        last_sel = sel
    if set:
        select += ':%d' %(last_sel)
    return (select[1:])

    # convert select-string to sel_list
def extractSelectStr(selectStr):
    selectStr += '.'
    sel_list = list()
    num_str = ''
    num = 0
    set_num = -1
    for ch in selectStr:
        if ch.isalnum():
            num_str += ch
            continue
        
        num = int(num_str)
        num_str = ''
            
        if set_num > -1:
            while (set_num < num):
                sel_list.append(set_num)
                set_num += 1
            set_num = -1
        
        if ch == ',':
            sel_list.append(num)
            
        if ch == ':':
            set_num = num
             
    sel_list.append(num)        
    return (sorted(sel_list))

def getClock():
    answer = rspctl("--clock")
    #print answer[-6:-3]
    clock = float(answer[-7:-4])
    return (clock)
    
# function used for antenna testing        
def swapXY(state):
    global logger
    if state in (0,1):
        if state == 1:
            logger.info("XY-output swapped")
        else:
            logger.info("XY-output normal")
        rspctl('--swapxy=%d' %(state))

def resetRSPsettings():
    global logger
    if rspctl       ('--clock').find('200MHz') < 0:
        rspctl      ('--clock=200')
        logger.info ("Changed Clock to 200MHz")
        time.sleep  (2.0)
    rspctl('--wg=0', wait=0.0)
    rspctl('--rcuprsg=0', wait=0.0)
    rspctl('--datastream=0', wait=0.0)
    rspctl('--splitter=0', wait=0.0)
    rspctl('--specinv=0', wait=0.0)
    rspctl('--bitmode=16', wait=0.0)
    rspctl('--rcumode=0', wait=0.0)
    rspctl('--rcuenable=0', wait=0.0)
    #rspctl         ('--hbadelays=%s' %(('128,'*16)[:-1]), wait=8.0)
    
def turnonRCUs(mode, rcus):
    global logger
    global rcumode
    start_mode = rcumode
    select = selectStr(rcus)
    logger.info("turn RCU's on, mode %d" %(mode))
    logger.info("enable rcus")
    rspctl('--rcuenable=1 --select=%s' %(select), wait=0.0)
    logger.info("setweights")
    rspctl('--aweights=8000,0', wait=0.0)
    if mode == 5:
        rspctl('--specinv=1', wait=0.0)
    else:
        rspctl('--specinv=0', wait=0.0)
    logger.info("set rcu mode")
    rsp_rcu_mode(mode, rcus)
    rcumode = mode
    
def turnoffRCUs():
    global logger
    global rcumode
    logger.info("RCU's off, mode 0")
    rspctl('--rcumode=0', wait=0.0)
    rspctl('--rcuenable=0', wait=0.0)
    rspctl('--aweights=0,0', wait=1.0)
    rcumode = 0

# set rcu mode, if mode > 4(hba) turn on hba's in steps to avoid power dips
def rsp_rcu_mode(mode, rcus):
    global rcumode
    rcumode = mode
    if mode > 0 and mode < 5: # lba modes
        select = selectStr(rcus)
        rspctl('--rcumode=%d --select=%s' %(mode, select), wait=6.0)
        return (0)
    elif mode < 8: # hba modes
        # maximum 12 power RCUs each step
        steps = int(round(len(rcus) / 24.))
        for step in range(0,(steps*2),2):
            rculist = sorted(rcus[step::(steps*2)]+rcus[step+1::(steps*2)])
            select = string.join(list([str(rcu) for rcu in rculist]),',')
            rspctl('--rcumode=%d --select=%s' %(mode, select), wait=2.0)
        time.sleep(6.0)
        return (0) 
    else:
        return (-1)
        
# set hba_delays in steps to avoid power dips, and discharge if needed
def rsp_hba_delay(delay, rcus, discharge=True):
    global logger
    global active_delay_str
    
    if delay == active_delay_str:
        logger.debug("requested delay already active, skip hbadelay command")
        return (0)
    
    if discharge == True:
        # count number of elements off in last command
        n_hba_off = 0
        for i in active_delay_str.split(','):
            if int(i,10) & 0x02:
                n_hba_off += 1
        
        # count number of elements on in new command, and make discharge string
        n_hba_on = 0
        if n_hba_off > 0:
            discharge_str = ''
            for i in delay.split(','):
                if int(i,10) & 0x02:
                    discharge_str += "2,"
                else:
                    discharge_str += "0,"
                    n_hba_on += 1
        
        # discharge if needed
        if n_hba_off > 0 and n_hba_on > 0:
            logger.info("set hbadelays to 0 for 1 second")
            if n_hba_on > 2:
                steps = int(round(len(rcus) / 24.))
                logger.debug("send hbadelay command in %d steps" %(steps))
                for step in range(0,(steps*2),2):
                    rculist = sorted(rcus[step::(steps*2)]+rcus[step+1::(steps*2)])
                    select = string.join(list([str(rcu) for rcu in rculist]),',')
                    rspctl('--hbadelay=%s --select=%s' %(discharge_str[:-1], select), wait=1.0)
                    rspctl('--hbadelay=%s --select=%s' %(discharge_str[:-1], select), wait=1.0)
                time.sleep(3.0)
            else:    
                rspctl('--hbadelay=%s' %(discharge_str[:-1]), wait=1.0)
                rspctl('--hbadelay=%s' %(discharge_str[:-1]), wait=4.0)
    
    logger.debug("send hbadelay command")
    rspctl('--hbadelay=%s' %(delay), wait=1.0)
    rspctl('--hbadelay=%s' %(delay), wait=4.0)

    active_delay_str = delay
    return (0) 

    
