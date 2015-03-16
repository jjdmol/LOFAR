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
broker="127.0.0.1" 
address=""
options=""
 # "create:always, node: { type: queue, durable: True}"
 
class ToBus:
    def __init__(self, address, broker=broker, options=options):
        self.connection = Connection(broker)
        self.connection.reconnect = True
 
        try:
            self.connection.open()
            self.session = self.connection.session() 
            #self.receiver = self.session.receiver("%s;{%s}" %(address,options))
	    #self.receiver.capacity = 32
            self.sender = self.session.sender(address)
 
        except MessagingError,m:
            print " OMG!!"
            print m
 
    def sendstr(self,messagetxt,subject="",reply_to=""):
        msg = Message(messgaetxt)
        msg.subject=subject
        msg.durable=True
        if (reply_to != ""):
           msg.reply_to=reply_to
        self.sender.send(msg)

    def send(self,msg):
        if msg is None:
          print( " attempt to send None object!")
        else:
          msg.durable=True # force durable messages
          self.sender.send(msg)
        
 
class FromBus:
    def __init__(self, address=address, broker=broker, options=options):
        self.connection = Connection(broker)
        self.connection.reconnect = True
 
        try:
            self.connection.open()
            self.session = self.connection.session() 
            self.receiver = self.session.receiver("%s;{%s}" %(address,options))
            #self.address = self.receiver.address
	    self.receiver.capacity = 1
 
        except MessagingError,m:
            print " OMG!!"
            print m
 
    def get(self,timeout=None):
	try:
        	self.msg= self.receiver.fetch(timeout)
	except exceptions.Empty, e:
                self.msg = None
        return self.msg
        #.content, self.msg.subject
 
    def ack(self,msg):
        if msg is None:
          print(" Ack on None object!")
        else:
          self.session.acknowledge(msg)
 
class BusReply:
    def __init__(self, address="Reply", broker=broker, options=options):
        self.address = "%s%s ; { %s }" %(str(uuid4()),address,options)
        self.connection = Connection(broker)
        self.connection.reconnect = True
 
        try:
            self.connection.open()
            self.session = self.connection.session()
            self.receiver = self.session.receiver(self.address)
            self.address = self.receiver.source
            #self.receiver.capacity = 32
 
        except MessagingError,m:
            print " OMG!!"
            print m
 
    def get(self,timeout=None):
        try:
                self.msg= self.receiver.fetch(timeout)
        except exceptions.Empty, e:
                return "None" , "None"
        return self.msg.content, self.msg.subject
 
    def ack(self):
          self.session.acknowledge()
 
class MultiReceiveBus:
    def __init__(self, handler, address=address, broker=broker, options=options):
        self.connection = Connection(broker)
        self.connection.reconnect = True
        self.handlers={}
        try:
            self.connection.open()
            self.session = self.connection.session()
            receiver = self.session.receiver("%s;{%s}" %(address,options))
            receiver.capacity = 1 #32
            self.handlers[receiver] = handler
 
        except MessagingError,m:
            print " OMG!!"
            print m
 
    def add(self,handler,address,options=options):
        try:
            receiver=self.session.receiver("%s;{%s}" %(address,options))
            receiver.capacity = 1 #32
            self.handlers[receiver]=handler
        except MessagingError,m:
            print "Error adding receiver"
            print m
 
    def HandleMessages(self):
        while True:
                print "waiting for messages"
                receiver = self.session.next_receiver()
                print "got incoming message"
                handler = self.handlers[receiver]
                msg = receiver.fetch()
                handler(self,msg.content,msg.subject)
 
    def ack(self,msg):
          self.session.acknowledge(msg)

