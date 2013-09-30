#!/usr/bin/python
#
# read last test log file (.csv)
# and send test result to PVSS,
# and write to PVSS log file
#
# P.Donker

import sys
import os
from time import sleep

libPath = '/opt/stationtest/lib'
sys.path.insert(0, libPath)

from general_lib import *
from lofar_lib import *

args   = dict()
logdir = ""
logger = 0
nLBL   = 0
nLBH   = 0
nHBA   = 0
nRSP   = 0

# PVSS states
State = dict({'OFF':0, 'OPERATIONAL':1, 'MAINTENANCE':2, 'TEST':3, 'SUSPICIOUS':4, 'BROKEN':5})


def main():
    global args, logdir, logger, nRSP, nLBL, nLBH, nHBA
    getArguments()
    logdir = getLogDir()
    ID, nRSP, nTBB, nLBL, nLBH, nHBA = readStationConfig()
    logger = cPVSSLogger(logdir)
    
    if args.has_key('RESET'):
        resetPVSS(state=0)
        addManualDataToPVSS()
    
    if args.has_key('NO_UPDATE'):
        print "PVSS Update skipped"
        sys.exit()
        
    if args.has_key('MANUAL'):
        addManualDataToPVSS()
    
    # read last log file from checkhardware
    testfilename = '%s_StationTest.csv' %(getHostName())
    fullFilename = os.path.join(logdir, testfilename)
    if args.has_key('FILE'):
        fullFilename = args.get('FILE')
        
    try:
        f = open(fullFilename, 'r')
    except IOError:
        print "file not found %s" %(fullFilename)
        return
        
    testdata = f.readlines()
    f.close()
    bad_lba, bad_hba = addDataToPVSS(testdata)
    addDataToBadRcuFile(bad_lba, bad_hba)

# print help screen
def printHelp():
    print "----------------------------------------------------------------------------"
    print "Usage of arguments"
    print "Output of last stationcheck is always send to pvss also the bad_rcu file is made"
    print "-h                   : this help screen"
    
    print "-manual              : send manual list"     
    print "-reset[=type]        : set all state fields to ok for type if given"
    print "                       type = all | lba | lbl | lbh | hba (all=default)"
    print "-no_update           : skip pvss update"
    print "-test                : do not send to PVSS"
    print "-file=[full filename]: filename to use"
    print ""
    #print "-L=x                : x = flag level"
    print " NEXT KEYS ARE ONLY USED FOR HBA ERRORS"
    print "-S=x                 : rf, flag only if deviation greater than x dB"
    print "-N=x,y,z             : noise, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    print "-J=x,y,z             : jitter, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    print "-SN                  : do not flag summator noise"
    print "-SP                  : do not flag spurious signals"
    print "-O                   : do not flag oscillating signals"
    print "-M=x                 : modem, flag only if error in x elements (x=0..16)"
    print "-E                   : do not flag results of element test"
    print " NEXT KEYS ARE ONLY USED FOR LBA ERRORS"
    print "-LBLS=x              : lbl rf, flag only if deviation greater than x dB"
    print "-LBLN=x,y,z          : noise, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    print "-LBLJ=x,y,z          : jitter, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    print "-LBHS=x              : lbh rf, flag only if deviation greater than x dB"
    print "-LBHN=x,y,z          : noise, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    print "-LBHJ=x,y,z          : jitter, flag only if available more than x% of time (x=0..100)"
    print "                       or available more than y% of time and fluctuation > z dB"
    
# get command line arguments
def getArguments():
    global args
    for i in range(len(sys.argv)):
        if sys.argv[i][0] == '-':
            if sys.argv[i].find('=') != -1:
                valpos = sys.argv[i].find('=')
                key = sys.argv[i][1:valpos].upper() 
                val = sys.argv[i][valpos+1:].split(',')
                if len(val) > 1:
                    args[key] = val
                else:
                    args[key] = val[0]
            else:
                args[sys.argv[i][1:].upper()]='-'

        if args.has_key('H') or args.has_key('HELP'):
            printHelp()
            sys.exit()
    return

