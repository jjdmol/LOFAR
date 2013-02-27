#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb
from copy import deepcopy
from math import *


INTRO=""" 
Created a file containing all antenna coordinates for the online software.
""" 

def print_help():
    print "Usage: make_antenna_list [<stationname>]"

#
# findStationInfo(stationName)
#
def findStationInfo(stationName):
    """
    Return all basic station info (eg. nr RSPboards) from a station.
    """
    pattern=re.compile("^"+stationName+"[ \t].*", re.IGNORECASE | re.MULTILINE)
    match = pattern.search(open("../StaticMetaData/StationInfo.dat").read())
    if not match:
        raise "\nFatal error: "+stationName+" is not defined in file 'StationInfo.dat'"
    return match.group().split()

#
# MAIN
#
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print_help()
        sys.exit(0)

    (name, stationID, stnType, long, lat, height, nrRSP, nrTBB, nrLBA, nrHBA, HBAsplit, LBAcal ) = findStationInfo(sys.argv[1])
    db = pgdb.connect(user="postgres", host="dop50", database="coordtest")
    print "#Stn	ID	Type	RSP	RCU	Pol	Position					Orientation"
    print "%s	%s	%s	%d	%d	-1	[%s,%s,%s]	[0,0,0]" % (name, stationID, "center", -1, -1, long, lat, height)
    for infoType in [ 'marker', 'lba', 'hba' ]:
        cursor = db.cursor()
        cursor.execute("select * from get_ref_objects(%s, %s)", (sys.argv[1], infoType))
        counter = 0
        while (1):
            record = cursor.fetchone()
            if record == None:
                break
            RSPnr = int(record[2]%100/4)
            print "%s	%s	%s%d	%d	%d	x	[%s,%s,%s]	[0,0,0]" % (name, stationID, infoType, int(record[2])%100, RSPnr, counter, record[3], record[4], record[5])
            print "%s	%s	%s%d	%d	%d	y	[%s,%s,%s]	[0,0,0]" % (name, stationID, infoType, int(record[2])%100, RSPnr, counter+1, record[3], record[4], record[5])
            counter = counter + 2
    db.close()
    sys.exit(1)

