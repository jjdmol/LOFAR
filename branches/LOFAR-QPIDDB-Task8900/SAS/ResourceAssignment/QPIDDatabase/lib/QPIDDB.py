#!/usr/bin/python
import psycopg2 as pg
import psycopg2.extras as pgdefs



class qpidinfra:
    def __init__(self):
	self.conn=pg.connect("dbname='qpidinfra'")


    def ensure_connect(self):
        if (self.conn and self.conn.status==1):
	    return
	self.conn=pg.connect("dbname='qpidinfra'")
	if (self.conn and self.conn.status==1):
	    return
	else:
	    raise Exception( "Not connected" )

    def doquery(self,query):
	self.ensure_connect()
	cur = self.conn.cursor(cursor_factory = pgdefs.RealDictCursor)
        cur.execute(query)
	return cur.fetchall()

    def docommit(self,query):
	self.ensure_connect()
	cur = self.conn.cursor()
	cur.execute(query)
	print cur.statusmessage
	self.conn.commit()

    def perqueue(self,callback):
	ret=self.doquery("select hostname,queuename from persistentqueues INNER join hosts on (hid=hostid) INNER JOIN queues on (qid=queueid);")
	for item in ret:
	    callback(item['hostname'],item['queuename'])
	
    def perexchange(self,callback):
	ret= self.doquery("select hostname,exchangename from persistentexchanges INNER join hosts on (hid=hostid) INNER JOIN exchanges on (eid=exchangeid);")
	for item in ret:
	    callback(item['hostname'],item['exchangename'])


    def perfederationexchange(self,callback):
	# cur.execute("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename , keyname from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid) JOIN routingkeys on (keyid=kid);")
	ret=self.doquery("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid);")
	for item in ret:
	    callback(item['fromhost'],item['tohost'],item['exchangename'],'#') #item['keyname'])

    def gethostid(self,hostname):
	tmp=self.doquery("select * from hosts where hostname='%s';" %(hostname))
	if (tmp==[]):
	    return 0
	return tmp[0]['hostid']

    def getqueueid(self,queuename):
	tmp=self.doquery("select * from queues where queuename='%s';" %(queuename))
	if (tmp==[]):
	    return 0
	return tmp[0]['queueid']

    def getexchangeid(self,exchangename):
	tmp=self.doquery("select * from exchanges where exchangename='%s';" %(exchangename))
	if (tmp==[]):
	    return 0
	return tmp[0]['exchangeid']

    def addhost(self,hostname):
	id=self.gethostid(hostname)

	if (id!=0):
	    print ("Host %s already available in database with id = %d" %(hostname,id))
	    return

	self.docommit("insert into hosts (hostname) VALUES ('%s');" %(hostname))
	    
    def addqueue(self,queue):
	id=self.getqueueid(queue)
	
	if (id!=0):
	    print ("Queue %s already available in database with id = %d" %(queue,id))
	    return

	self.docommit("insert into queues (queuename)  VALUES ('%s');" %(queue))

    def addexchange(self,exchange):
	id=self.getexchangeid(exchange)

	if (id!=0):
	    print ("Exchange %s already available in database with id = %d" %(exchange,id))
	    return

	self.docommit("insert into exchanges (exchangename) VALUES ('%s');" %(exchange))


    def getqueuebinding(self,queueid,hostid):
	ret=self.doquery("select * from persistentqueues where qid=%s and hid=%s;" %(queueid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pquid']

    def addqueuebinding(self,queueid,hostid):
	if (self.getqueuebinding(queueid,hostid)==0):
	    self.docommit("insert into persistentqueues (qid,hid) VALUES (%d,%d);" %(queueid,hostid))


    def getexchangebinding(self,exchangeid,hostid):
	ret=self.doquery("select * from persistentexchanges where eid=%s and hid=%s;" %(exchangeid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pexid']

    def addexchangebinding(self,exchangeid,hostid):
	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.docommit("insert into persistentexchanges (eid,hid) VALUES ( %s , %s ) ;" %(exchangeid,hostid))

    def getqueueroute(self,queueid,fromid,toid):
	ret=self.doquery("select * from queueroutes where qid=%s and fromhost=%s and tohost=%s;" %(queueid,fromid,toid))
	if (ret==[]):
	    return 0
	return ret[0]['qrouteid']


    def addqueueroute(self,queueid,fromid,toid):
	if (self.getqueueroute(queueid,fromid,toid)==0):
	    self.docommit("insert into queueroutes (qid,fromhost,tohost) VALUES ( %s , %s , %s );" %(queueid,fromid,toid))

    def getexchangeroute(self,exchangeid,routingkey,fromid,toid):
	ret=self.doquery("select * from exchangeroutes where eid=%s and fromhost=%s and tohost=%s and routingkey='%s';" %(exchangeid,fromid,toid,routingkey))
	if (ret==[]):
	    return 0;
	return ret[0]['erouteid']

    def addexchangeroute(self,exchangeid,routingkey,fromid,toid):
	if (getexchangeroute(self,exchangeid,routingkey,fromid,toid)==0):
	    self.docommit("insert into exchangeroutes (eid,fromhost,tohost,routingkey);" %(exchangeid,fromid,toid,routingkey))


    def bindqueuetohost(self,queue,host):
	hostid=self.gethostid(host)
	if (hostid==0):
	    self.addhost(host)
	    hostid=self.gethostid(host)
	
	queueid=self.getqueueid(queue)
	if (queueid==0):
	    self.addqueue(queue)
	    queueid=self.getqueueid(queue)
	
	bindid=self.getqueuebinding(queueid,hostid)
	if (bindid==0): # not found
	    self.addqueuebinding(queueid,hostid)
	else:
	    print ("Queue %s already binded with broker %s in database" %(queue,host))

    def bindexchangetohost(self,exchange,host):
	hostid=self.gethostid(host)
	if (hostid==0):
	    self.addhost(host)
	    hostid=self.gethostid(host)
	
	exchangeid=self.getexchangeid(exchange)
	if (exchangeid==0):
	    self.addexchange(exchange)
	    exchangeid=self.getexchangeid(exchange)


	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.addexchangebinding(exchangeid,hostid)
	else:
	    print("Exchange %s already binded with broker %s in database" %(exchange,host))


    def setqueueroute(self,queuename,fromname,toname):
	fromid=self.gethostid(fromname)
	toid=self.gethostid(toname)
	queueid=self.getqueueid(queuename)
	self.addqueueroute(queueid,fromid,toid)

    def setexchangeroute(self,exchangename,routingkey,fromname,toname):
	exchangeid=self.getexchangeid(exchangename)
	fromid=self.gethostid(fromname)
	toid=self.gethostid(toname)
	self.addexchangeroute(exchangeid,routingkey,fromid,toid)


