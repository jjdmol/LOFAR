#!/usr/bin/python
from optparse import OptionParser
import sys, time
from qpid.messaging import *
parser = OptionParser()
parser.add_option("-a", "--address", dest="address", default="testqueue",
                  help="address (name of queue or topic)", metavar="FILE")
parser.add_option("-b", "--broker", dest="broker", default="localhost",
                  help="broker hostname")
parser.add_option("-c", "--count", dest="count", default=1,
                  help="number of messages to be sent")
parser.add_option("-m", "--message", dest="message", default="void",
                  help="number of messages to be sent")

(options, args) = parser.parse_args()

print "options :" ,
print options
print "args :" ,
print args

broker=options.__dict__['broker']
address=options.__dict__['address']
count=int(options.__dict__['count'])
message=options.__dict__['message']

print " setup connection with ",
print broker
print " on queue or topic :",
print address
print " count of messages :",
print count

connection = Connection(broker)

try:
  connection.open()
  print " opened "
  session = connection.session() 
  print " session "
  sender = session.sender(address)
  print " sending message "
  while count >0:
	#time.sleep(2)
	print 'send message: Hello world! %d' %(count)
	if message=="void":
		sender.send(Message('Hello world! %d' %(count)))
	else:
		sender.send(Message(message))
	count -= 1


except MessagingError,m:
  print m
finally:
  connection.close() 
