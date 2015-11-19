#!/usr/bin/python

import psycopg2 as pg
import psycopg2.extras as pgdefs
from lofar.messaging import Service

#import logging
#import sys
#logging.basicConfig(stream=sys.stdout, level=logging.INFO)
SERVICENAME = "IngestJobQueryService"
BUSNAME     = "simpletest"
DATABASE    = "datamonitor"
USER        = "peterzon"
PASSWORD    = "welkom001"

class DBlistener:
    def __init__(self):
        self.conn= pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
        self.QueryJobs="select export_jobs.id, observations.obsid, observations.nr_files, export_jobs.description, export_jobs.start, export_jobs.update, export_jobs.username, export_jobs.state, export_jobs.name, export_jobs.project, export_jobs.location from export_jobs,observations where export_jobs.id=observations.job_id order by export_jobs.state, export_jobs.id ,observations.obsid;"
        self.QueryMain="select length from archiving_queues where name='main';"


    def doquery(self,q):
        cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
        cur.execute(q)
        return cur.fetchall()

    def __call__(self,text):
        return self.run(text)

    def run(self,text):
        cmd = text.split(";")
        if (cmd[0]=="ArchivingStatus"):
            maininfo = self.doquery(self.QueryMain)
            jobinfo  = self.doquery(self.QueryJobs)
            return { "main" : maininfo, "jobs" : jobinfo };
        return "Unknown Command"


with Service(SERVICENAME, DBlistener().run, busname=BUSNAME) as GetServerState:
    GetServerState.wait_for_interrupt()
