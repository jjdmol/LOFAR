#!/usr/bin/python

from lofar.qpidinfrastructure.QPIDDB import qpidinfra
from lofar.common import dbcredentials

def qpidconfig_add_queue(settings):
    print ("qpid-config -b %s add queue %s --durable" %(settings['hostname'],settings['queuename']))

def qpidconfig_add_topic(settings):
    print ("qpid-config -b %s add exchange topic %s --durable" %(settings['hostname'],settings['exchangename']))

def qpidroute_add(settings):
    print ("qpid-route -d route del %s %s %s \'%s\' " %(settings['tohost'],settings['fromhost'],settings['exchangename'],settings['routingkey']))
    print ("qpid-route -d route add %s %s %s \'%s\' " %(settings['tohost'],settings['fromhost'],settings['exchangename'],settings['routingkey']))

def qpidQroute_add(settings):
    print ("qpid-route -d queue del %s %s '%s' '%s'" %(settings['tohost'],settings['fromhost'],settings['exchangename'],settings['queuename']))
    print ("qpid-route -d queue add %s %s '%s' '%s'" %(settings['tohost'],settings['fromhost'],settings['exchangename'],settings['queuename']))

dbcreds = dbcredentials.DBCredentials().get("qpidinfra")
QPIDinfra = qpidinfra(dbcreds)
QPIDinfra.perqueue(qpidconfig_add_queue)
QPIDinfra.perexchange(qpidconfig_add_topic)
QPIDinfra.perfederationexchange(qpidroute_add)
QPIDinfra.perfederationqueue(qpidQroute_add)

