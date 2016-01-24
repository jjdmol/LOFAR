#!/usr/bin/env python
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

try:
  import qpid.messaging as messaging
  MESSAGING_ENABLED = True
except ImportError:
  import noqpidfallback as messaging
  MESSAGING_ENABLED = False

import os
import logging
import lofar.messagebus.message as message
import atexit

# Candidate for a config file
broker="127.0.0.1" 
options="create:never"

logger=logging.getLogger("MessageBus")

class BusException(Exception):
    pass

class Session:
    def __init__(self, broker):
        self.closed = False
        self.connection = messaging.Connection(broker)
        self.connection.reconnect = True

        logger.info("[Bus] Connecting to broker %s", broker)
        try:
            self.connection.open()
            logger.info("[Bus] Connected to broker %s", broker)
            self.session = self.connection.session() 
        except messaging.MessagingError, m:
            raise BusException(m)

        # NOTE: We cannot use:
        #  __del__: its broken (does not always get called, destruction order is unpredictable)
        #  with:    not supported in python 2.4, does not work well on arrays of objects
        #  weakref: dpes not guarantee to be called (depends on gc)
        #
        # Note that this atexit call will prevent self from being destructed until the end of the program,
        # since a reference will be retained
        atexit.register(self.close)

    def close(self):
        if self.closed:
            return

        self.closed = True

        # NOTE: session.close() freezes under certain error conditions,
        # f.e. opening a receiver on a non-existing queue.
        # This seems to happen whenever a Python exception was thrown
        # by the qpid wrapper.
        #
        # This especially happens if we would put this code in __del__.
        # Note that we cannot use __enter__ and __exit__ either due to
        # ccu001/ccu099 still carrying python 2.4.
        #
        # See https://issues.apache.org/jira/browse/QPID-6402
        #
        # We set a timeout to prevent freezing, which obviously leads
        # to data loss if the stall was legit.
        try:
            self.connection.close(5.0)
        except messaging.exceptions.Timeout, t:
            logger.error("[Bus] Could not close connection: %s", t)

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.close()
        return False

    def address(self, queue, options):
        return "%s%s; {%s}" % (self._queue_prefix(), queue, options)

    def _queue_prefix(self):
      return os.environ.get("QUEUE_PREFIX", "")

class ToBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)
        self.queue = queue

        try:
            self.sender = self.session.sender(self.address(queue, options))
        except messaging.MessagingError, m:
            raise BusException(m)

    def send(self, msg):
        try:
            logger.info("[ToBus] Sending message to queue %s", self.queue)

            try:
              # Send Message or MessageContent object
              self.sender.send(msg.qpidMsg())
            except AttributeError:
              # Send string or messaging.Message object
              self.sender.send(msg)

            logger.info("[ToBus] Message sent to queue %s", self.queue)
        except messaging.SessionError, m:
            raise BusException(m)

class FromBus(Session):
    def __init__(self, queue, options=options, broker=broker):
        Session.__init__(self, broker)

        self.add_queue(queue, options)

    def add_queue(self, queue, options=options):
        try:
            receiver = self.session.receiver(self.address(queue, options))
        except messaging.MessagingError, m:
            raise BusException(m)

        # Need capacity >=1 for 'self.session.next_receiver' to function across multiple queues
        receiver.capacity = 1

    def get(self, timeout=None):
        msg = None

        logger.info("[FromBus] Waiting for message")
        try:
            receiver = self.session.next_receiver(timeout)
            if receiver != None:
                logger.info("[FromBus] Message available on queue %s", receiver.source)
                msg = receiver.fetch() # receiver.get() is better, but requires qpid 0.31+
                if msg is None:
                    logger.error("[FromBus] Could not retrieve available message on queue %s", receiver.source)
                else:
                    logger.info("[FromBus] Message received on queue %s", receiver.source)
        except messaging.exceptions.Empty, e:
            return None

        if msg is None:
          return None
        else:
          return message.Message(qpidMsg=msg)

    def ack(self, msg):
        self.session.acknowledge(msg.qpidMsg())
        logging.info("[FromBus] Message ACK'ed");

