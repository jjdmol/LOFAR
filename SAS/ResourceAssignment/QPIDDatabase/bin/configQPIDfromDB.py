#!/usr/bin/python

# setup PYTHONPATH while testing: export PYTHONPATH=../lib:$PYTHONPATH

from QPIDDB import qpidinfra


def qpidconfig_add_queue(host,queue):
    print ("qpid-config -b %s add queue %s --durable" %(host,queue))

def qpidconfig_add_topic(host,exchange):
    print ("qpid-config -b %s add exchange topic %s --durable" %(host,exchange))

def qpidroute_add(fromhost,tohost,exchange,routingkey):
    print ("qpid-route -d route add %s %s %s \'%s\' " %(tohost,fromhost,exchange,routingkey))

def qpidQroute_add(fromhost,tohost,queue):
    print ("qpid-route -d queue add %s %s %s amq.direct" %(tohost,fromhost,queue))

QPIDinfra = qpidinfra()
QPIDinfra.perqueue(qpidconfig_add_queue)
QPIDinfra.perexchange(qpidconfig_add_topic)
QPIDinfra.perfederationexchange(qpidroute_add)
QPIDinfra.perfederationqueue(qpidQroute_add)





