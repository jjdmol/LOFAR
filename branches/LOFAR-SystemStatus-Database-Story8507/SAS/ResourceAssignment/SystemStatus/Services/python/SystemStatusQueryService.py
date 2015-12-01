#!/usr/bin/python


import psycopg2 as pg
import psycopg2.extras as pgdefs
from lofar.messaging import Service

#import logging
#import sys
#logging.basicConfig(stream=sys.stdout, level=logging.INFO)

SERVICENAME = "GetServerState"
BUSNAME     = "simpletest"
DATABASE    = "datamonitor"
USER        = "peterzon"
PASSWORD    = "welkom001"

conn = pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
query="select * from hosts inner join datapaths on hosts.id = datapaths.hostid;"

class DBlistener:
    def __init__(self):
        global count
        self.conn= pg.connect("dbname=%s user=%s password=%s" % (DATABASE,USER,PASSWORD))
        self.query="select * from hosts where groupid=%s inner join datapaths on hosts.id = datapaths.hostid;"

    def get_groups():
        grouptoid  = {}
        with self.conn.cursor(cursor_factory = pgdefs.RealDictCursor) as cur:
            cur.execute("select * from servergroups;")
            for domain in cur.fetchall():
                grouptoid[domain['groupname']]=domain['id']
        return grouptoid


    def __call__(self,text):
        grouptoid = get_groups()
        if text not in grouptoid:
            return "Not Available"
        with self.conn.cursor(cursor_factory = pgdefs.RealDictCursor) as cur:
            cur.execute(self.query % grouptoid[text] )
            res = cur.fetchall()
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


myfn=DBlistener()

with Service(SERVICENAME,myfn,busname=BUSNAME) as myserv:
    myserv.WaitForInterrupt()

