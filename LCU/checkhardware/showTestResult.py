#!/usr/bin/python

# Show logfile

import sys
import os
import string

libPath = os.path.join(os.getcwd(),'lib')
sys.path.insert(0, libPath)

from general_lib import *
from lofar_lib import *

def printHelp():
    print "possible option for this script"
    print "-------------------------------"
    print "-h  print this help screen"
    print "-s  station to show 'CS002C'"
    print "-------------------------------"
    sys.exit(0)
    

i = 1
args = dict()
while i < len(sys.argv):
    if sys.argv[i][0] == '-':
        opt = sys.argv[i][1]
        if opt in ('s',):
            optval = sys.argv[i+1]
            args[opt] = optval.upper()
            i += 2
        else:
            args[opt] = '-'
            i += 1       

if args.has_key('h'):
    printHelp()
StID = args.get('s', getHostName())

#print os.path.join(os.getcwd(),'log')


def main():
    f = open("checkHardware.conf", 'r')
    data = f.readlines()
    f.close()
    for line in data:
        if line.find('log-dir') != -1:
            key, logdir = line.strip().split('=')
    
    testfilename = '%s_StationTest.csv' %(StID)
    fullFilename = os.path.join(logdir, testfilename)
    f = open(fullFilename, 'r')
    data = f.readlines()
    f.close()
    
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
    
    for line in data:
        partnumber = -1
        if line[0] == '#':
            continue
        d = line.strip().split(',')
        if len(d) < 4:
            continue
        date = d[0]
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
            print " Test date : %s-%s-%s" %(date[6:], date[4:6], date[:4])
            print " Tests done: %s" %(string.join(d[4:],', ')) 
            
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
        
        
        if part == 'LBL':
            if part != _part:
                _part = part
                hdr = "\n== LBA Low "
                print hdr + "="*(104-len(hdr))    

            lbaNumber = partnumber + 48
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
                print "   Antenna %2d, %-11s, has Fallen >> X=%sdB Xoffset=%s  Y=%sdB Yoffset=%s" %\
                      (lbaNumber, 'RCU %d/%d' %(RCUx, RCUy), kv.get('X',('?',)), kv.get('Xoff',('?',)), kv.get('Y',('?',)), kv.get('Yoff',('?',)))
            
            if msg == 'LOW_NOISE':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X Low Noise" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y Low Noise" %(lbaNumber, 'RCU %d' %(RCUy))
                    
            if msg == 'HIGH_NOISE':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X High Noise" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y High Noise" %(lbaNumber, 'RCU %d' %(RCUy))
                    
            if msg == 'FAIL':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X RF fail  >>  signal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('X'))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y RF fail  >>  signal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUy), kv.get('Y'))    

        
        if part == 'LBH':
            if part != _part:
                _part = part
                hdr = "\n== LBA High "
                print hdr + "="*(104-len(hdr))    

            lbaNumber = partnumber
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
                      (kv.get('AVG','???'))

            if msg == 'DOWN':
                print "   Antenna %2d, %-11s, has Fallen >> X=%sdB Xoffset=%s  Y=%sdB Yoffset=%s" %\
                      (lbaNumber, 'RCU %d/%d' %(RCUx, RCUy), kv.get('X',('?',)), kv.get('Xoff',('?',)), kv.get('Y',('?',)), kv.get('Yoff',('?',)))
            
            if msg == 'LOW_NOISE':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X Low Noise" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y Low Noise" %(lbaNumber, 'RCU %d' %(RCUy))
                    
            if msg == 'HIGH_NOISE':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X High Noise" %(lbaNumber, 'RCU %d' %(RCUx))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y High Noise" %(lbaNumber, 'RCU %d' %(RCUy))
            
            if msg == 'FAIL':
                if kv.has_key('X'):
                    print "   Antenna %2d, %-7s, X RF fail  >>  signal=%sdB" %\
                          (lbaNumber, 'RCU %d' %(RCUx), kv.get('X'))
                if kv.has_key('Y'):
                    print "   Antenna %2d, %-7s, Y RF fail  >>  signal=%sdB" %\
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
                
            if msg == 'C_SUMMATOR':
                _c_summator_defect = 1
                print "   NO modem communication all elements"
            
            if msg == 'P_SUMMATOR':
                print "   No RF all elements" 
            
            if msg == 'LOW_NOISE':
                if kv.has_key('X'):
                    print "   X Low Noise"
                if kv.has_key('Y'):
                    print "   Y Low Noise"
                    
            if msg == 'HIGH_NOISE':
                if kv.has_key('X'):
                    print "   X High Noise"
                if kv.has_key('Y'):
                    print "   Y High Noise"
            
            if msg == 'FAIL':
                # loop over number of elements
                for i in range(1,17,1):
                    
                    if (not _c_summator_defect and kv.has_key('M%d' %(i))) or kv.has_key('X%d' %(i)) or kv.has_key('Y%d' %(i)) or kv.has_key('OSCX%d' %(i)) or kv.has_key('OSCY%d' %(i)):
                        print "   Element %d" %(i)

                    if (not _c_summator_defect and kv.has_key('M%d' %(i))):
                        info = kv.get('M%d' %(i))
                        if info == 'error':
                            print "       Modem error"
                        if info == '??':
                            print "       No modem communication"
                    else:
                        if kv.has_key('LNX%d' %(i)):
                            print "      X Low Noise"
                        
                        if kv.has_key('HNX%d' %(i)):
                            print "      X High Noise"
                            
                        if kv.has_key('LNY%d' %(i)):
                            print "      Y Low Noise"
                        
                        if kv.has_key('HNY%d' %(i)):
                            print "      Y High Noise"
                                  
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
