#!/usr/bin/python

import sys
from qpid.messaging import *
from optparse import OptionParser


parser = OptionParser()
parser.add_option("-a", "--address", dest="address", default="testqueue;{create:always}",
                  help="address (name of queue or topic)", metavar="FILE")
parser.add_option("-b", "--broker", dest="broker", default="localhost",
                  help="broker hostname")
parser.add_option("-c", "--count", dest="count", default=1,
                  help="number of messages to be sent")

(options, args) = parser.parse_args()

print "options :" ,
print options
print "args :" ,
print args

broker=options.__dict__['broker']
address=options.__dict__['address']
count=int(options.__dict__['count'])


print " setup connection "
#if len(sys.argv)<3 else sys.argv[2]

connection = Connection(broker)

try:
  connection.open()
  print " opened "
  session = connection.session() 
  print " session "
  receiver = session.receiver(address)
  message = receiver.fetch()
  while (message and count):
  	print "received :",
  	print message.content
  	session.acknowledge()
	if count>0:
		count = count - 1
	if count>0:
		message = receiver.fetch()

except MessagingError,m:
  print m
finally:
  connection.close() 
