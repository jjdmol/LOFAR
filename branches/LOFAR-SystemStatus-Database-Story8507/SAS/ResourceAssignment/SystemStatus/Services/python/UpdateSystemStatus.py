#!/usr/bin/env python
from lofar.messaging.Service import Service
import psycopg2 as pg
import psycopg2.extras as pgdefs

# create service:


DATABASE="systemstatus"
USER="systemstatus"
PASSWORD="welkom001"


def AddJobID( Text ):
    global conn
    ret="Unknown Result"
    print "AddJobID incoming : %s" %(str(Text))
    #check if message has version >=0.1 and <1.0
    if (Text["Version Major"]==0):
      if (Text["Version Minor"]>=1):
        ID=Text["Id"]
        JobID=Text["JobID"]
        NumFiles=Text["NumFiles"]
        with conn.cursor() as cur:
          cur.execute("INSERT INTO observations (id, job_id,nr_files) VALUES ('%s','%s','%s');" %(ID,JobID,NumFiles))
          conn.commit()
          ret="Success"
    return ret

# Used settings
ServiceName="AddObservation"
#BusName="simpletest"
BusName="simpletest"



try:
    conn = pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
except Exception as e:
    print e
else:
    with Service(ServiceName,AddJobID,busname=BusName) as myserv:
        # wait for control-C or kill
        myserv.WaitForInterrupt()

