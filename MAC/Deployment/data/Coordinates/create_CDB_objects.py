#!/usr/bin/env python
import re,sys,pg

dbName="donker"
dbHost="10.87.2.185"
#dbHost="dop50"

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
# getStationList
#
def getStationList():
    """
    Returns a list containing all stationnames
    """
    pattern=re.compile("^[A-Z]{2}[0-9]{3}[ \t].*", re.IGNORECASE | re.MULTILINE)
    return [ station.split()[0] for station in pattern.findall(open("../StaticMetaData/StationInfo.dat").read())]

#
# MAIN
#
if __name__ == '__main__':
    print "Connecting to database coordtest"
    db = pg.connect(user="postgres", host=dbHost, dbname=dbName)
    
    pol = 2 # number of polarizations
    for station in getStationList():
        print findStationInfo(station)
        if (len(findStationInfo(station)) < 12):
            continue
        (name, stationID, stnType, long, lat, height, nrRSP, nrTBB, nrLBA, nrHBA, HBAsplit, LBAcal ) = findStationInfo(station)
        if long != '0.0':
            print "updating %s to the coordinate database " % station
            for lba in xrange(0, int(nrLBA)*2):
                db.query("select * from add_object('%s', '%s', %d)" % ( name, "LBA", lba ))
            if HBAsplit == 'Yes':
                for hba in xrange(0, int(nrHBA)):
                    db.query("select * from add_object('%s', '%s', %d)" % ( name, "HBA0", hba ))
                for hba in xrange(int(nrHBA), int(nrHBA)*2):
                    db.query("select * from add_object('%s', '%s', %d)" % ( name, "HBA1", hba ))
            else:
                for hba in xrange(0, int(nrHBA)*2):
                    db.query("select * from add_object('%s', '%s', %d)" % ( name, "HBA", hba ))

# ... to be continued
