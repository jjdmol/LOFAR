#!/usr/bin/python
# format converter
# from csv file to log file
#
# P.Donker

import sys
import os

libPath = os.path.join(os.getcwd(),'lib')
sys.path.insert(0, libPath)

from general_lib import *

f = open("checkHardware.conf", 'r')
data = f.readlines()
f.close()
for line in data:
    if line.find('log-dir') != -1:
        key, logdir = line.strip().split('=')

log = cStationLogger(logdir)

testfilename = '%s_StationTest.csv' %(getHostName())
fullFilename = os.path.join(logdir, testfilename)
f = open(fullFilename, 'r')
testdata = f.readlines()
f.close()

for line in testdata:
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
    
    if part == 'TBB':
        if msgType == 'VERSION':
            if partNr == '---':
                log.addLine('TBBver>: Sv=normal  Pr=normal , Version Error TBBdriver=%s tbbctl=%s' %\
                           (keyinfo['TBBDRIVER'], keyinfo['TBBCTL']))
            else:
                log.addLine('TBBver>: Sv=normal  Pr=normal , Board=%d Version Error TP=%s MP=%s' %\
                           (partNr, keyinfo['TP'], keyinfo['MP']))
        elif msgType == 'MEMORY':
            log.addLine('TBBmem>: Sv=normal  Pr=normal , Board=%d Memory Error' %\
                           (partNr))
    
    if part == 'RSP':
        if msgType == 'VERSION':
            log.addLine('TBBver>: Sv=normal  Pr=normal , Board=%d, Version Error BP=%s AP=%s' %\
                       (partNr, keyinfo['BP'], keyinfo['AP']))
    
    if part == 'LBL':
        rcu = partNr * 2 
        if msgType == 'FAIL':
            if keyinfo.has_key('X'):
                log.addLine('LBAmd1>: Sv=normal  Pr=normal , LBA Outer (LBL) defect: RCU: %d factor: %3.1f' %\
                           (partNr*2, float(keyinfo['X'])))
            if keyinfo.has_key('Y'):
                log.addLine('LBAmd1>: Sv=normal  Pr=normal , LBA Outer (LBL) defect: RCU: %d factor: %3.1f' %\
                           (partNr*2+1, float(keyinfo['Y'])))               
        
        elif msgType == 'DOWN':
            log.addLine('LBAmd1>: Sv=normal  Pr=normal , LBA Outer (LBL) down: RCU: %d factor: %3.1f offset: %d' %\
                       (partNr*2, float(keyinfo['X']), int(keyinfo['Xoff'])))
            log.addLine('LBAmd1>: Sv=normal  Pr=normal , LBA Outer (LBL) down: RCU: %d factor: %3.1f offset: %d' %\
                       (partNr*2+1, float(keyinfo['Y']), int(keyinfo['Yoff'])))
        
        elif msgType == 'TOOLOW':
                log.addLine('LBAmd1>: Sv=normal  Pr=normal , LBA	levels to low!!!' )               
            
    if part == 'LBH':
        if msgType == 'FAIL':
            if keyinfo.has_key('X'):
                log.addLine('LBAmd3>: Sv=normal  Pr=normal , LBA Inner (LBH) defect: RCU: %s factor: %3.1f' %\
                           (partNr*2, float(keyinfo['X'])))
            if keyinfo.has_key('Y'):
                log.addLine('LBAmd3>: Sv=normal  Pr=normal , LBA Inner (LBH) defect: RCU: %s factor: %3.1f' %\
                           (partNr*2, float(keyinfo['Y'])))           
        
        elif msgType == 'DOWN':
            log.addLine('LBAmd3>: Sv=normal  Pr=normal , LBA Inner (LBH) down: RCU: %d factor: %3.1f offset: %d' %\
                       (partNr*2, float(keyinfo['X']), int(keyinfo['Xoff'])))
            log.addLine('LBAmd3>: Sv=normal  Pr=normal , LBA Inner (LBH) down: RCU: %d factor: %3.1f offset: %d' %\
                       (partNr*2+1, float(keyinfo['Y']), int(keyinfo['Yoff'])))
        
        elif msgType == 'TOOLOW':
                log.addLine('LBAmd3>: Sv=normal  Pr=normal , LBA	levels to low!!!' )               

    if part == 'HBA':
        if msgType == 'FAIL':
            for elem_nr in range(1,17,1):

                if keyinfo.has_key('M%d' %(elem_nr)):
                    val1 = keyinfo.get('M%d' %(elem_nr))
                    log.addLine('HBAmdt>: Sv=normal  Pr=normal , Tile %d - RCU %d; Element %s Suspicious. : (%s))' %\
                               (partNr, partNr*2+1, elem_nr, val1))

                if keyinfo.has_key('X%d' %(elem_nr)):
                    val = keyinfo.get('X%d' %(elem_nr))

                    log.addLine('HBAmd5>: Sv=normal  Pr=normal , Tile %d - RCU %d; Element %d Broken. RF-signal to low : (Factor = %s, CtrlWord = 128)' %\
                               (partNr, partNr*2, elem_nr, val[0]))
                    log.addLine('HBAmd5>: Sv=normal  Pr=normal , Tile %d - RCU %d; Element %d Broken. RF-signal to low : (Factor = %s, CtrlWord = 253)' %\
                               (partNr, partNr*2, elem_nr, val[3]))
                               
                if keyinfo.has_key('Y%d' %(elem_nr)):
                    val = keyinfo.get('Y%d' %(elem_nr))
                    log.addLine('HBAmd5>: Sv=normal  Pr=normal , Tile %d - RCU %d; Element %d Broken. RF-signal to low : (Factor = %s, CtrlWord = 128))' %\
                               (partNr, partNr*2+1, elem_nr, val[0]))
                    log.addLine('HBAmd5>: Sv=normal  Pr=normal , Tile %d - RCU %d; Element %d Broken. RF-signal to low : (Factor = %s, CtrlWord = 253))' %\
                               (partNr, partNr*2+1, elem_nr, val[3]))
