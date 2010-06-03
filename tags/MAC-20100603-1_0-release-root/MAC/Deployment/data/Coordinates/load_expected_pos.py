#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

#
# getCoordLines
#
def getCoordLines(filename):
    """
    Returns a list containing all lines with coordinates
    """
    pattern=re.compile(r"^[HL]{1}[0-9]+,.*", re.IGNORECASE | re.MULTILINE)
    return [ line for line in pattern.findall(open(filename).read())]

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
    stationname = filename[ filename.find('/')+1 : filename.find('/')+1 + 5].upper()
    objecttype = 'LBA, HBA'
    refSys   = 'ETRS89'
    refFrame = 'ETRF89'
    method   = 'derived'
    date     = '2010-01-01'
    pers1    = 'Brentjens'
    pers2    = 'Donker'
    pers3    = ''
    derived  = ''
    absRef   = ''
    comment  = 'expected coordinates, Brentjens'
    # check some data against the database
    station = []
    station.append(stationname)
    
    db = pgdb.connect(user="postgres", host=dbHost, database=dbName)
    cursor = db.cursor()
    
    # check person2
    cursor.execute("select name from personnel where name = '%s'" % pers2 )
    if cursor.rowcount != 1:
        print "Person: '%s' is not in the personnel file, add it (Y/N)?" % pers2
        if raw_input().upper() == "Y":
            insertcmd = db.cursor();
            insertcmd.execute("insert into personnel values ('%s')" % pers2)
            db.commit()
        else:
            sys.exit(1);
    
    # check stationname
    cursor.execute("select name from station")
    stations = cursor.fetchall()
    if station not in stations:
        print "station %s is not a legal stationame" % stationname
        sys.exit(1)
    db.close()

    # show metadata to user
    print 'station              : ', stationname
    print 'object types         : ', objecttype
    print 'reference system     : ', refSys
    print 'reference frame      : ', refFrame
    print 'measurement method   : ', method
    print 'measurement date     : ', date
    print 'person 1             : ', pers1
    print 'person 2             : ', pers2
    print 'person 3             : ', pers3
    print 'absolute reference   : ', absRef
    print 'comment              : ', comment

    #if raw_input('Continue processing this file (Y/N)?').upper() != "Y":
    #   sys.exit(1)
    
    print 'processing ',
    sys.stdout.flush()
    # calling stored procedures only works from the pg module for some reason.
    db = pg.connect(user="postgres", host=dbHost, dbname=dbName)
    sX = sY = sZ = 0
    pol = 2 # number of polarizations
    for cline in getCoordLines(sys.argv[1]):
        (name,X,Y,Z,P,Q,R,rcuX,rcuY) = cline.strip().split(',')
        
        # set object type (LBA, HBA, HBA0 or HBA1)
        if name[:1] == 'L':
            objecttype = 'LBA'

        elif name[:1] == 'H':
            if stationname[:1] == 'C': # core station 2 hba fields of 24 tiles each
                if int(name[1:]) < 24:
                    objecttype = 'HBA0'
                else:
                    objecttype = 'HBA1'
            else:                      # remote station or internation station one hba filed
                objecttype = 'HBA'
            
        print name,
        sys.stdout.flush()
        
        # add RCU X coordinates
        number = int(name[1:]) * pol
        #print objecttype, number
        # make sure the object exists
        db.query("select * from add_object('%s','%s',%s)" % ( stationname, objecttype, number))
        # add the coord.
        db.query("select * from add_ref_coord('%s','%s',%s,%s,%s,%s,%s,%s,%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')" %\
            ( stationname, objecttype, number, X, Y, Z, sX, sY, sZ, refSys, refFrame, method, date, pers1, pers2, pers3, absRef, derived, comment))
        
        # add RCU Y coordinates
        number = (int(name[1:]) * pol) + 1
        #print objecttype, number
        # make sure the object exists
        db.query("select * from add_object('%s','%s',%s)" % ( stationname, objecttype, number))
        # add the coord.
        db.query("select * from add_ref_coord('%s','%s',%s,%s,%s,%s,%s,%s,%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')" %\
            ( stationname, objecttype, number, X, Y, Z, sX, sY, sZ, refSys, refFrame, method, date, pers1, pers2, pers3, absRef, derived, comment))
    print ' Done'