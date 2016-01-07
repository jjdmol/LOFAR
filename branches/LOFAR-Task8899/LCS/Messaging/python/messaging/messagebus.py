# messagebus.py: Provide an easy way exchange messages on the message bus.
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: messagebus.py 1580 2015-09-30 14:18:57Z loose $

"""
Provide an easy way exchange messages on the message bus.
"""

from lofar.messaging.exceptions import MessageBusError, MessageFactoryError
from lofar.messaging.messages import to_qpid_message, MESSAGE_FACTORY
from lofar.common.util import raise_exception

import qpid.messaging
import logging
import sys

logger = logging.getLogger(__name__)

# Default settings for often used parameters.
DEFAULT_ADDRESS_OPTIONS = {'create': 'never'}
DEFAULT_BROKER = "localhost:5672"
DEFAULT_BROKER_OPTIONS = {'reconnect': True}
DEFAULT_RECEIVER_CAPACITY = 1
DEFAULT_TIMEOUT = 5


class FromBus(object):
    """
    This class provides an easy way to fetch messages from the message bus.
    Note that most methods require that a FromBus object is used *inside* a
    context. When entering the context, the connection with the broker is
    opened, and a session and a receiver are created. When exiting the context,
    the connection to the broker is closed; as a side-effect the receiver(s)
    and session are destroyed.

    note:: The rationale behind using a context is that this is (unfortunately)
    the *only* way that we can guarantee proper resource management. If there
    were a __deinit__() as counterpart to __init__(), we could have used that.
    We cannot use __del__(), because it is not the counterpart of __init__(),
    but that of __new__().
    """

    def __init__(self, address, options=None, broker=None):
        """
        Initializer.
        :param address: valid Qpid address
        :param options: valid Qpid address options, e.g. {'create': 'never'}
        :param broker: valid Qpid broker URL, e.g. "localhost:5672"
        """
        self.address = address
        self.options = options if options else DEFAULT_ADDRESS_OPTIONS
        self.broker = broker if broker else DEFAULT_BROKER

        self.connection = qpid.messaging.Connection(self.broker,
                                                    **DEFAULT_BROKER_OPTIONS)
        self.session = None
        self.opened=0

    def open(self):
        """
        The following actions will be performed when entering a context:
        * connect to the broker
        * create a session
        * add a receiver
        The connection to the broker will be closed if any of these failed.
        :raise MessageBusError: if any of the above actions failed.
        :return: self
        """
        if (self.opened==0):
          try:
            self.connection.open()
            logger.info("[FromBus] Connected to broker: %s", self.broker)
            self.session = self.connection.session()
            logger.debug("[FromBus] Created session: %s", self.session.name)
            self.add_queue(self.address, self.options)
          except qpid.messaging.MessagingError:
            self.__exit__(*sys.exc_info())
            raise_exception(MessageBusError, "[FromBus] Initialization failed")
          except MessageBusError:
            self.__exit__(*sys.exc_info())
            raise
        self.opened+=1
      

    def __enter__(self):
        self.open()
        return self

    def close(self):
        """
        The following actions will be performed:
        * close the connection to the broker
        * set session to None
        :param exc_type: type of exception thrown in context
        :param exc_val: value of exception thrown in context
        :param exc_tb: traceback of exception thrown in context
        """
        if (self.opened==1):
          try:
            if self.connection.opened():
                self.connection.close(DEFAULT_TIMEOUT)
          except qpid.messaging.exceptions.Timeout:
            raise_exception(MessageBusError,
                            "[FromBus] Failed to disconnect from broker: %s" %
                            self.broker)
          finally:
            self.session = None
          logger.info("[FromBus] Disconnected from broker: %s", self.broker)
        self.opened-=1
       

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _check_session(self):
        """
        Check if there's an active session.
        :raise MessageBusError: if there's no active session
        """
        if self.session is None:
            raise MessageBusError(
                "[FromBus] No active session (broker: %s)" % self.broker)

    def add_queue(self, address, options=None):
        """
        Add a queue that you want to receive messages from.
        :param address: valid Qpid address
        :param options: dict containing valid Qpid address options
        """
        self._check_session()
        options = options if options else self.options
        options.setdefault("capacity", DEFAULT_RECEIVER_CAPACITY)
        what = "receiver for source: %s (broker: %s, session: %s)" % \
               (address, self.broker, self.session.name)
        try:
            self.session.receiver(address, **options)
        except qpid.messaging.MessagingError:
            raise_exception(MessageBusError,
                            "[FromBus] Failed to create %s" % what)
        logger.info("[FromBus] Created %s", what)

    def receive(self, timeout=DEFAULT_TIMEOUT):
        """
        Receive the next message from any of the queues we're listening on.
        :param timeout: maximum time in seconds to wait for a message.
        :return: received message, None if timeout occurred.
        """
        self._check_session()
        logger.debug("[FromBus] Waiting %s seconds for next message", timeout)
        try:
            recv = self.session.next_receiver(timeout)
            msg = recv.fetch()
        except qpid.messaging.exceptions.Empty:
            logger.debug(
                "[FromBus] No message received within %s seconds", timeout)
            return None
        except qpid.messaging.MessagingError:
            raise_exception(MessageBusError,
                            "[FromBus] Failed to fetch message from queue: "
                            "%s" % self.address) 
        logger.info("[FromBus] Message received on queue: %s subject: %s" % (self.address, msg.subject))
        logger.debug("[FromBus] %s" % msg)
        try:
            amsg = MESSAGE_FACTORY.create(msg)
        except MessageFactoryError:
            self.reject(msg)
            raise_exception(MessageBusError, "[FromBus] Message rejected")
        # self.ack(msg)
        return amsg

    def ack(self, msg):
        """
         Acknowledge a message. This will inform Qpid that the message can
        safely be removed from the queue.
        :param msg: message to be acknowledged
        """
        self._check_session()
        qmsg = to_qpid_message(msg)
        self.session.acknowledge(qmsg)
        logger.debug("[FromBus] acknowledged message: %s", qmsg)

    def nack(self, msg):
        """
        Do not acknowledge a message. This will inform Qpid that the message
        has to be redelivered. You cannot nack a message that has already
        been acknowledged.
        :param msg: message to not be acknowledged

        .. attention::
            We should call qpid.messaging.Session.release() here,but that is
            not available in Qpid-Python 0.32. We therefore use
            qpid.messaging.Session.acknowledge() instead.
        """
        logger.warning("[FromBus] nack() is not supported, using ack() instead")
        self.ack(msg)

    def reject(self, msg):
        """
        Reject a message. This will inform Qpid that the message should not be
        redelivered. You cannot reject a message that has already been
        acknowledged.
        :param msg: message to be rejected

        .. attention::
            We should call qpid.messaging.Session.reject() here, but that is
            not available in Qpid-Python 0.32. We therefore use
            qpid.messaging.Session.acknowledge() instead.
        """
        logger.warning(
            "[FromBus] reject() is not supported, using ack() instead")
        self.ack(msg)


