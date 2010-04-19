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
def getRotationLines(filename):
    """
    Returns a list containing all lines with rotations
    """
    f = open(filename,'r')
    lines = f.readlines()
    f.close()
    return [ line.strip().split(',') for line in lines[3:]]


##
def getRotationMatrix(line):
    #print line
    station = str(line[0]).upper().strip()
    anttype = str(line[1]).upper().strip()
    # make db matrix [3][3]
    matrix = "ARRAY[[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]]" %\
             (float(line[2]),float(line[3]),float(line[4]), \
              float(line[5]),float(line[6]),float(line[7]), \
              float(line[8]),float(line[9]),float(line[10]))
        
    return(station,anttype,matrix)


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

    #filename = 'rotation-matrices/rotation_matrices.dat'
    
    lines = getRotationLines(filename)
    for line in lines:
        (stationname,anttype,matrix) = getRotationMatrix(line)
        if stationname == 'CS001': print stationname,'  ',anttype,'  ',matrix[0]
        # check stationname
        cursor.execute("select name from station")
        stations = cursor.fetchall()
        
        station = []
        station.append(stationname)
        if station not in stations:
            print "station %s is not a legal stationame" % stationname
            sys.exit(1)
        try:
            db2.query("select * from add_rotation_matrix('%s','%s',%s)" %(stationname, anttype, matrix))
            
            print stationname,'  ',anttype,'  ',matrix
        except:
            print 'ERR, station=%s has no types defined' %(stationname)
        
    print ' Done'
    db1.close()
    db2.close()
    sys.exit(1)