# get logdir from configuration file
def getLogDir():
    logdir = ""
    # look for log directory
    f = open("/opt/stationtest/checkHardware.conf", 'r')
    data = f.readlines()
    f.close()
    for line in data:
        if line.find('log-dir-local') != -1:             
            key, logdir = line.strip().split('=')    
    return(logdir)

# send comment, key and value to PVSS and write to file
def sendToPVSS(comment, pvss_key, value):
    global logger, args
    
    if len(comment) > 0:
        comment = 'stationtest::'+comment
    else:
        comment = 'stationtest'
    arguments = '%s %s %d' %(comment, pvss_key, value)
    logger.addLine(arguments[11:])
    if args.has_key('TEST'):
        print arguments
    else:
        response = sendCmd('setObjectState', arguments)
        sleep(0.05)
        return(response)
    return("")

# set all antenna info to ok
def resetPVSS(state=0):
    global args, libPath, State, nRSP, nLBL, nLBH, nHBA
    
    reset_type = args.get('RESET','ALL').upper()
    filename = "reset_pvss.log"
    full_filename = os.path.join(libPath, filename)
    f = open(full_filename, 'w')
    
    if reset_type == 'ALL':    
        for rcu in range(nRSP*8):
            board = int(rcu / 8)
            rack  = int(board / 4)
            cabinet = int(rack / 2)    
            f.write("LOFAR_PIC_Cabinet%d_Subrack%d_RSPBoard%d_RCU%d %d\n" %(cabinet, rack, board, rcu, state))
    
    if reset_type in ('ALL','LBA','LBH'):
        for ant in range(nLBH):
            f.write("LOFAR_PIC_LBA%03d %d\n" %(ant, state))

    if reset_type in ('ALL','LBA','LBL'):
        for ant in range(nLBL):
            f.write("LOFAR_PIC_LBA%03d %d\n" %(ant+48, state))

    if reset_type in ('ALL','HBA'):        
        for tile in range(nHBA):
            f.write("LOFAR_PIC_HBA%02d %d\n" %(tile, state))
            
            for elem in range(16):
                f.write("LOFAR_PIC_HBA%02d.element%02d %d\n" %(tile, elem, state))
                f.write("LOFAR_PIC_HBA%02d.element%02d.comm %d\n" %(tile, elem, state))
                f.write("LOFAR_PIC_HBA%02d.element%02d.X %d\n" %(tile, elem, state))
                f.write("LOFAR_PIC_HBA%02d.element%02d.Y %d\n" %(tile, elem, state))
    f.close()
    if not args.has_key('TEST'):
        sendCmd("setObjectState", "stationtes:reset %s" %(full_filename))
        sleep(2.0)
    


# add manual filled list with bad antennas to pvss
def addManualDataToPVSS():
    global State, logdir
    filename = "bad_antenna_list.txt"
    full_filename = os.path.join(logdir, filename)
    try:
        f = open(full_filename, 'r')
    except IOError:
        print "%s not found" %(filename)
        return
    data = f.read()
    f.close()
    for line in data:
        if line[0] == '#':
            continue
        if line.upper().find(getHostName()) > -1:
            bad_antenna_list = line.strip().split(' ')[1:]
            for ant in bad_antenna_list:
                part    = ant[:3].upper()
                part_nr = int(ant[3:])
                if part == 'LBA':
                    sendToPVSS("manualy-marked", "LOFAR_PIC_LBA%03d" %(part_nr), State['BROKEN'])
                if part == 'HBA':
                    sendToPVSS("manualy-marked", "LOFAR_PIC_HBA%02d" %(part_nr), State['BROKEN'])
            return
    
