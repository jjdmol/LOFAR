#!/usr/bin/python

# Show logfile

import sys
import os
import string
import datetime
import time

def printHelp():
    print "possible option for this script"
    print "-------------------------------"
    print "-h            print this help screen"
    print "-s=CS002C     station to show 'CS002C'"
    print "-d=2          show last 2 days"
    print "-f=full_path_filename"
    #print "-p=full_path  path too this script"
    print "-------------------------------"
    sys.exit(0)

i = 1
args = dict()
while i < len(sys.argv):
    if sys.argv[i][0] == '-':
        opt = sys.argv[i][1].upper()
        optval = '-'
        valpos = sys.argv[i].find('=')
        if valpos != -1:
            optval = sys.argv[i][valpos+1:]
        args[opt] = optval
        i += 1       

if args.has_key('H'):
    printHelp()
    sys.exit(0)

runPath = r'/opt/stationtest'
if args.has_key('P'):
	runPath = args.get('P')
libPath = runPath+r'/lib'
sys.path.insert(0, libPath)

from general_lib import *
from lofar_lib import *

StID = args.get('S', getHostName()).upper()

def main():
    f = open(runPath+r'/checkHardware.conf', 'r')
    data = f.readlines()
    f.close()
    for line in data:
        if line.find('log-dir-local') != -1:
            key, logdir = line.strip().split('=')    
    
    if args.has_key('F'):
        fullFilename = args.get('F')
    else:
        if args.has_key('D'):
            testfilename = '%s_StationTestHistory.csv' %(StID)
        else:
            testfilename = '%s_StationTest.csv' %(StID)
    
        if os.path.exists(logdir):
            fullFilename = os.path.join(logdir, testfilename)
        else:
            print "not a valid log dir"
            sys.exit(-1)
        
    try:
        f = open(fullFilename, 'r')
        data = f.readlines()
        f.close()
    except:
        print "%s not found in %s" %(testfilename, logdir)
        sys.exit(-1)
        
    RCUx = RCUy = 0
    
    banner = "\n"
    banner += "------------------------------------------------------------------------------------------------------\n"
    banner += " #       #     ###  #####       ###  #  #  ####   ###  #  #       ####   ####   ###   #   #  #   #####\n"
    banner += " #      # #   #       #        #     #  #  #     #     # #        #   #  #     #      #   #  #     #  \n"
    banner += " #     #   #   ###    #        #     ####  ###   #     ##         ####   ###    ###   #   #  #     #  \n"
    banner += " #     #####      #   #        #     #  #  #     #     # #        #  #   #         #  #   #  #     #  \n"
    banner += " ####  #   #   ###    #    o    ###  #  #  ####   ###  #  #   o   #   #  ####   ###    ###   ####  #  \n"
    banner += "------------------------------------------------------------------------------------------------------\n"
    print banner
    
    _part = ''
    _part_nr = -1
    _element_nr = -1
    _c_summator_defect = -1
    
    first_date = 0
    if args.has_key('D'):
        days = int(args.get('D'))
        #linedate = data[len(data)-1].strip().split(',')[0]
        #print linedate
        #dt = datetime.date(int(linedate[:4]), int(linedate[4:6]), int(linedate[6:]))
        #start_date = dt - datetime.timedelta(days-1)
        #first_date = int(getShortDateStr(tm=start_date.timetuple()))
        #print first_date
        
        days_cnt = 1
        
        first_date = data[-1].strip().split(',')[0]
        print first_date
        for i in range(len(data)-1,-1,-1):
            line = data[i]
            if line[0] == '#':
                continue
            line_date = line.strip().split(',')[0]
            if line_date != first_date:
                first_date = line_date
                days_cnt += 1
            if days_cnt == days:
                break
        print first_date

        
    last_date = first_date
    for line in data:
        partnumber = -1
        if line[0] == '#':
            continue
            
        d = line.strip().split(',')
        if len(d) < 4:
            continue
        date = d[0]
        
        if last_date != date:
            print '\n'+'#'*103
        last_date = date
        
        if first_date != 0 and int(date) < int(first_date):
            continue
            
        part = d[1]
        if d[2] != '---':
            partnumber = int(d[2])
            RCUx = partnumber * 2
            RCUy = partnumber * 2 + 1
        msg = d[3].strip()
        kv = dict()
        for i in range(4,len(d)):
            if d[i].find('=') != -1:
                key, valstr = d[i].split('=')
                vallist = valstr.split()
                if len(vallist) == 1:
                    kv[key] = vallist[0]
                elif len(vallist) > 1:
                    kv[key] = vallist
            else:
                kv[d[i]] = '-'
                
        if part == 'NFO':
            #if args.has_key('D'):
            #	print
            #	print '-'*103
            #    print "   NEW TEST  "*8
            #    print '-'*103
            #    
            if msg == 'STATION':
                print ">> Station name : %s" %(kv.get('NAME'))
            
            if msg == 'RUNTIME':
                print ">> Check date   : %s-%s-%s" %(date[6:], date[4:6], date[:4])
                print ">> Check runtime: %s .. %s" %(kv.get('START'), kv.get('STOP'))
            
            if msg == 'CHECKS':
                print ">> Checks done  : %s" %(string.join(d[4:],', ')) 
            
            if msg == 'STATISTICS':
                print ">> Bad antennas : LBL=%s, LBH=%s, HBA=%s" %\
                      (kv.get('BAD_LBL'), kv.get('BAD_LBH'), kv.get('BAD_HBA'))
            
        if part == 'RSP':
            if part != _part:
                _part = part
                hdr = "\n== RSP "
                print hdr + "="*(104-len(hdr))    

            if msg == 'VERSION':
                if kv.has_key('RSPDRIVER'):
                    print "    Wrong RSPDriver version, %s" %(kv.get('RSPDRIVER'))
                if kv.has_key('RSPCTL'):
                    print "    Wrong rspctl version, %s" %(kv.get('RSPCTL'))    
                if kv.has_key('AP') or kv.has_key('BP'):
                    print "    Board %2d wrong firmware version: AP=%s BP=%s" %(partnumber, kv.get('AP'), kv.get('BP'))
        
        if part == 'TBB':
            if part != _part:
                _part = part
                
                hdr = "\n== TBB "
                print hdr + "="*(104-len(hdr))    
            
            if msg == 'VERSION':
                if kv.has_key('TBBDRIVER'):
                    print "    Wrong TBBDriver version, %s" %(kv.get('TBBDRIVER'))
                if kv.has_key('TBBCTL'):
                    print "    Wrong tbbctl version, %s" %(kv.get('TBBCTL'))    
                if kv.has_key('TP') or kv.has_key('MP'):
                    print "    Board %2d wrong firmware version: TP=%s MP=%s" %(partnumber, kv.get('TP'), kv.get('MP'))
            
            if msg == 'MEMORY':
                print "    Board %2d Memory address or dataline error" %(partnumber)
        
        if part == 'RCU':
            if part != _part:
                _part = part
                hdr = "\n== RCU "
                print hdr + "="*(104-len(hdr))    
                
            if msg == 'BROKEN':
                print "    RCU %d Broken" %(partnumber)
        
        
        if part in ('LBL','LBH'):
            if part != _part:
                _part = part
                if part == 'LBL':
                    hdr = "\n== LBA Low "
                else:
                    hdr = "\n== LBA High "
                print hdr + "="*(104-len(hdr))    
            
            lbaNumber = partnumber
            if part == 'LBL':
                lbaNumber += 48
            
            if msg == 'NOSIGNAL':
                print "   NO test signal found"
            
            if msg == 'TESTSIGNAL':
                print
                print " X test done with subband=%s and ref.signal=%sdB" %\
                      (kv.get('SUBBANDX'), kv.get('SIGNALX'))
                print " Y test done with subband=%s and ref.signal=%sdB" %\
                      (kv.get('SUBBANDY'), kv.get('SIGNALY'))       
            
            if msg == 'TOOLOW':
                print "   Average signal strenght Too Low  AVG %sdB" %\
                      (kv.get('AVG'))
            
            if msg == 'DOWN':
                    print "   Antenna %2d, %-11s, has Fallen: X=%sdB Xoffset=%s  Y=%sdB Yoffset=%s" %\
                      (lbaNumber, 'RCU %d/%d' %(RCUx, RCUy), kv.get('X',('?',)), kv.get('Xoff',('?',)), kv.get('Y',('?',)), kv.get('Yoff',('?',)))
            
            if msg == 'OSCILLATION':
                if kv.has_key('X')  or kv.has_key('Xbands'):
                    print "   Antenna %2d, %-7s, X Oscillation" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y')  or kv.has_key('Ybands'):
                    print "   Antenna %2d, %-7s, Y Oscillation" %(lbaNumber, 'RCU %d' %(RCUy))
            
            if msg == 'LOW_NOISE':
                if kv.has_key('Xproc'):
                    print "   Antenna %2d, %-7s, X Low Noise:  %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('Xproc'), kv.get('Xval'), kv.get('Xdiff','-'), kv.get('Xref'))
                if kv.has_key('Yproc'):
                    print "   Antenna %2d, %-7s, Y Low Noise:  %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUy), kv.get('Yproc'), kv.get('Yval'), kv.get('Ydiff','-'), kv.get('Yref'))
                    
            if msg == 'HIGH_NOISE':
                if kv.has_key('Xproc'):
                    print "   Antenna %2d, %-7s, X High Noise: %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('Xproc'), kv.get('Xval'), kv.get('Xdiff','-'), kv.get('Xref'))
                if kv.has_key('Yproc'):
                    print "   Antenna %2d, %-7s, Y High Noise: %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUy), kv.get('Yproc'), kv.get('Yval'), kv.get('Ydiff','-'), kv.get('Yref'))
            
            if msg == 'JITTER':
                if kv.has_key('Xdiff'):
                    print "   Antenna %2d, %-7s, X Jitter:     %s%% bad, fluctuation=%sdB, normal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('Xproc','-'), kv.get('Xdiff'), kv.get('Xref'))
                if kv.has_key('Ydiff'):
                    print "   Antenna %2d, %-7s, Y Jitter:     %s%% bad, fluctuation=%sdB, normal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUy), kv.get('Yproc','-'), kv.get('Ydiff'), kv.get('Yref'))
                          
            if msg == 'SPURIOUS':
                if kv.has_key('X'): 
                    print "   Antenna %2d, %-7s, X Spurious signals found" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y'): 
                    print "   Antenna %2d, %-7s, Y Spurious signals found" %(lbaNumber, 'RCU %d' %(RCUy))
                    
            if msg == 'FAIL' or msg == 'RF_FAIL':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X RF fail:    signal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('X'))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y RF fail:    signal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUy), kv.get('Y'))    
        
        if part == 'HBA':
            if part != _part:
                _part = part
                hdr = "\n== HBA "
                print hdr + "="*(104-len(hdr))    
            
            if partnumber != -1 and partnumber != _part_nr:
                _part_nr = partnumber
                _c_summator_defect = 0
                header = "Tile %d (RCU %d/%d)" %(partnumber, RCUx, RCUy)
                print "\n-- %s %s" %(header, '-'*(99-len(header)))
                
            if msg == 'NOSIGNAL':
                print "   NO test signal found"
                
            if msg == 'OSCILLATION':
                if kv.has_key('X') or kv.has_key('Xbands'):
                    print "   X Oscillation"
                if kv.has_key('Y') or kv.has_key('Ybands'):
                    print "   Y Oscillation"
                        
            if msg == 'C_SUMMATOR':
                _c_summator_defect = 1
                print "   Modem errors (all elements)"
            
            if msg == 'P_SUMMATOR':
                print "   No RF all elements" 
            
            if msg == 'SUMMATOR_NOISE':
                if kv.has_key('X'):
                    print "   X Summator noise"
                if kv.has_key('Y'):
                    print "   Y Summator noise"
                    
            if msg == 'SPURIOUS':
                if kv.has_key('X'):
                    print "   X Spurious signals"
                if kv.has_key('Y'):
                    print "   Y Spurious signals"
                            
            if msg == 'LOW_NOISE':
                if kv.has_key('Xproc'):
                    print "   X Low Noise:  %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %(kv.get('Xproc'), kv.get('Xval'), kv.get('Xdiff','-'), kv.get('Xref'))
                if kv.has_key('Yproc'):
                    print "   Y Low Noise:  %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %(kv.get('Yproc'), kv.get('Yval'), kv.get('Ydiff','-'), kv.get('Yref'))
                    
            if msg == 'HIGH_NOISE':
                if kv.has_key('Xproc'):
                    print "   X High Noise: %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %(kv.get('Xproc'), kv.get('Xval'), kv.get('Xdiff','-'), kv.get('Xref'))
                if kv.has_key('Yproc'):
                    print "   Y High Noise: %s%% bad, signal=%sdB, fluctuation=%sdB, limit=%sdB" %(kv.get('Yproc'), kv.get('Yval'), kv.get('Ydiff','-'), kv.get('Yref'))
            
            if msg == 'JITTER':
                if kv.has_key('Xdiff'):
                    print "   X Jitter:     %s%% bad, fluctuation=%sdB, normal=%sdB" %(kv.get('Xproc'), kv.get('Xdiff'), kv.get('Xref'))
                if kv.has_key('Ydiff'):
                    print "   Y Jitter:     %s%% bad, fluctuation=%sdB, normal=%sdB" %(kv.get('Yproc'), kv.get('Ydiff'), kv.get('Yref'))
            
            if msg == 'RF_FAIL' or msg == 'RF_TILE_FAIL':
                if kv.has_key('X'):
                    signal_128, sb_128, ref_128, signal_253, sb_253, ref_253 = kv.get('X')
                    print "   X RF Fail:    no-delay(test=%5.1fdB ref=%5.1fdB sb=%d)  full-delay(test=%5.1fdB ref=%5.1fdB sb=%d)" %\
                          (float(signal_128), float(ref_128), int(sb_128), float(signal_253), float(ref_253), int(sb_253))
                if kv.has_key('Y'):
                    signal_128, sb_128, ref_128, signal_253, sb_253, ref_253 = kv.get('Y')
                    print "   Y RF Fail:    no-delay(test=%5.1fdB ref=%5.1fdB sb=%d)  full-delay(test=%5.1fdB ref=%5.1fdB sb=%d)" %\
                          (float(signal_128), float(ref_128), int(sb_128), float(signal_253), float(ref_253), int(sb_253))
            
            if msg == 'E_FAIL':
                # loop over number of elements
                for i in range(1,17,1):
                    if _c_summator_defect:
                        continue
                        
                    if kv.has_key('M%d' %(i)) or kv.has_key('X%d' %(i)) or kv.has_key('Y%d' %(i)) \
                       or kv.has_key('OX%d' %(i)) or kv.has_key('OY%d' %(i)) \
                       or kv.has_key('SPX%d' %(i)) or kv.has_key('SPY%d' %(i)) \
                       or kv.has_key('LNX%d' %(i)) or kv.has_key('HNX%d' %(i)) or kv.has_key('JX%d' %(i)) \
                       or kv.has_key('LNY%d' %(i)) or kv.has_key('HNY%d' %(i)) or kv.has_key('JY%d' %(i)):
                        print "   Element %d" %(i)

                    if kv.has_key('M%d' %(i)):
                        info = kv.get('M%d' %(i))
                        if info == 'error':
                            print "       Modem error"
                        if info == '??':
                            print "       No modem communication"
                    else:
                        if kv.has_key('OX%d' %(i)):
                            print "      X Oscillating" 
                        
                        if kv.has_key('OY%d' %(i)):
                            print "      Y Oscillating" 
                        
                        if kv.has_key('SPX%d' %(i)):
                            print "      X Spurious" 
                        
                        if kv.has_key('SPY%d' %(i)):
                            print "      Y Spurious" 
                                
                        if kv.has_key('LNX%d' %(i)):
                            print "      X Low Noise, signal=%sdB fluctuation=%sdB" %(kv.get('LNX%d' %(i))[0], kv.get('LNX%d' %(i))[1])
                        
                        if kv.has_key('HNX%d' %(i)):
                            print "      X High Noise, signal=%sdB fluctuation=%sdB" %(kv.get('HNX%d' %(i))[0], kv.get('HNX%d' %(i))[1])
                        
                        if kv.has_key('JX%d' %(i)):
                            print "      X Jitter, fluctuation=%sdB" %(kv.get('JX%d' %(i)))    
                            
                        if kv.has_key('LNY%d' %(i)):
                            print "      Y Low Noise, signal=%sdB fluctuation=%sdB" %(kv.get('LNY%d' %(i))[0], kv.get('LNY%d' %(i))[1])
                        
                        if kv.has_key('HNY%d' %(i)):
                            print "      Y High Noise, signal=%sdB fluctuation=%sdB" %(kv.get('HNY%d' %(i))[0], kv.get('HNY%d' %(i))[1])
                        
                        if kv.has_key('JY%d' %(i)):
                            print "      Y Jitter, fluctuation=%sdB" %(kv.get('JY%d' %(i)))    
                                  
                        if kv.has_key('X%d' %(i)):
                            signal_128, sb_128, ref_128, signal_253, sb_253, ref_253 = kv.get('X%d' %(i))
                            print "      X RF Fail:  no-delay(test=%5.1fdB ref=%5.1fdB sb=%d)  full-delay(test=%5.1fdB ref=%5.1fdB sb=%d)" %\
                                  (float(signal_128), float(ref_128), int(sb_128), float(signal_253), float(ref_253), int(sb_253))          
                        
                        if kv.has_key('Y%d' %(i)):
                            signal_128, sb_128, ref_128, signal_253, sb_253, ref_253 = kv.get('Y%d' %(i))
                            print "      Y RF Fail:  no-delay(test=%5.1fdB ref=%5.1fdB sb=%d)  full-delay(test=%5.1fdB ref=%5.1fdB sb=%d)" %\
                                  (float(signal_128), float(ref_128), int(sb_128), float(signal_253), float(ref_253), int(sb_253))
    print '\n'+'#'*103
if __name__ == '__main__':
    main()
