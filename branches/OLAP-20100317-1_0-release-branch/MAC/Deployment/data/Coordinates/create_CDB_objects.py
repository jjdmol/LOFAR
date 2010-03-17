#!/usr/bin/env python
import re,sys,pg

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
    db = pg.connect(user="postgres", host="dop50", dbname="coordtest")

    for station in getStationList():
        print findStationInfo(station)
        if (len(findStationInfo(station)) < 12):
            continue
        (name, stationID, stnType, long, lat, height, nrRSP, nrTBB, nrLBA, nrHBA, HBAsplit, LBAcal ) = findStationInfo(station)
        print "updating %s to the coordinate database" % station
        for lba in xrange(0, int(nrLBA)):
            db.query("select * from add_object('%s', '%s', %d)" % ( name, "LBA", lba ))
        for hba in xrange(0, int(nrHBA)):
            db.query("select * from add_object('%s', '%s', %d)" % ( name, "HBA", hba ))

# ... to be continued
