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
	    callback(item)
	
    def perexchange(self,callback):
	ret= self.doquery("select hostname,exchangename from persistentexchanges INNER join hosts on (hid=hostid) INNER JOIN exchanges on (eid=exchangeid);")
	for item in ret:
	    callback(item)


    def perfederationexchange(self,callback):
	# cur.execute("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename , keyname from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid) JOIN routingkeys on (keyid=kid);")
	ret=self.doquery("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename , dynamic , routingkey from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid);")
	for item in ret:
	    callback(item)

    def perfederationqueue(self,callback):
	ret=self.doquery("select h1.hostname as fromhost ,h2.hostname as tohost , queuename, exchangename from queueroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN queues on (queueid=qid) JOIN exchanges on (exchangeid=eid);")
	for item in ret:
	    callback(item)

    def getid(self,itemtype,itemname):
	tmp=self.doquery("select * from %ss where %sname='%s';" %(itemtype,itemtype,itemname))
	if (tmp==[]):
	    return 0
	return tmp[0]["%sid" %(itemtype)]

    def delid(self,itemtype,itemid):
	if (id!=0):
	    self.docommit("delete from %ss where %sid=%d;"(itemtype,itemtype,itemid))

    def delname(self,itemtype,itemname, verbose=True):
	id= self.getid(itemtype,itemname)
	if (id):
	    if verbose:
		print("Deleting %s from table %ss." %(itemname,itemtype))
	    self.docommit("delete from %ss where %sid=%d and %sname='%s'" %(itemtype,itemtype,itemtype,itemname))
	else:
	    print("%s %s not found in database." %(itemtype,itemname))

    def getname(self,itemtype,itemid):
	res=self.doquery("select %sname from %ss where %sid=%d;" %(itemtype,itemtype,itemtype,itemid))
	if (res!=[]):
	    return res[0]["%sname" %(itemtype)]
	return 'NotAvailableInDatabase'

    def gethostid(self,hostname):
	return self.getid('host',hostname)

    def getqueueid(self,queuename):
	return self.getid('queue',queuename)

    def getexchangeid(self,exchangename):
	return self.getid('exchange',exchangename)

    def additem(self,itemtype,itemname,verbose=True):
	id = self.getid(itemtype,itemname,verbose)
	if (id!=0):
	    if verbose:
		print("%s %s already available in database." %(itemtype,itemname))
	    return id
	self.docommit("insert into %ss (%sname) values ('%s');" %(itemtype,itemtype,itemname))
	if verbose:
	    print (" added %s %s to DB" %(itemtype,itemname))
	return self.getid(itemtype,itemname,verbose)

    def delitem(self,itemtype,itemname,verbose=True):
	id = self.getid(itemtype,itemname,verbose)
	if (id!=0):
	    if verbose:
		print("Deleting from table %s the item %s." %(itemtype,itemname))
	    self.docommit("delete from %ss where %sid=%d and %sname='%s';" %(itemtype,itemtype,id,itemtype,itemname))
	    return 0;
	print("%s %s not found in the database" %(itemtype,itemname))


    def addhost(self,hostname,verbose=True):
	return self.additem('host',hostname,verbose)
	    
    def addqueue(self,queue, verbose=True):
	return self.additem('queue',queue,verbose)

    def addexchange(self,exchange, verbose=True):
	return self.additem('exchange',exchange,verbose)

    def delhost(self,hostname, verbose=True):
	return self.delitem('host',hostname,verbose)

    def delqueue(self,queuename, verbose=True):
	return self.delitem('queue',queuename)

    def delexchange(self,exchangename, verbose=True):
	return self.delitem('exchange',exchangename,verbose)

    def getqueuebinding(self,queueid,hostid):
	ret=self.doquery("select * from persistentqueues where qid=%s and hid=%s;" %(queueid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pquid']

    def addqueuebinding(self,queueid,hostid):
	if (self.getqueuebinding(queueid,hostid)==0):
	    self.docommit("insert into persistentqueues (qid,hid) VALUES (%d,%d);" %(queueid,hostid))

    def delqueuebinding(self,queueid,hostid):
	id = self.getqueuebinding(queueid,hostid)
	if (id):
	    print("Deleting binding for queue %d on host %d" %(queueid,hostid))
	    self.docommit("delete from persistentqueues where pquid=%d and qid=%d and hid=%d;" %(id,queueid,hostid))
	    return 0
	return 1

    def getexchangebinding(self,exchangeid,hostid):
	ret=self.doquery("select * from persistentexchanges where eid=%s and hid=%s;" %(exchangeid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pexid']

    def addexchangebinding(self,exchangeid,hostid):
	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.docommit("insert into persistentexchanges (eid,hid) VALUES ( %s , %s ) ;" %(exchangeid,hostid))

    def delexchangebinding(self,exchangeid,hostid):
	id = self.getexchangebinding(exchangeid,hostid)
	if (id!=0):
	    print("Deleting binding for exchange %d on host %d" %(exchangeid,hostid))
	    self.docommit("delete from persistentexchanges where pexid=%d and eid=%d and hid=%d;" %(id,exchangeid,hostid))
            return 0
	return 1

    def getqueueroute(self,queueid,fromid,toid):
	ret=self.doquery("select * from queueroutes where qid=%s and fromhost=%s and tohost=%s;" %(queueid,fromid,toid))
	if (ret==[]):
	    return 0
	return ret[0]['qrouteid']

    def addqueueroute(self,queueid,fromid,toid,exchangeid):
	if (self.getqueueroute(queueid,fromid,toid)==0):
	    self.docommit("insert into queueroutes (qid,fromhost,tohost,eid) VALUES ( %s , %s , %s, %s );" %(queueid,fromid,toid,exchangeid))

    def delqueueroute(self,queueid,fromid,toid):
	id=self.getqueueroute(queueid,fromid,toid)
	if (id!=0):
	    print("Removing queueroute for queue %d from host %d to host %d" %(queueid,fromid,toid))
	    self.docommit("delete from queueroutes where qrouteid=%d;" %(queuerouteid))

    def getexchangeroute(self,exchangeid,routingkey,fromid,toid):
	ret=self.doquery("select * from exchangeroutes where eid=%s and fromhost=%s and tohost=%s and routingkey='%s';" %(exchangeid,fromid,toid,routingkey))
	if (ret==[]):
	    return 0
	return ret[0]['erouteid']

    def addexchangeroute(self,exchangeid,routingkey,fromid,toid,dynamic=False):
	if (getexchangeroute(self,exchangeid,routingkey,fromid,toid)==0):
	    self.docommit("insert into exchangeroutes (eid,fromhost,tohost,routingkey,dynamic);" %(exchangeid,fromid,toid,routingkey,dynamic))

    def delexchangeroute(self,exchangeid,routingkey,fromid,toid,dynamic=False):
	id = getexchangeroute(self,exchangeid,routingkey,fromid,toid)
	if (id!=0):
	    print("Removing exchangeroute for key %s and exchange %s from host %s to host %s" %(routingkey,exchangekey,fromid,toid))
	    self.docommit("delete from exchangeroutes where erouteid=%d;" %(id))


    def bindqueuetohost(self,queue,host):
	hostid=self.addhost(host)
	queueid=self.addqueue(queue)
	bindid=self.getqueuebinding(queueid,hostid)
	if (bindid==0): # not found
	    self.addqueuebinding(queueid,hostid)
	else:
	    print ("Queue %s already binded with broker %s in database" %(queue,host))

    def bindexchangetohost(self,exchange,host):
	hostid=self.addhost(host,verbose=False)
	exchangeid=self.addexchange(exchange,verbose=False)
	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.addexchangebinding(exchangeid,hostid)
	else:
	    print("Exchange %s already binded with broker %s in database" %(exchange,host))


    def setqueueroute(self,queuename,fromname,toname,exchange):
	fromid = self.addhost(fromname)
	toid = self.addhost(toname)
	queueid = self.addqueue(queuename)
	exchangeid = self.addexchange(exchange)
	self.addqueueroute(queueid,fromid,toid,exchangeid)

    def setexchangeroute(self,exchangename,routingkey,fromname,toname,dynamic=False):
	exchangeid = self.addexchange(exchangename)
	fromid = self.addhost(fromname)
	toid = self.addhost(toname)
	self.addexchangeroute(exchangeid,routingkey,fromid,toid,dynamic)

    def renamequeue(self,oldqueuename,newqueuename):
	self.docommit("update queues set queuename='%s' where queuename='%s';" %(newqueuename,oldqueuename))

    def renameexchange(self,oldexchangename,newexchangename):
	self.docommit("update exchanges set exchangename='%s' where exchangename='%s';" %(newexchangename,oldexchangename))


