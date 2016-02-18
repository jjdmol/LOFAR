#!/usr/bin/python
import psycopg2 as pg
import psycopg2.extras as pgdefs



class qpidinfra:
    def __init__(self):
	print "    "

    def perqueue(self,callback):
	self.conn=pg.connect("dbname='qpidinfra'")
	if (self.conn and self.conn.status==1):
	    cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
	    cur.execute("select hostname,queuename from persistentqueues INNER join hosts on (hid=hostid) INNER JOIN queues on (qid=queueid);")
	    ret = cur.fetchall();
	    for item in ret:
		callback(item['hostname'],item['queuename'])
	else:
	    print "Not connected"
	
    def perexchange(self,callback):
	self.conn=pg.connect("dbname='qpidinfra'")

	if (self.conn and self.conn.status==1):
	    cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
	    cur.execute("select hostname,exchangename from persistentexchanges INNER join hosts on (hid=hostid) INNER JOIN exchanges on (eid=exchangeid);")
	    ret = cur.fetchall();
	    for item in ret:
		callback(item['hostname'],item['exchangename'])
        else:
	    print "Not connected"


    def perfederationexchange(self,callback):
	self.conn=pg.connect("dbname='qpidinfra'")
	if (self.conn and self.conn.status==1):
	    cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
	    # cur.execute("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename , keyname from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid) JOIN routingkeys on (keyid=kid);")
	    cur.execute("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid);")
	    ret = cur.fetchall();
	    for item in ret:
		callback(item['fromhost'],item['tohost'],item['exchangename'],'#') #item['keyname'])
	else:
	    print "Not connected"

