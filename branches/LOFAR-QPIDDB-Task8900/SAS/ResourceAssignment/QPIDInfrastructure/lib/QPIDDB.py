#!/usr/bin/python

from psqlQPIDDB import psqlQPIDDB

class qpidinfra:
    """ Class to access and edit the QPIDInfra database.
    """
    def __init__(self):
	""" Initialize the database connection.
	"""
	self.db=psqlQPIDDB('qpidinfra')

    def perqueue(self,callback):
	""" Iterate over all queues defined in the database.
	example:
	    def callback(item):
	        print(" Host %s has Queue %s " %(item['hostname'],item['queuename']))
	
	    qpidinfra.perqueue(callback)
	
	the example will print a full list of hostnames and queuenames
	"""
	ret=self.db.doquery("select hostname,queuename from persistentqueues INNER join hosts on (hid=hostid) INNER JOIN queues on (qid=queueid);")
	for item in ret:
	    callback(item)
	
    def perexchange(self,callback):
	""" Iterate over all queues defined in the database.
	example:
	    def callback(item):
	        print(" Host %s has Exchange %s " %(item['hostname'],item['queuename']))
		
	    qpidinfra.perexchange(callback)
	
	the example will print a full list of hostnames and exchange names
	"""
	ret= self.db.doquery("select hostname,exchangename from persistentexchanges INNER join hosts on (hid=hostid) INNER JOIN exchanges on (eid=exchangeid);")
	for item in ret:
	    callback(item)


    def perfederationexchange(self,callback):
	""" Iterate over all routingkeys defined in all federated exchanges.
	example:
	    def callback(item):
	        fedtype = "Dynamic" if item['dynamic'] else "Static"
	        print(" %s Federation from %s to %s with routingkey %s for exchange %s" \
			%(fedtype,item['fromhost'],item['tohost'],item['routingkey'],item['exchangename']))
	
	    qpidinfra.perfederationexchange(callback)

	the example will return a full list of the federated exchanges.
	"""
	ret=self.db.doquery("select h1.hostname as fromhost ,h2.hostname as tohost , exchangename , dynamic , routingkey from exchangeroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN exchanges on (exchangeid=eid);")
	for item in ret:
	    callback(item)

    def perfederationqueue(self,callback):
	""" Iterate over all federated queues.
	example:
	    def callback(item):
	        print("  Federation for queue %s from %s to %s using exchange %s" \
			%(item['queuename'],item['fromhost'],item['tohost'],item['exchangename']))
	
	    qpidinfra.perfederationqueue(callback)

	the example will return a full list of the federated queues.
	"""

	ret=self.db.doquery("select h1.hostname as fromhost ,h2.hostname as tohost , queuename, exchangename from queueroutes JOIN hosts as h1 on (fromhost=h1.hostid) JOIN hosts as h2 on (tohost=h2.hostid) JOIN queues on (queueid=qid) JOIN exchanges on (exchangeid=eid);")
	for item in ret:
	    callback(item)

    def gethostid(self,hostname):
	""" return the database id of the given hostname or 0 if non existant.
	example:
	    id = gethostid('myhost.my.domain')
	"""
	return self.db.getid('host',hostname)

    def getqueueid(self,queuename):
	""" return the database id of the given queuename or 0 if non existant.
	example:
	    id = getqueueid('my.queue.name')
	"""
	return self.db.getid('queue',queuename)

    def getexchangeid(self,exchangename):
	""" return the id of the given exchangename or 0 if non existant.
	example:
	    id = getexchangeid('my.exchange.name')
	"""
	return self.db.getid('exchange',exchangename)

    def addhost(self,hostname,verbose=True):
	""" Add a hostname to the database. Hostnames will be stored in lowercase. Returns the id of the new entry.
	example:
	    id = addhost('myhost.my.domain')
	"""
	return self.db.additem('host',hostname,verbose)
	    
    def addqueue(self,queue, verbose=True):
	""" Add a queuename to the database. Returns the id of the new entry.
        example:
	    id = addqueue('my.queue.name')
	"""
	return self.db.additem('queue',queue,verbose)

    def addexchange(self,exchange, verbose=True):
	""" Add a exchangename to the database. Returns the id of the new entry.
	example:
	    id = addexchange('my.exchange.name')
	"""

	return self.db.additem('exchange',exchange,verbose)

    def delhost(self,hostname, verbose=True):
	""" Delete the entry for the hostname and its associated bindings/federations.
	example:
	    delhost('myhost.my.domain')
	"""
	return self.db.delitem('host',hostname,verbose)

    def delqueue(self,queuename, verbose=True):
	""" Delete the queue definition from the list of available queuenames and remove all related bindings and federations.
	Use with care because this removes ALL the occurances of this queue on ALL hosts and related bindings/federations.
	example:
	    delqueue('my.queue.name')
	"""
	return self.db.delitem('queue',queuename)

    def delexchange(self,exchangename, verbose=True):
        """ Delete the exchange definition from the list of available exchangenames and remove all related bindings and federations.
        Use with care because this removes ALL the occurances of this exchange on ALL hosts and related bindings/federations.
	example:
	    delexchange('my.queue.name')
	"""
	return self.db.delitem('exchange',exchangename)

    def getqueuebinding(self,queueid,hostid):
	""" Retrieve the binding description for the given queueid and host id.
	returns 0 if none found.
	"""
	ret=self.db.doquery("select * from persistentqueues where qid=%s and hid=%s;" %(queueid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pquid']

    def addqueuebinding(self,queueid,hostid):
	""" Add a binding for the given queueid and hostid.
	"""
	if (self.getqueuebinding(queueid,hostid)==0):
	    self.db.docommit("insert into persistentqueues (qid,hid) VALUES (%d,%d);" %(queueid,hostid))

    def delqueuebinding(self,queueid,hostid):
	""" Delete the binding (if any) for the given queueid and hostid.
	"""
	id = self.getqueuebinding(queueid,hostid)
	if (id):
	    print("Deleting binding for queue %d on host %d" %(queueid,hostid))
	    self.db.docommit("delete from persistentqueues where pquid=%d and qid=%d and hid=%d;" %(id,queueid,hostid))
	    return 0
	return 1

    def getexchangebinding(self,exchangeid,hostid):
	""" Retrieve the info on the exchange binding for the given exchangeid and hostid.
	Returns 0 if none found.
	"""
	ret=self.db.doquery("select * from persistentexchanges where eid=%s and hid=%s;" %(exchangeid,hostid))
	if (ret==[]):
	    return 0
	return ret[0]['pexid']

    def addexchangebinding(self,exchangeid,hostid):
	""" Add an binding for the given exchangeid and hostid.
	"""
	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.db.docommit("insert into persistentexchanges (eid,hid) VALUES ( %s , %s ) ;" %(exchangeid,hostid))

    def delexchangebinding(self,exchangeid,hostid):
	""" Delete the binding for the given exchangeid and hostid.
	Returns 0 if the binding existed.
	Returns 1 if the binding did not exist.
	"""
	id = self.getexchangebinding(exchangeid,hostid)
	if (id!=0):
	    print("Deleting binding for exchange %d on host %d" %(exchangeid,hostid))
	    self.db.docommit("delete from persistentexchanges where pexid=%d and eid=%d and hid=%d;" %(id,exchangeid,hostid))
            return 0
	return 1

    def getqueueroute(self,queueid,fromid,toid):
	""" Retrieve the queueroute information for the given queueid, fromid and toid.
	fromid and toid are hostid for the sending and the receiving host respectively.
	"""
	ret=self.db.doquery("select * from queueroutes where qid=%s and fromhost=%s and tohost=%s;" %(queueid,fromid,toid))
	if (ret==[]):
	    return 0
	return ret[0]['qrouteid']

    def addqueueroute(self,queueid,fromid,toid,exchangeid):
	""" Add a queue route for the given queueid, fromid, toid and exchangeid.
	"""
	if (self.getqueueroute(queueid,fromid,toid)==0):
	    self.db.docommit("insert into queueroutes (qid,fromhost,tohost,eid) VALUES ( %s , %s , %s, %s );" %(queueid,fromid,toid,exchangeid))

    def delqueueroute(self,queueid,fromid,toid):
	""" Delete the queueroute for the given queueid,fromid and toid
	"""
	id=self.getqueueroute(queueid,fromid,toid)
	if (id!=0):
	    print("Removing queueroute for queue %d from host %d to host %d" %(queueid,fromid,toid))
	    self.db.docommit("delete from queueroutes where qrouteid=%d;" %(queuerouteid))

    def getexchangeroute(self,exchangeid,routingkey,fromid,toid):
	""" Retrieve the exchange route information for the give exchangeid, routingkey, fromid and toid.
	"""
	ret=self.db.doquery("select * from exchangeroutes where eid=%s and fromhost=%s and tohost=%s and routingkey='%s';" %(exchangeid,fromid,toid,routingkey))
	if (ret==[]):
	    return 0
	return ret[0]['erouteid']

    def addexchangeroute(self,exchangeid,routingkey,fromid,toid,dynamic=False):
	""" Add an exchange route for the given exchangeid with routingkey, fromid, toid and dynamic (bool).
	If dynamic is set to True the routing key won't have any effect since teh routing is assumed dynamic.
	"""
	if (getexchangeroute(self,exchangeid,routingkey,fromid,toid)==0):
	    self.db.docommit("insert into exchangeroutes (eid,fromhost,tohost,routingkey,dynamic);" %(exchangeid,fromid,toid,routingkey,dynamic))

    def delexchangeroute(self,exchangeid,routingkey,fromid,toid,dynamic=False):
	""" Delete the exchange route for exchangeid,routingkey,fromid,toid and dynamic(bool).
	"""
	id = getexchangeroute(self,exchangeid,routingkey,fromid,toid)
	if (id!=0):
	    print("Removing exchangeroute for key %s and exchange %s from host %s to host %s" %(routingkey,exchangekey,fromid,toid))
	    self.db.docommit("delete from exchangeroutes where erouteid=%d;" %(id))


    def bindqueuetohost(self,queue,host):
	""" Insert a binding in the database for queue on host.
	Both queue and host will be added to the database if needed.
	"""
	hostid=self.addhost(host)
	queueid=self.addqueue(queue)
	bindid=self.getqueuebinding(queueid,hostid)
	if (bindid==0): # not found
	    self.addqueuebinding(queueid,hostid)
	else:
	    print ("Queue %s already binded with broker %s in database" %(queue,host))

    def bindexchangetohost(self,exchange,host):
	""" Insert a binding in the database for exchange on host.
	Both exchange and host will be added to the database if needed.
	"""
	hostid=self.addhost(host,verbose=False)
	exchangeid=self.addexchange(exchange,verbose=False)
	if (self.getexchangebinding(exchangeid,hostid)==0):
	    self.addexchangebinding(exchangeid,hostid)
	else:
	    print("Exchange %s already binded with broker %s in database" %(exchange,host))


    def setqueueroute(self,queuename,fromname,toname,exchange):
	""" Insert a queue route in the database for queuename,fromname,toname,exchange.
	Queues, hosts and exchanges will be added to the database if needed.
	"""
	fromid = self.addhost(fromname)
	toid = self.addhost(toname)
	queueid = self.addqueue(queuename)
	exchangeid = self.addexchange(exchange)
	self.addqueueroute(queueid,fromid,toid,exchangeid)

    def setexchangeroute(self,exchangename,routingkey,fromname,toname,dynamic=False):
	""" Insert an exchangeroute for exchangename,routingkey,fromname,toname,dynamic (bool).
	Hosts and exchanges will be added to the database if needed.
	"""
	exchangeid = self.addexchange(exchangename)
	fromid = self.addhost(fromname)
	toid = self.addhost(toname)
	self.addexchangeroute(exchangeid,routingkey,fromid,toid,dynamic)

    def renamequeue(self,oldqueuename,newqueuename):
	""" rename the queue oldqueuename to newqueuename.
	This will impact all references to the oldqueuename.
	"""
	self.db.docommit("update queues set queuename='%s' where queuename='%s';" %(newqueuename,oldqueuename))

    def renameexchange(self,oldexchangename,newexchangename):
	""" rename the exchange oldexchangename to newexchangename.
	This will impact all references to the oldexchangename.
	"""
	self.db.docommit("update exchanges set exchangename='%s' where exchangename='%s';" %(newexchangename,oldexchangename))

