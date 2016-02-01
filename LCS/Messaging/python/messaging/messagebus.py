#!/usr/bin/env python

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
import uuid
import threading

logger = logging.getLogger(__name__)

# Default settings for often used parameters.
DEFAULT_ADDRESS_OPTIONS = {'create': 'never'}
DEFAULT_BROKER = "localhost:5672"
DEFAULT_BROKER_OPTIONS = {}
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

    def isConnected(self):
        return self.opened > 0

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
                            "[FromBus] Failed to fetch message from: "
                            "%s" % self.address) 
        logger.info("[FromBus] Message received on: %s%s" % (self.address, " subject: " + msg.subject if msg.subject else ""))
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
        logger.debug("[ToBus] Sending message to: %s (%s)",
                     sender.target, qmsg)
        try:
            sender.send(qmsg, timeout=timeout)
        except qpid.messaging.MessagingError:
            raise_exception(MessageBusError,
                            "[ToBus] Failed to send message to: %s" %
                            sender.target)
        logger.info("[ToBus] Message sent to: %s%s" % (sender.target, ' subject: '+message.subject if message.subject else ''))


class AbstractBusListener(object):
    """
    AbstractBusListener class for handling messages which are received on a message bus.
    Typical usage is to derive from this class and implement the handle_message method with concrete logic.
    """

    def __init__(self, address, broker=None, **kwargs):
        """
        Initialize AbstractBusListener object with address (str)
        :param address: valid Qpid address
        additional parameters in kwargs:
            options=   <dict>  Dictionary of options passed to QPID
            exclusive= <bool>  Create an exclusive binding so no other listeners can consume duplicate messages (default: False)
            numthreads= <int>  Number of parallel threads processing messages (default: 1)
            verbose=   <bool>  Output extra logging over stdout (default: False)
        """
        self.address          = address
        self.broker           = broker
        self.running          = [False]
        self._listening       = False
        self.exclusive        = kwargs.pop("exclusive", False)
        self._numthreads      = kwargs.pop("numthreads", 1)
        self.verbose          = kwargs.pop("verbose", False)
        self.frombus_options  = {"capacity": self._numthreads*20}
        options               = kwargs.pop("options", None)

        if len(kwargs):
            raise AttributeError("Unexpected argument passed to AbstractBusListener constructor: %s", kwargs)

        # Set appropriate flags for exclusive binding
        if self.exclusive == True:
            binding_key = address.split('/')[-1]
            self.frombus_options["link"] = '''{name:"%s",
                                              x-bindings:[{key: %s,
                                                          arguments: {"qpid.exclusive-binding":True}}]}''' % (str(uuid.uuid4()), binding_key)

        # only add options if it is given as a dictionary
        if isinstance(options,dict):
            for key,val in options.iteritems():
                self.frombus_options[key] = val

    def _debug(self, txt):
        """
        Internal use only.
        """
        if self.verbose == True:
            logger.debug("[%s: %s]", self.__class__.__name__, txt)

    def isListening(self):
        return self._listening

    def start_listening(self, numthreads=None):
        """
        Start the background threads and process incoming messages.
        """
        if self._listening == True:
            return

        self._bus_listener  = FromBus(self.address, broker=self.broker, options=self.frombus_options)
        self._bus_listener.open()

        if numthreads != None:
            self._numthreads = numthreads

        # use a list to ensure that threads always 'see' changes in the running state.
        self.running = [ True ]
        self._threads = {}
        for i in range(self._numthreads):
            thread = threading.Thread(target=self._loop)
            self._threads[thread] = self._create_thread_args(i)
            thread.start()
        self._listening = True

    def _create_thread_args(self, index):
        return {'index':index,
                'num_received_messages':0,
                'num_processed_messages':0}

    def stop_listening(self):
        """
        Stop the background threads that listen to incoming messages.
        """
        # stop all running threads
        if self.running[0] == True:
            self.running[0] = False
            for thread, args in self._threads.items():
                thread.join()
                logger.info("Thread %2d: STOPPED Listening for messages on %s" % (args['index'], self.address))
                logger.info("           %d messages received and %d processed OK." % (args['num_received_messages'], args['num_processed_messages']))
        self._listening = False

        # close the listeners
        if self._bus_listener.isConnected():
            self._bus_listener.close()


    def __enter__(self):
        """
        Internal use only. Handles scope with keyword 'with'
        """
        self.start_listening()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        Internal use only. Handles scope with keyword 'with'
        """
        self.stop_listening()

    def _onListenLoopBegin(self):
        "Called before main processing loop is entered."
        pass

    def _onBeforeReceiveMessage(self):
        "Called in main processing loop just before a blocking wait for messages is done."
        pass

    def _handleMessage(self, msg):
        "Implement this method in your subclass to handle a received message"
        raise NotImplementedError("Please implement the _handleMessage method in your subclass to handle a received message")

    def _onAfterReceiveMessage(self, successful):
        "Called in the main loop after the result was send back to the requester."
        "@successful@ reflects the state of the handling: true/false"
        pass

    def _onListenLoopEnd(self):
        "Called after main processing loop is finished."
        pass

    def _loop(self):
        """
        Internal use only. Message listener loop that receives messages and starts the attached function with the message content as argument.
        """
        currentThread = threading.currentThread()
        args = self._threads[currentThread]
        thread_idx = args['index']
        logger.info( "Thread %d START Listening for messages on %s" %(thread_idx, self.address))
        try:
            self._onListenLoopBegin()
        except Exception as e:
            logger.error("onListenLoopBegin() failed with %s", e)

        while self.running[0]:
            try:
                self._onBeforeReceiveMessage()
            except Exception as e:
                logger.error("onBeforeReceiveMessage() failed with %s", e)
                continue

            try:
                # get the next message
                lofar_msg = self._bus_listener.receive(1)
                # retry if timed-out
                if lofar_msg is None:
                    continue

                # Keep track of number of received messages
                args['num_received_messages'] += 1

                # Execute the handler function and send reply back to client
                try:
                    self._debug("Running handler")

                    self._handleMessage(lofar_msg)

                    self._debug("Finished handler")

                    self._bus_listener.ack(lofar_msg)

                    args['num_processed_messages'] += 1

                    try:
                        self._onAfterReceiveMessage(True)
                    except Exception as e:
                        logger.error("onAfterReceiveMessage() failed with %s", e)
                        continue

                except Exception as e:
                    # Any thrown exceptions either Service exception or unhandled exception
                    # during the execution of the service handler is caught here.
                    self._debug(str(e))
                    try:
                        self._onAfterReceiveMessage(False)
                    except Exception as e:
                        logger.error("onAfterReceiveMessage() failed with %s", e)
                    continue

            except Exception as e:
                # Unknown problem in the library. Report this and continue.
                logger.error("[%s:] ERROR during processing of incoming message.\n%s" %(self.__class__.__name__, str(e)))
                logger.info("Thread %d: Resuming listening on %s " % (thread_idx, self.address))

        try:
            self._onListenLoopEnd()
        except Exception as e:
            logger.error("finalize_loop() failed with %s", e)


__all__ = ["FromBus", "ToBus", "AbstractBusListener"]
