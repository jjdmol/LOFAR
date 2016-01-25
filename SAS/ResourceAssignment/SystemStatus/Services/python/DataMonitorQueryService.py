#!/usr/bin/python


import psycopg2 as pg
import psycopg2.extras as pgdefs
from lofar.messaging import Service
from lofar.common.util import waitForInterrupt

import logging
import sys
logging.basicConfig(stream=sys.stdout, level=logging.INFO)
logger = logging.getLogger(__name__)

SERVICENAME = "GetServerState"
BUSNAME     = "simpletest"
DATABASE    = "datamonitor"
USER        = "lofarsys"
PASSWORD    = "welkom001"

class DBlistener:
    def __init__(self):
        global count
        self.conn= pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
	self.DBconnected = (self.conn and self.conn.status==1)
        self.query="select * from hosts inner join datapaths on hosts.id = datapaths.hostid;"
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

    def ensure_connected(self):
	self.DBconnected = (self.conn and self.conn.status==1)
	if not self.DBconnected:
	    try:
		self.conn= pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
		self.DBconnected = (self.conn and self.conn.status==1)
	    except Exception as e:
		logger.error("DB connection could not be restored.")
	return self.DBconnected

    def doquery(self,q):
        cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
        cur.execute(q)
        ret= cur.fetchall()
        return ret

    def getstatenames(self):
        qres=self.doquery(self.Qgetstatenames)
        names={}
        for i in qres:
            names[str(i['id'])]=i['statename']
        return names

    def getactivegroupnames(self):
        names={}
        qres = self.doquery(self.Qgetactivegroupnames)
        for i in qres:
            names[str(i['id'])]=i['groupname']
        return names

    def gethostsforgid(self,gid):
        hosts = self.doquery(self.Qgethostsforgid.replace("GID",gid))
        groups = self.getactivegroupnames()
        ret = { "groupname":groups[str(gid)] , "nodes":hosts }
        return ret

    def counthostsforgroups(self,groups,states):
        qres=self.doquery(self.Qgethostsforgroups)
        ret={}
        for gid,name in groups.iteritems():
            ret[name]={}
            for sid,sname in states.iteritems():
                ret[name][sname]=0
        for row in qres:
            ret[groups[str(row['groupid'])]][states[str(row['statusid'])]]+=1
        return ret


    def __call__(self,text):
        return self.run(text)

    def run(self,text):
	if (self.ensure_connected()==False):
	    raise Exception ("Not connected to Database")

        print text

        # incoming string is processed as <command>[;<param>[;<param>..]]
        cmd=text.split(";")
        print cmd
        if (cmd[0]=="listall"):
            res = self.doquery(self.query)
            nodes={}
            domain={'name':'CEP4','storage':[]}
            ret={'domain':domain,'nodes':nodes}
            try:
              for i in res:
               # new node names need to setup the basic node info first.
               if i['hostname'] not in nodes:
                    nodes[i['hostname']]={'status':i['statusid'],'groupid':i['groupid'],'storage':[]}
               # fill in the node storage info
               nodes[i['hostname']]['storage'].append({'path':i['path'],'totalspace':i['totalspace'],'usedspace':i['usedspace'],'claimedspace':i['claimedspace']})
            except Exception as e:
               # we might not have all the components
               print e

            return ret
        if (cmd[0]=="statenames"):
            ret = self.getstatenames()
            return ret

        if (cmd[0]=="getactivegroups"):
            ret=self.getactivegroupnames()
            return ret

        if (cmd[0]=="countactivehosts"):
            snames=self.getstatenames()
            gnames=self.getactivegroupnames()
            ret=self.counthostsforgroups(gnames,snames)
            return ret

        if (cmd[0]=="gethostsforgid"):
            gid=cmd[1];
            return self.gethostsforgid(gid)


with Service(SERVICENAME,DBlistener().run,busname=BUSNAME) as GetServerState:
    waitForInterrupt()

