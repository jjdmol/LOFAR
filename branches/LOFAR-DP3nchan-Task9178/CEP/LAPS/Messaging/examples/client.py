#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
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

(options, args) = parser.parse_args()

print "options :" ,
print options
print "args :" ,
print args

broker=options.__dict__['broker']
address=options.__dict__['address']
count=int(options.__dict__['count'])

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
        sender.send(Message('Hello world! %d' %(count)))
        count -= 1


except MessagingError,m:
    print m
finally:
    connection.close() 
