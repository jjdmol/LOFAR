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

try:
  import qpid.messaging
  enabled = True
except ImportError:
  enabled = False

import os
import lofar.messagebus.message as message

# Candidate for a config file
broker="127.0.0.1" 
options="create:never"

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

    def address(self, queue, options):
        return "%s%s; {%s}" % (self._queue_prefix(), queue, options)

    def _queue_prefix(self):
      return os.environ.get("QUEUE_PREFIX", "")

class ToBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)

        try:
            self.sender = self.session.sender(self.address(queue, options))
        except qpid.messaging.MessagingError, m:
            raise BusException(m)

    def __del__(self):
        self.connection.close()

    def send(self, msg, verbatim=False):
        # (Re)generate the QPID message body
        if not verbatim:
            msg.generate_message()

        try:
            self.sender.send(msg.qpidMsg)
        except qpid.messaging.SessionError, m:
            raise BusException(m)

class FromBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)

        self.add_queue(queue, options)

    def add_queue(self, queue, options=options):
        try:
            receiver = self.session.receiver(self.address(queue, options))

            # Need capacity >=1 for 'self.session.next_receiver' to function across multiple queues
            receiver.capacity = 1 #32
        except qpid.messaging.MessagingError, m:
            raise BusException(m)

    def get(self, timeout=None):
        msg = None

        try:
            receiver = self.session.next_receiver(timeout)
            if receiver != None: msg = receiver.fetch() # receiver.get() is better, but requires qpid 0.31+
        except qpid.messaging.exceptions.Empty, e:
            return None

        if msg is None:
          return None
        else:
          return message.Message(qpidMsg=msg)

    def ack(self, msg):
        self.session.acknowledge(msg.qpidMsg)

