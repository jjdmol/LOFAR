#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

#
# getHeaderLines
#
def getHeaderLines(filename):
    """
    Returns a list containing all lines that do NOT contains coordinate data.
    """
    pattern=re.compile("^[a-zA-Z]+.*", re.IGNORECASE | re.MULTILINE)
    answer = {}
    for line in pattern.findall(open(filename).read()):
        if line.count(';') == 1:
            (key, value) = line.split(';') 
            answer[key]=value
    return answer

#
# getCoordLines
#
def getCoordLines(filename):
    """
    Returns a list containing all lines with coordinates
    """
    pattern=re.compile("^[0-9]+;.*", re.IGNORECASE | re.MULTILINE)
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

    # process metadata info
    stationname = objecttype = refSys = refFrame = method = date = pers1 = pers2 = pers3 = absRef = derived = comment = ""
    metadata = getHeaderLines(sys.argv[1])
    if metadata.has_key("stationname"): stationname = metadata["stationname"]
    if metadata.has_key("infotype"): objecttype = metadata["infotype"]
    if metadata.has_key("ref_system"): refSys = metadata["ref_system"]
    if metadata.has_key("ref_frame"):  refFrame = metadata["ref_frame"]
    if metadata.has_key("method"): method = metadata["method"]
    if metadata.has_key("measure_date"): date = metadata["measure_date"]
    if metadata.has_key("person1"): pers1 = metadata["person1"]
    if metadata.has_key("person2"): pers2 = metadata["person2"]
    if metadata.has_key("person3"): pers3 = metadata["person3"]
    if metadata.has_key("absolute_reference"): absRef = metadata["absolute_reference"]
    if metadata.has_key("comment"): comment = metadata["comment"]

    # check some data against the database
    station = []
    station.append(stationname)
    objtype = []
    objtype.append(objecttype)

    db = pgdb.connect(user="postgres", host=dbHost, database=dbName)
    cursor = db.cursor()
    # check stationname
    cursor.execute("select name from station")
    stations = cursor.fetchall()
    if station not in stations:
        print "station %s is not a legal stationame" % stationname
        sys.exit(1)
    #check objecttype
    cursor.execute("select * from object_type")
    objecttypes = cursor.fetchall()
    if objtype not in objecttypes:
        print "objecttype must be one of: ",  objecttypes
        sys.exit(1)
    # check person1
    cursor.execute("select name from personnel where name = '%s'" % pers1 )
    if cursor.rowcount != 1:
        print "Person: '%s' is not in the personnel file, add it (Y/N)?" % pers1
        if raw_input().upper() == "Y":
            insertcmd = db.cursor();
            insertcmd.execute("insert into personnel values ('%s')" % pers1)
            db.commit()
        else:
            sys.exit(1);
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
    # check person3
    cursor.execute("select name from personnel where name = '%s'" % pers3 )
    if cursor.rowcount != 1:
        print "Person: '%s' is not in the personnel file, add it (Y/N)?" % pers3
        if raw_input().upper() == "Y":
            insertcmd = db.cursor();
            insertcmd.execute("insert into personnel values ('%s')" % pers3)
            db.commit()
        else:
            sys.exit(1);
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

    if raw_input('Continue processing this file (Y/N)?').upper() != "Y":
       sys.exit(1)

    # calling stored procedures only works from the pg module for some reason.
    db = pg.connect(user="postgres", host=dbHost, dbname=dbName)
    for cline in getCoordLines(sys.argv[1]):
        ( number, X, Y, Z, sX, sY, sZ ) = cline.split(';')
        print objecttype, number
        # make sure the object exists
        db.query("select * from add_object('%s','%s',%s)" % ( stationname, objecttype, number))
        # add the coord.
        db.query("select * from add_ref_coord('%s','%s',%s,%s,%s,%s,%s,%s,%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')" % ( stationname, objecttype, number, X, Y, Z, sX, sY, sZ, refSys, refFrame, method, date, pers1, pers2, pers3, absRef, derived, comment))
