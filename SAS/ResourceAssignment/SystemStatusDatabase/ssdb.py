#!/usr/bin/python


import psycopg2 as pg
import psycopg2.extras as pgdefs
from lofar.messaging import Service
from lofar.common.util import waitForInterrupt
from lofar.common import dbcredentials

import logging
import sys
logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

class SSDB:
    def __init__(self,**kwargs):
        self.dbcreds =  kwargs.pop("dbcreds", None)
        self.conn = None
        self.DBconnected = (self.conn and self.conn.status==1)
        self.Qlistall="select * from hosts inner join datapaths on hosts.id = datapaths.hostid;"
        self.Qgetstatenames="select statename,id from states;"
        self.Qgetactivegroupnames="select groupname,id from servergroups where active='y';"
        self.Qgethostsforgroups="select hostname,groupid,statusid from hosts;"
        self.Qgethostsforgid= "select hosts.hostname,states.id,states.statename, \
                 servergroups.groupname,datapaths.path,datapaths.totalspace, \
                 datapaths.usedspace,datapaths.claimedspace \
            from hosts,states,servergroups,datapaths \
            where states.id=hosts.statusid \
                  and servergroups.id=hosts.groupid \
                  and servergroups.id = GID \
                  and datapaths.hostid=hosts.id \
            order by hosts.hostname,datapaths.path;"
        self.QueryIngestJobs="select export_jobs.id, observations.obsid, observations.nr_files, export_jobs.description, export_jobs.start, export_jobs.update, export_jobs.username, export_jobs.state, export_jobs.name, export_jobs.project, export_jobs.location from export_jobs,observations where export_jobs.id=observations.job_id order by export_jobs.state, export_jobs.id ,observations.obsid;"
        self.QueryIngestMain="select length from archiving_queues where name='main';"

    def ensure_connected(self):
        self.DBconnected = (self.conn and self.conn.status==1)
        if not self.DBconnected:
            try:
                self.conn = pg.connect(host=self.dbcreds.host,
                                       user=self.dbcreds.user,
                                       password=self.dbcreds.password,
                                       database=self.dbcreds.database,
                                       connect_timeout=5)
                self.DBconnected = (self.conn and self.conn.status==1)
            except Exception as e:
                logger.error("[SSDBconnector]: DB connection could not be restored: %s" % (e,))
        return self.DBconnected

    def _doquery(self,q):
        self.ensure_connected()
        cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
        cur.execute(q)
        ret= cur.fetchall()
        return ret

    def _dolquery(self,q):
        cur = self.conn.cursor()
        cur.execute(q)
        ret= cur.fetchall()
        return ret


    def getstatenames(self):
        return self._doquery(self.Qgetstatenames)

    def getactivegroupnames(self):
        return self._doquery(self.Qgetactivegroupnames)

    def gethostsforgid(self,gid):
        return self._doquery(self.Qgethostsforgid.replace("GID",gid))

    def gethostsforgroups(self):
        return self._doquery(self.Qgethostsforgroups)

    def listall(self):
        return self._doquery(self.Qlistall)

    def getIngestJobs(self):
        return self._doquery(self.QueryIngestJobs)

    def getIngestMain(self):
        return self._doquery(self.QueryIngestMain)
