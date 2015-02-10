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

import qpid.messaging

# Candidate for a config file
broker="127.0.0.1" 
options="create:always, node: { type: queue, durable: True }"

class BusException(Exception):
    pass

class Session:
    def __init__(self, broker):
        self.connection = qpid.messaging.Connection(broker)
        self.connection.reconnect = True

        try:
            self.connection.open()
            self.session = self.connection.session() 
        except qpid.messaging.MessagingError, m:
            raise BusException(m)

    def __del__(self):
        self.connection.close()

class ToBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)

        try:
            self.sender = self.session.sender("%s;{%s}" % (queue, options))
        except qpid.messaging.MessagingError, m:
            raise BusException(m)

    def __del__(self):
        self.connection.close()

    def sendstr(self,Str):
        msg = qpid.messaging.Message(Str)
        msg.durable = True
        if (reply_to != ""):
           msg.reply_to = reply_to

        self.sendmsg(msg)

    def sendmsg(self,msg):
        self.sender.send(msg)

class FromBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)

        self.add_queue(queue, options)

    def add_queue(self, queue, options=options):
        try:
            receiver = self.session.receiver("%s;{%s}" % (queue, options))

            # Need capacity >=1 for 'self.session.next_receiver' to function across multiple queues
            receiver.capacity = 1 #32
        except qpid.messaging.MessagingError, m:
            raise BusException(m)

    def getmsg(self, timeout=None):
        msg = None

        try:
            receiver = self.session.next_receiver(timeout)
            if receiver != None: msg = self.receiver.get()
        except qpid.messaging.exceptions.Empty, e:
            return None

        return msg

    def ack(self, msg):
        self.session.acknowledge(msg)

