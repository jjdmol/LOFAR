#!/usr/bin/python

# setup PYTHONPATH while testing: export PYTHONPATH=../lib:$PYTHONPATH

from QPIDDB import qpidinfra

S_INDB = 1
S_ONQPID = 2



class host:
    def __init__(self):
	self.queues={}
	self.exchanges={}
	self.queueroutes={}
	self.exchangeroutes={}

    def __repr__(self):
	return "<host: queues: '%s' exchanges '%s' queueroutes '%s' exchangeroutes '%s' >" \
		%(self.queues.__repr__(), self.exchanges.__repr__(), self.queueroutes.__repr__(),\
		self.exchangeroutes.__repr__() )

    def __str__(self):
	return "HOST: queues=%s exchanges=%s queueroutes=%s exchangeroutes=%s\n" \
		%(self.queues, self.exchanges, self.queueroutes, self.exchangeroutes)
    
    def tag(self,item,index,state):
	item[index]=item.get(index,0) | state

    def untag(self,item,index,state):
	self.queue[index]=item.get(index,0) & ~state
    
    def tagqueue(self,queue,state):
	self.tag(self.queues,queue,state)

    def untagqueue(self,queue,state):
	self.untag(self.queues,queue,state)

    def tagexchange(self,exchange,state):
	self.tag(self.exchanges,exchange,state)

    def untagexchange(self,exchange,state):
	self.untag(self.exchanges,exchange,state)

    def tagqueueroute(self,tohost,queue,state):
	index=tohost+':'+queue
	self.tag(self.queueroutes,index,state)

    def untagqueueroute(self,tohost,queue,state):
	index=tohost+':'+queue
	self.untag(self.queueroutes,index,state)

    def tagexchangeroute(self,tohost,exchange,key,state):
	index=tohost+':'+exchange+':'+key
	self.tag(self.exchangeroutes,index,state)

    def untagexchangeroute(self,tohost,exchange,key,state):
	index=tohost+':'+exchange+':'+key
	self.untag(self.exchangeroutes,index,state)

Hosts={}

DEFINED=1
SEEN=2

   
def Host(hostname):
    if hostname not in Hosts:
	Hosts[hostname]=host()
    return Hosts[hostname]



def qpidconfig_add_queue(settings):
    Host(settings['hostname']).tagqueue(settings['queuename'],DEFINED)

def qpidconfig_add_topic(settings):
    Host(settings['hostname']).tagexchange(settings['exchangename'],DEFINED)

def qpidroute_add(settings):
    Host(settings['fromhost']).tagexchangeroute(settings['tohost'],settings['exchangename'],settings['routingkey'],DEFINED)

def qpidQroute_add(settings):
    Host(settings['fromhost']).tagqueueroute(settings['tohost'],settings['queuename'],DEFINED)


QPIDinfra = qpidinfra()


QPIDinfra.perqueue(qpidconfig_add_queue)
QPIDinfra.perexchange(qpidconfig_add_topic)
QPIDinfra.perfederationexchange(qpidroute_add)
QPIDinfra.perfederationqueue(qpidQroute_add)

print Hosts 
print " - "
print "Done."
print " ------------------------------------------"
print "QPIDinfra config fetched from DB"
print "Next step: retrieve config from brokers. TBD."
print " ------------------------------------------" 

