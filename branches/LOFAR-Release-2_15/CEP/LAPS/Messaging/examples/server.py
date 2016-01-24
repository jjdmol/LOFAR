#!/usr/bin/python
# Copyright (C) 2012-2014  ASTRON (Netherlands Institute for Radio Astronomy)
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
    while message:
        print "received :",
        print message.content
        session.acknowledge()
    message = receiver.fetch()

except MessagingError,m:
    print m
finally:
    connection.close() 
