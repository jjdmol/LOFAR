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
# id.. TDB
from qpid.messaging import *

# Candidate for a config file
broker="localhost" 
options="create:always, node: { type: queue, durable: True}"

class ToBus():
    def __init__(self, address, options=options, broker=broker):
        self.connection = Connection(broker)
        self.connection.reconnect = True

        try:
            self.connection.open()
            self.session = self.connection.session() 
            self.sender = self.session.sender(address)

        except MessagingError,m:
            print " OMG!!"
            print m

    def sendstr(self,Str):
        msg = Message(Str)
        msg.durable=True
        if (reply_to != ""):
           msg.reply_to=reply_to
        self.sender.send(msg)

    def sendmsg(self,msg):
	self.sender.send(msg)

class FromBus():
    def __init__(self, address, options=options, broker=broker):
        self.connection = Connection(broker)
        self.connection.reconnect = True
        
        try:
            self.connection.open()
            self.session = self.connection.session()
            receiver = self.session.receiver("%s;{%s}" %(address,options))
            receiver.capacity = 1 #32
            
        except MessagingError,m:
            print " OMG!!"
            print m

    def add(self,address,options=options):
        try:
            receiver=self.session.receiver("%s;{%s}" %(address,options))
            receiver.capacity = 1 #32
        except MessagingError,m:
            print "Error adding receiver"
            print m

    def getmsg(self,timeout=None):
        msg=None
	try:
                receiver = self.session.next_receiver(timeout)
        	if receiver != None: msg = self.receiver.get()
	except exceptions.Empty, e:
		return "None" , "None"
        return msg

    def ack(self,msg):
          self.session.acknowledge(msg)