class ToBus(object):
    """
    This class provides an easy way to post messages onto the message bus.

    Note that most methods require that a ToBus object is used *inside* a
    context. When entering the context, the connection with the broker is
    opened, and a session and a sender are created. When exiting the context,
    the connection to the broker is closed; as a side-effect the sender and
    session are destroyed.

    note:: The rationale behind using a context is that this is (unfortunately)
    the *only* way that we can guarantee proper resource management. If there
    were a __deinit__() as counterpart to __init__(), we could have used that.
    We cannot use __del__(), because it is not the counterpart of __init__(),
    but that of __new__().
    """

    def __init__(self, address, options=None, broker=None):
        """
        Initializer.
        :param address: valid Qpid address
        :param options: valid Qpid address options, e.g. {'create': 'never'}
        :param broker: valid Qpid broker URL, e.g. "localhost:5672"
        """
        self.address = address
        self.options = options if options else DEFAULT_ADDRESS_OPTIONS
        self.broker = broker if broker else DEFAULT_BROKER

        self.connection = qpid.messaging.Connection(self.broker,
                                                    **DEFAULT_BROKER_OPTIONS)
        self.session = None
        self.opened = 0

    def open(self):
        if (self.opened==0):
           try:
             self.connection.open()
             logger.info("[ToBus] Connected to broker: %s", self.broker)
             self.session = self.connection.session()
             logger.debug("[ToBus] Created session: %s", self.session.name)
             self._add_queue(self.address, self.options)
           except qpid.messaging.MessagingError:
             self.__exit__(*sys.exc_info())
             raise_exception(MessageBusError, "[ToBus] Initialization failed")
           except MessageBusError:
             self.__exit__(*sys.exc_info())
             raise
        self.opened+=1


    def __enter__(self):
        """
        The following actions will be performed when entering a context:
        * connect to the broker
        * create a session
        * add a sender
        The connection to the broker will be closed if any of these failed.
        :raise MessageBusError: if any of the above actions failed.
        :return: self
        """
        """
        try:
            self.connection.open()
            logger.info("[ToBus] Connected to broker: %s", self.broker)
            self.session = self.connection.session()
            logger.debug("[ToBus] Created session: %s", self.session.name)
            self._add_queue(self.address, self.options)
        except qpid.messaging.MessagingError:
            self.__exit__(*sys.exc_info())
            raise_exception(MessageBusError, "[ToBus] Initialization failed")
        except MessageBusError:
            self.__exit__(*sys.exc_info())
            raise
        """
        self.open()
        return self

    def close(self):
        if (self.opened==1):
           try:
             if self.connection.opened():
                self.connection.close(DEFAULT_TIMEOUT)
           except qpid.messaging.exceptions.Timeout:
              raise_exception(MessageBusError,
                            "[ToBus] Failed to disconnect from broker %s" %
                            self.broker)
           finally:
              self.session = None
        self.opened-=1

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        The following actions will be performed:
        * close the connection to the broker
        * set `session` and `sender` to None
        :param exc_type: type of exception thrown in context
        :param exc_val: value of exception thrown in context
        :param exc_tb: traceback of exception thrown in context
        :raise MessageBusError: if disconnect from broker fails
        """
        try:
            if self.connection.opened():
                self.connection.close(DEFAULT_TIMEOUT)
        except qpid.messaging.exceptions.Timeout:
            raise_exception(MessageBusError,
                            "[ToBus] Failed to disconnect from broker %s" %
                            self.broker)
        finally:
            self.session = None
        logger.info("[ToBus] Disconnected from broker: %s", self.broker)

    def _check_session(self):
        """
        Check if there's an active session.
        :raise MessageBusError: if there's no active session
        """
        if self.session is None:
            raise MessageBusError("[ToBus] No active session (broker: %s)" %
                                  self.broker)

    def _get_sender(self):
        """
        Get the sender associated with the current session.
        :raise MessageBusError: if there's no active session, or if there's not
        exactly one sender
        :return: sender object
        """
        self._check_session()
        nr_senders = len(self.session.senders)
        if nr_senders == 1:
            return self.session.senders[0]
        else:
            msg = "No senders" if nr_senders == 0 else "More than one sender"
            raise MessageBusError("[ToBus] %s (broker: %s, session %s)" %
                                  (msg, self.broker, self.session))

    def _add_queue(self, address, options):
        """
        Add a queue that you want to sends messages to.
        :param address: valid Qpid address
        :param options: dict containing valid Qpid address options
        :raise MessageBusError: if sender could not be created
        """
        self._check_session()
        what = "sender for target: %s (broker: %s, session: %s)" % (
            address, self.broker, self.session.name)
        try:
            self.session.sender(address, **options)
        except qpid.messaging.MessagingError:
            raise_exception(MessageBusError,
                            "[ToBus] Failed to create %s" % what)
        logger.info("[ToBus] Created %s", what)

    def send(self, message, timeout=DEFAULT_TIMEOUT):
        """
        Send a message to the exchange (target) we're connected to.
        :param message: message to be sent
        :param timeout: maximum time in seconds to wait for send action
        :return:
        """
        sender = self._get_sender()
        qmsg = to_qpid_message(message)
        logger.debug("[ToBus] Sending message to queue: %s (%s)",
                     sender.target, qmsg)
        try:
            sender.send(qmsg, timeout=timeout)
        except qpid.messaging.MessagingError:
            raise_exception(MessageBusError,
                            "[ToBus] Failed to send message to queue: %s" %
                            sender.target)
        logger.info("[ToBus] Message sent to queue: %s subject: %s" % (sender.target, message.subject))


__all__ = ["FromBus", "ToBus"]
