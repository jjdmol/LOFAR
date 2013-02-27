# lofar_lib

import os
import time
from general_lib import sendCmd

CoreStations          = ('CS001C','CS002C','CS003C','CS004C','CS005C','CS006C','CS007C','CS011C',\
                         'CS013C','CS017C','CS021C','CS024C','CS026C','CS028C','CS030C','CS031',\
                         'CS032C','CS101C','CS103C','CS201C','CS301C','CS302C','CS401C','CS501C')

RemoteStations        = ('RS106C','RS205C','RS208C','RS210C','RS305C','RS306C','RS307C','RS310C',\
                         'RS406C','RS407C','RS409C','RS503C','RS508C','RS509C')

InternationalStations =	('DE601C','DE602C','DE603C','DE604C','DE605C','FR606C','SE607C','UK608C')


StationType = dict( CS=1, RS=2, IS=3 )


def dataDir():
    return (r'/tmp/data')

# remove all *.dat 
def removeAllDataFiles():
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
def readStationConfig():
    f = open('/opt/lofar/etc/RemoteStation.conf', 'r')
    lines = f.readlines()
    f.close()
    
    ID = nRSP = nTBB = nLBL = nLBH = nHBA = 0
    
    for line in lines:
        if line[0] == '#':
            continue
        ptr = line.find('STATION_ID')
        if ptr > 0:
            ptr = line.find('=', ptr) + 1
            ID = int(line[ptr:].strip())
            continue
        ptr = line.find('N_RSPBOARDS')
        if ptr > 0:
            ptr = line.find('=', ptr) + 1
            nRSP = int(line[ptr:].strip())
            continue
        ptr = line.find('N_TBBOARDS')
        if ptr > 0:
            ptr = line.find('=', ptr) + 1
            nTBB = int(line[ptr:].strip())
            continue
        ptr = line.find('N_LBAS')
        if ptr > 0:
            ptr = line.find('=', ptr) + 1
            nLBA = int(line[ptr:].strip())
    
            if nLBA == nRSP * 8:
                nLBL = nLBA / 2
                nLBH = nLBA / 2
            else:
                nLBL = 0
                nLBH = nLBA       
            continue
        ptr = line.find('N_HBAS')
        if ptr > 0:
            ptr = line.find('=', ptr) + 1
            nHBA = int(line[ptr:].strip())
            continue
    return(ID, nRSP, nTBB, nLBL, nLBH, nHBA)
   

def swlevel(level=None):
    answer = sendCmd('swlevel')
    current_level = int(answer.splitlines()[0][-1])
    if (level != None):
        if (level != current_level):
            answer = sendCmd('swlevel', str(level))
            current_level = int(answer.splitlines()[0][-1])
    return (current_level)


# Run rspctl command with given args and return response
def rspctl(args='', wait=0.0):
    if args != '':
        response = sendCmd('rspctl', args)
        #print "rspctl %s" %(args)
        if wait > 0.0:
            time.sleep(wait)
        return (response)
    return ('No args given')


# Run tbbctl command with given args and return response
def tbbctl(args=''):
    if args != '':
        return (sendCmd('tbbctl', args))
    return ('No args given')


# set rcu mode, if mode > 4(hba) turn on hba's in steps to avoid power dips
def rsp_rcumode(mode, n_rcus=96):
    if mode > 0 and mode < 4: # lba modes
        rspctl('--rcumode=%d' %(mode))
        return (0)
    elif mode < 8: # hba modes
        #n_rcus = n_boards * 8
        n_pwr_rcus = n_rcus / 2
        # maximum 12 power RCUs each step
        steps = n_pwr_rcus / 12 # 4 steps for NL stations, 8 steps for IS stations
        jump = n_rcus / 12      # jump = 8 for NL stations and 16 for IS stations
        
        if steps == 0: steps = 1
        if jump < 2: jump = 2
        
        for step in range(steps):
            selection = '--select='
            for rcu in range(step*2, n_rcus, jump):
                selection += '%d,%d,' %(rcu,rcu+1)
            rspctl('--rcumode=%d %s' %(mode, selection[:-1]))
            time.sleep(1.0)
        time.sleep(1.0)
        return (0) 
    else:
        return (-1)
        

    
