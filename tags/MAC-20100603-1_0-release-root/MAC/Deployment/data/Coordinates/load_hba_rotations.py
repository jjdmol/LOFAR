#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg
from math import *
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

db1 = pgdb.connect(user="postgres", host=dbHost, database=dbName)
cursor = db1.cursor()

# calling stored procedures only works from the pg module for some reason.
db2 = pg.connect(user="postgres", host=dbHost, dbname=dbName)

#
# getRotationLines
#
def getRotationLines(filename):
    """
    Returns a list containing all lines with rotations
    """
    f = open(filename,'r')
    lines = f.readlines()
    f.close()
    return [ line.strip().split(',') for line in lines[1:]]

##
def getRotation(line):
    hba0 = hba1 = 0.0
    station = str(line[0]).upper()
    if line[1] != '':
        hba0 = (int(line[1])/360.) * 2. * pi
    if line[2] != '':
        hba1 = (int(line[2])/360.) * 2. * pi
    return(station,hba0,hba1)

#
# MAIN
#
if __name__ == '__main__':

    # check syntax of invocation
    # Expected syntax: load_measurement stationname objecttypes datafile
    #
    if (len(sys.argv) != 2):
        print "Syntax: %s datafile" % sys.argv[0]
        sys.exit(1)
    filename = str(sys.argv[1])
    
    lines = getRotationLines(filename)
    for line in lines:
        (stationname,rotation0,rotation1) = getRotation(line)
                
        # check stationname
        cursor.execute("select name from station")
        stations = cursor.fetchall()
        
        station = []
        station.append(stationname)
        if station not in stations:
            print "station %s is not a legal stationame" % stationname
            sys.exit(1)
        try:
            if rotation1 == None:
                db2.query("select * from add_field_rotation('%s','HBA',%s)" %( stationname, rotation0))
            if rotation0 != None and rotation1 != None:
                db2.query("select * from add_field_rotation('%s','HBA0',%s)" %( stationname, rotation0))
                db2.query("select * from add_field_rotation('%s','HBA1',%s)" %( stationname, rotation1))
                print 'station=%s  rotation0=%f  rotation1=%f' %(stationname,rotation0, rotation1)
        except:
            print 'ERR, station=%s has no types defined' %(stationname)
        
    print ' Done'
    db1.close()
    db2.close()
    sys.exit(1)