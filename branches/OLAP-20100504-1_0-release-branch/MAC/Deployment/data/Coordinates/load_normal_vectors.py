#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg
import numpy as np
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
def getLines(filename):
    """
    Returns a list containing all lines with normal vectors
    """
    f = open(filename,'r')
    lines = f.readlines()
    f.close()
    return [ line.strip().split(',') for line in lines[3:]]


##
def getNormalVector(line):
    #print line
    station = str(line[0]).upper().strip()
    anttype = str(line[1]).upper().strip()
    # make db vector [3]
    vector = "ARRAY[%f,%f,%f]" %\
             (float(line[2]),float(line[3]),float(line[4]))
        
    return(station,anttype,vector)


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
    
    #filename = 'rotation-matrices/normal_vectors.dat'
    
    lines = getLines(filename)
    for line in lines:
        (stationname,anttype,vector) = getNormalVector(line)
        # check stationname
        cursor.execute("select name from station")
        stations = cursor.fetchall()
        
        station = []
        station.append(stationname)
        if station not in stations:
            print "station %s is not a legal stationame" % stationname
            sys.exit(1)
        try:
            db2.query("select * from add_normal_vector('%s','%s',%s)" %(stationname, anttype, vector))
            print "%s    %s    %s" %(stationname,anttype,vector)
        except:
            print 'ERR, station=%s has no types defined' %(stationname)
        
    print ' Done'
    db1.close()
    db2.close()
    sys.exit(1)