# add result data from checkhardware to PVSS
def addDataToPVSS(data):
    global args
    global State
    bad_lba = dict()
    bad_hba = dict() 
    
    RFrefX = 0.0
    RFrefY = 0.0
    
         
    for line in data:
        if line[0] == '#':
            continue
        keyinfo = dict()
        info = line.split(',')
        #print info
        date    = info[0]
        part    = info[1]
        partNr = '---'
        if info[2] != '---':
            partNr  = int(info[2])
        msgType = info[3].strip()
        for i in range(4,len(info)):
                if info[i].find('=') != -1:
                    key, valstr = info[i].split('=')
                    vallist = valstr.split()
                    if len(vallist) == 1:
                        keyinfo[key] = vallist[0]
                    elif len(vallist) > 1:
                        keyinfo[key] = vallist
                else:
                    keyinfo[info[i]] = '-'
        
        
        if part == 'LBL':
            lban_limits = args.get('LBLN','0.0')
            lbaj_limits = args.get('LBLJ','0.0')
            lbas_limit  = args.get('LBLS','0.0')
        elif part == 'LBH':
            lban_limits = args.get('LBHN','0.0')
            lbaj_limits = args.get('LBHJ','0.0')
            lbas_limit  = args.get('LBHS','0.0')
        
        if part in ('LBL', 'LBH'):
            if msgType == 'TESTSIGNAL':
                RFrefX = float(keyinfo.get('SIGNALX','0.0'))
                RFrefY = float(keyinfo.get('SIGNALY','0.0'))
                
            if msgType == 'LOW_NOISE':
                if float(keyinfo.get('Xproc','0.0')) >= 100.0 or float(keyinfo.get('Yproc','0.0')) >= 100.0:
                    sendToPVSS("low-noise", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                    bad_lba[partNr] = 1

            elif msgType == 'HIGH_NOISE':
                proc_limit_2 = 0.0
                diff_limit = 0.0
                if len(lban_limits) > 1:
                    proc_limit_1 = float(lban_limits[0])
                    proc_limit_2 = float(lban_limits[1])
                    diff_limit = float(lban_limits[2])
                else:
                    proc_limit_1 = float(lban_limits)
                    
                if float(keyinfo.get('Xproc','0.0')) >= proc_limit_1 or float(keyinfo.get('Yproc','0.0')) >= proc_limit_1:
                    if (float(keyinfo.get('Xproc','0.0')) < proc_limit_2 and float(keyinfo.get('Xdiff','0.0')) < diff_limit) and\
                       (float(keyinfo.get('Yproc','0.0')) < proc_limit_2 and float(keyinfo.get('Ydiff','0.0')) < diff_limit):
                        pass
                    else:
                        sendToPVSS("noise", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lba[partNr] = 1

            elif msgType == 'JITTER':
                proc_limit_2 = 0.0
                diff_limit = 0.0
                if len(lbaj_limits) > 1:
                    proc_limit_1 = float(lbaj_limits[0])
                    proc_limit_2 = float(lbaj_limits[1])
                    diff_limit = float(lbla_limits[2])
                else:
                    proc_limit_1 = float(lbla_limits)
                
                if float(keyinfo.get('Xproc','0.0')) >= proc_limit_1 or float(keyinfo.get('Yproc','0.0')) >= proc_limit_1:
                    if (float(keyinfo.get('Xproc','0.0')) < proc_limit_2 and float(keyinfo.get('Xdiff','0.0')) < diff_limit) and\
                       (float(keyinfo.get('Yproc','0.0')) < proc_limit_2 and float(keyinfo.get('Ydiff','0.0')) < diff_limit):
                        pass
                    else:
                        sendToPVSS("jitter", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lba[partNr] = 1

            elif msgType == 'JITTER':
                limits = args.get('LBHJ','0.0')
                proc_limit_2 = 0.0
                diff_limit = 0.0
                if len(limits) > 1:
                    proc_limit_1 = float(limits[0])
                    proc_limit_2 = float(limits[1])
                    diff_limit = float(limits[2])
                else:
                    proc_limit_1 = float(limits)
                
                if float(keyinfo.get('Xproc','0.0')) >= proc_limit_1 or float(keyinfo.get('Yproc','0.0')) >= proc_limit_1:
                    if (float(keyinfo.get('Xproc','0.0')) < proc_limit_2 and float(keyinfo.get('Xdiff','0.0')) < diff_limit) and\
                       (float(keyinfo.get('Yproc','0.0')) < proc_limit_2 and float(keyinfo.get('Ydiff','0.0')) < diff_limit):
                        pass
                    else:
                        sendToPVSS("jitter", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lbh[partNr] = 1
            
            elif msgType == 'OSCILLATION':
                sendToPVSS("oscillating", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lba[partNr] = 1

            elif msgType == 'RF_FAIL':
                comment = "rf-fail-"
                flag = False
                X = float(keyinfo.get('X','0.0'))
                Y = float(keyinfo.get('Y','0.0'))
                if X > 0.0:
                    if abs(X - RFrefX) > float(lbas_limit):
                        comment += "X"
                        flag = True
                if Y > 0.0:
                    if abs(Y - RFrefY) > float(lbas_limit):
                        comment += "Y"
                        flag = True        
                if flag:
                    p#rint 'LBL %3.1f (%3.1f) %3.1f (%3.1f)' %(X, RFrefX, Y, RFrefY)
                    sendToPVSS(comment, "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lba[partNr] = 1
            
            elif msgType == 'DOWN':
                sendToPVSS("down", "LOFAR_PIC_LBA%03d" %(partNr), State['BROKEN'])
                bad_lba[partNr] = 1
                    
        if part == 'HBA':
            if msgType == 'LOW_NOISE':
                if float(keyinfo.get('Xproc','0.0')) >= 100.0 or float(keyinfo.get('Yproc','0.0')) >= 100.0:
                    sendToPVSS("low-noise", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                    bad_hba[partNr] = 1
                    
            elif msgType == 'HIGH_NOISE':
                limits = args.get('N','0.0')
                proc_limit_2 = 0.0
                diff_limit = 0.0
                if len(limits) > 1:
                    proc_limit_1 = float(limits[0])
                    proc_limit_2 = float(limits[1])
                    diff_limit = float(limits[2])
                else:
                    proc_limit_1 = float(limits)
                    
                if float(keyinfo.get('Xproc','0.0')) >= proc_limit_1 or float(keyinfo.get('Yproc','0.0')) >= proc_limit_1:
                    if (float(keyinfo.get('Xproc','0.0')) < proc_limit_2 and float(keyinfo.get('Xdiff','0.0')) < diff_limit) and\
                       (float(keyinfo.get('Yproc','0.0')) < proc_limit_2 and float(keyinfo.get('Ydiff','0.0')) < diff_limit):
                        pass
                    else:
                        sendToPVSS("noise", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1

            elif msgType == 'JITTER':
                limits = args.get('J','0.0')
                proc_limit_2 = 0.0
                diff_limit = 0.0
                if len(limits) > 1:
                    proc_limit_1 = float(limits[0])
                    proc_limit_2 = float(limits[1])
                    diff_limit = float(limits[2])
                else:
                    proc_limit_1 = float(limits)
                
                if float(keyinfo.get('Xproc','0.0')) >= proc_limit_1 or float(keyinfo.get('Yproc','0.0')) >= proc_limit_1:
                    if (float(keyinfo.get('Xproc','0.0')) < proc_limit_2 and float(keyinfo.get('Xdiff','0.0')) < diff_limit) and\
                       (float(keyinfo.get('Yproc','0.0')) < proc_limit_2 and float(keyinfo.get('Ydiff','0.0')) < diff_limit):
                        pass
                    else:
                        sendToPVSS("jitter", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
                    
            elif msgType == 'OSCILLATION':
                if not args.has_key('O'):
                    sendToPVSS("oscillating", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
                    
            elif msgType == 'C_SUMMATOR':
                sendToPVSS("moden-fail", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
            
            elif msgType == 'SUMMATOR_NOISE':
                if not args.has_key('SN'):
                    sendToPVSS("summator-noise", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
                                
            elif msgType == 'SPURIOUS':
                if not args.has_key('SP'):
                    sendToPVSS("spurious-signals", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
                    
            elif msgType == 'RF_FAIL':
                flag = False
                limit = float(args.get('S','0'))
                X = keyinfo.get('X',[])
                Y = keyinfo.get('Y',[])
                if len(X):
                    if abs(float(X[0]) - float(X[2])) > limit:
                        flag = True
                if len(Y):
                    if abs(float(Y[0]) - float(Y[2])) > limit:
                        flag = True        
                if flag:    
                    sendToPVSS("rf-tile-fail", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                bad_hba[partNr] = 1
                    
            elif msgType == 'E_FAIL':
                if args.has_key('E') == False:
                    max_errors   = 2
                    modem_errors = 0
                    LNX_errors   = 0
                    LNY_errors   = 0
                    RFX_errors   = 0
                    RFY_errors   = 0
                    
                    # check first total number of errors in tile
                    for elem_nr in range(1,17,1):
                        if keyinfo.has_key('M%d' %(elem_nr)):
                            modem_errors += 1
                        if keyinfo.has_key('LNX%d' %(elem_nr)):
                            LNX_errors += 1
                        if keyinfo.has_key('LNY%d' %(elem_nr)):
                            LNY_errors += 1
                        if keyinfo.has_key('X%d' %(elem_nr)):
                            RFX_errors += 1
                        if keyinfo.has_key('Y%d' %(elem_nr)):
                            RFY_errors += 1    
                            

                    send_tile_errors = 0        
                    for elem_nr in range(1,17,1):
                        send_elem_errors = 0
                        
                        if modem_errors > max_errors and keyinfo.has_key('M%d' %(elem_nr)):
                            sendToPVSS("rf-fail", "LOFAR_PIC_HBA%02d.element%02d.comm" %(partNr, elem_nr-1), State['BROKEN'])
                            send_elem_errors += 1
                        
                        comment = ""
                        if (RFX_errors > max_errors) and keyinfo.has_key('X%d' %(elem_nr)):
                            comment += "rf-fail&"
                        
                        if (LNX_errors > max_errors) and keyinfo.has_key('LNX%d' %(elem_nr)):
                            comment += "low-noise&"
                        
                        if keyinfo.has_key('HNX%d' %(elem_nr)) or keyinfo.has_key('JX%d' %(elem_nr)):
                            comment += "noise&"
                        
                        if len(comment) > 0:
                            sendToPVSS(comment[:-1], "LOFAR_PIC_HBA%02d.element%02d.X" %(partNr, elem_nr-1), State['BROKEN'])
                            send_elem_errors += 1
                        
                        
                        comment = ""
                        if (RFY_errors > max_errors) and keyinfo.has_key('Y%d' %(elem_nr)):
                            comment += "rf-fail&"
    
                        if (LNY_errors > max_errors) and keyinfo.has_key('LNY%d' %(elem_nr)):
                            comment += "low-noise&"
                        
                        if keyinfo.has_key('HNY%d' %(elem_nr)) or keyinfo.has_key('JY%d' %(elem_nr)):
                            comment += "noise&"
                        
                        if len(comment) > 0:    
                            sendToPVSS(comment[:-1], "LOFAR_PIC_HBA%02d.element%02d.Y" %(partNr, elem_nr-1), State['BROKEN'])
                            send_elem_errors += 1
                    
                    
                        if send_elem_errors > 0:
                            sendToPVSS("rf-fail", "LOFAR_PIC_HBA%02d.element%02d" %(partNr, elem_nr-1), State['BROKEN'])
                            send_tile_errors += 1
                    
                    if send_tile_errors > 0:
                        sendToPVSS("", "LOFAR_PIC_HBA%02d" %(partNr), State['BROKEN'])
                        bad_hba[partNr] = 1
                    
    return (list(bad_lba), list(bad_hba))                    

# write bad rcu's to file in logdir
def addDataToBadRcuFile(bad_lba, bad_hba):
    global nLBL
    
    # add bad rcus to file                               
    filename = '%s_bad_rcus.txt' %(getHostName())
    full_filename = os.path.join(logdir, filename) 
    f = open(full_filename, 'w')

#    if nLBL:
#        bad = ""
#        for ant in sorted(bad_lbl):
#            bad += "%d," %(ant*2)
#            bad += "%d," %(ant*2+1)
#        if len(bad):
#            bad = bad[:-1]
#        bad = "LBL=[" + bad + "]\n"
#        f.write(bad)
    
    bad = ""
    for ant in sorted(bad_lba):
        bad += "%d," %(ant*2)
        bad += "%d," %(ant*2+1)
    if len(bad):
        bad = bad[:-1]
    bad = "LBA=[" + bad + "]\n"
    f.write(bad)
    
    bad = ""
    for tile in sorted(bad_hba):
        bad += "%d," %(tile*2)
        bad += "%d," %(tile*2+1)
    if len(bad):
        bad = bad[:-1]
    bad = "HBA=[" + bad + "]\n"
    f.write(bad)
    
    f.close()


if __name__ == "__main__":
    main()
