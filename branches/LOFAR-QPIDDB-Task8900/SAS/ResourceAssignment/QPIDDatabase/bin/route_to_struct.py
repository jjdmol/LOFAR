#!/usr/bin/python

import sys
from QPIDDB import qpidinfra




todb=qpidinfra()

tosearch = sys.stdin.readlines()

numlines = len(tosearch)

#fqdn=['lhn001.cep2.lofar',
#      'ccu001.control.lofar',
#      'sas001.control.lofar',
#      'cbm001.control.lofar',
#      'cbm001.control.lofar',
#      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',      'cbm001.control.lofar',
      

def to_hostname(s):
    leafname=s.split(':')[0].split('.')[0].lower()
    fqdn="%s.control.lofar" %(leafname)
    #print leafname[0:5]
    if (leafname[0:5]=='locus'):
	fqdn="%s.cep2.lofar" %(leafname)
    if (leafname=='lhn001'):
	fqdn="%s.cep2.lofar" %(leafname)

    return fqdn

def to_exchangename(s):
    exchangename=s.split('=')[1].split(')')[0]
    print(" found exchangename '%s'" %(exchangename))
    return exchangename


print (" Num lines %d " %(numlines))

offset=0

while (tosearch[offset] != 'Static Routes:\n'):
    #print ( "'%s'" %( tosearch[offset]))
    offset += 1
    if (offset==numlines):
	print "notfound"
	break

if (offset!=numlines):
    for offset in range(offset,numlines):
	s = tosearch[offset].split(' ')
	if ( len(s) ==5 ): # valid description
	    hosta=to_hostname(s[2])
	    exchangename=to_exchangename(s[2])
	    if (exchangename == ''):
		exchangename='lofar.default.bus'
	    queuename=s[4].split('=')[1].split(')')[0]
	    hostb=to_hostname(s[4]) #.split(':')[0].split('.')
	    if (s[3]=='<='):
		todb.bindexchangetohost(exchangename,hosta)
		todb.bindexchangetohost(exchangename,hostb)
		todb.bindqueuetohost(queuename,hosta)
		todb.bindqueuetohost(queuename,hostb)
		todb.setqueueroute(queuename,hostb,hosta,exchangename)
		print ("# queue %s from %s to %s" %(queuename,hostb,hosta))
	    if (s[3]=='=>'):
		todb.bindqueuetohost(queuename,hosta)
		todb.bindqueuetohost(queuename,hostb)
		todb.setqueueroute(queuename,hosta,hostb)
		print ("# queue %s from %s to %s" %(queuename,hosta,hostb))


