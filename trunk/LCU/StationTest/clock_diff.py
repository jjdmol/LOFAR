#!/usr/bin/python

import sys
import time
import datetime

if len(sys.argv) == 1:
    print '============================='
    print 'No argumetns found'
    print 'usage: clock_diff.py 20091208'
    print '============================='
    exit(0)

dir = r'/var/log/ntpstats/'
file = 'clockstats.'+str(sys.argv[1])

fullfilename = dir + file
print 'loading ', fullfilename
f = open(fullfilename, mode='r')
clock = []
clock = clock + f.readlines()
f.close()

def secs2hms(secs):
    hours = int(secs / 3600.)
    minutes = int((secs - (hours * 3600)) / 60.)
    seconds = secs - (hours * 3600.) - (minutes * 60.)
    return(hours, minutes, seconds)

if __name__ == "__main__":
    missedSecs = 0
    doubleSecs = 0
    lastSec = -1.0
    for dp in clock:
        # skip info
        if len(dp) < 148:
            continue
        
        dp = dp.replace('  ',' ')
        sd = dp.strip('\n').strip().split(' ')      #sd = splitted data
       
        nowSec = float(sd[1])
        if lastSec >= 0:
            diff = nowSec - lastSec
            missedSec = int(round(diff - 1.0))
            (hn, mn, sn) = secs2hms(nowSec)
            (hl, ml, sl) = secs2hms(lastSec)
            if missedSec > 0:
                missedSecs += missedSec
                
                if missedSec == 1:
                    print '%d second  missing (%8.3f)%d:%02d:%06.3f - (%8.3f)%d:%02d:%06.3f' %\
                          (missedSec, nowSec, hn, mn, sn, lastSec, hl, ml, sl)
                else:
                    print '%d seconds missing (%8.3f)%d:%02d:%06.3f - (%8.3f)%d:%02d:%06.3f' %\
                          (missedSec, nowSec, hn, mn, sn, lastSec, hl, ml, sl)
            if missedSec < 0:
                doubleSecs += (missedSec * -1)
                print 'dupplicate second (%8.3f)%d:%02d:%06.3f - (%8.3f)%d:%02d:%06.3f' %\
                          (nowSec, hn, mn, sn, lastSec, hl, ml, sl)

            lastSec = nowSec
        else:
            lastSec = nowSec
    print 'Total missed seconds ', missedSecs
    print 'Total dupplicate seconds ', doubleSecs
       
    
