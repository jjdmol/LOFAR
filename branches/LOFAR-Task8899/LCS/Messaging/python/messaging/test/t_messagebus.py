# t_messagebus.py: Test program for the module lofar.messaging.messagebus
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
# $Id: t_messagebus.py 1580 2015-09-30 14:18:57Z loose $

"""
Test program for the module lofar.messaging.messagebus
"""

import re
import struct
import sys
import unittest

from lofar.messaging.messages import *
from lofar.messaging.messagebus import *
from lofar.messaging.exceptions import MessageBusError, InvalidMessage

TIMEOUT = 1.0


# ========  FromBus unit tests  ======== #

class FromBusInitFailed(unittest.TestCase):
    """
    Class to test initialization failures of FromBus
    """

    def setUp(self):
        self.error = "[FromBus] Initialization failed"

    def test_no_broker_address(self):
        """
        Connecting to non-existent broker address must raise MessageBusError
        """
        regexp = re.escape(self.error)
        regexp += '.*' + 'No address associated with hostname'
        with self.assertRaisesRegexp(MessageBusError, regexp):
            with FromBus(QUEUE, broker="foo.bar"):
                pass

    def test_connection_refused(self):
        """
        Connecting to broker on wrong port must raise MessageBusError
        """
        regexp = re.escape(self.error) + '.*' + 'Connection refused'
        with self.assertRaisesRegexp(MessageBusError, regexp):
            with FromBus("fake" + QUEUE, broker="localhost:4"):
                pass


class FromBusNotInContext(unittest.TestCase):
    """
    Class to test that exception is raised when FromBus is used outside context
    """

    def setUp(self):
        self.frombus = FromBus(QUEUE)
        self.error = "[FromBus] No active session"

    def test_add_queue_raises(self):
        """
        Adding a queue when outside context must raise MessageBusError
        """
        with self.assertRaisesRegexp(MessageBusError, re.escape(self.error)):
            self.frombus.add_queue("fooqueue")

    def test_receive_raises(self):
        """
        Getting a message when outside context must raise MessageBusError
        """
        with self.assertRaisesRegexp(MessageBusError, re.escape(self.error)):
            self.frombus.receive()

    def test_ack_raises(self):
        """
        Ack-ing a message when outside context must raise MessageBusError
        """
        with self.assertRaisesRegexp(MessageBusError, re.escape(self.error)):
            self.frombus.ack(None)

    def test_nack_raises(self):
        """
        Nack-ing a message when outside context must raise MessageBusError
        """
        with self.assertRaisesRegexp(MessageBusError, re.escape(self.error)):
            self.frombus.nack(None)

    def test_reject_raises(self):
        """
        Rejecting a message when outside context must raise MessageBusError
        """
        with self.assertRaisesRegexp(MessageBusError, re.escape(self.error)):
            self.frombus.reject(None)


class FromBusInContext(unittest.TestCase):
    """
    Class to test FromBus when inside context.
    """

    def setUp(self):
        self.frombus = FromBus(QUEUE)
        self.error = "[FromBus] Failed to create receiver for source"

    def test_add_queue_fails(self):
        """
        Adding a non-existent queue must raise MessageBusError
        """
        queue = "fake" + QUEUE
        regexp = re.escape(self.error) + '.*' + 'NotFound: no such queue'
        with self.assertRaisesRegexp(MessageBusError, regexp):
            with self.frombus:
                self.frombus.add_queue(queue)

    def test_add_queue_succeeds(self):
        """
        Adding an existing queue must succeed, resulting in one more receiver
        """
        with self.frombus:
            nr_recv = len(self.frombus.session.receivers)
            self.frombus.add_queue(QUEUE)
            self.assertEqual(nr_recv + 1, len(self.frombus.session.receivers))

    def test_receive_timeout(self):
        """
        Getting a message when there's none must yield None after timeout.
        """
        with self.frombus:
            self.assertIsNone(self.frombus.receive(timeout=TIMEOUT))


# ========  ToBus unit tests  ======== #

class ToBusInitFailed(unittest.TestCase):
    """
    Class to test initialization failures of ToBus
    """

    def setUp(self):
        self.error = "[ToBus] Initialization failed"

    def test_no_broker_address(self):
        """
        Connecting to non-existent broker address must raise MessageBusError
        """
        regexp = re.escape(self.error)
        regexp += '.*' + 'No address associated with hostname'
        with self.assertRaisesRegexp(MessageBusError, regexp):
            with ToBus(QUEUE, broker="foo.bar"):
                pass

    def test_connection_refused(self):
        """
        Connecting to broker on wrong port must raise MessageBusError
        """
        # Cache and disable auto reconnect
        old_reconnect = DEFAULT_BROKER_OPTIONS.get('reconnect', False)
        DEFAULT_BROKER_OPTIONS['reconnect'] = False

        regexp = re.escape(self.error) + '.*' + 'Connection refused'
        with self.assertRaisesRegexp(MessageBusError, regexp):
            with ToBus(QUEUE, broker="localhost:4"):
                pass

        # Restore auto reconnect
        DEFAULT_BROKER_OPTIONS['reconnect'] = old_reconnect


class ToBusSendMessage(unittest.TestCase):
    """
    Class to test different error conditions when sending a message
    """

    def setUp(self):
        self.tobus = ToBus(QUEUE)

    def test_send_outside_context_raises(self):
        """
        If a ToBus object is used outside a context, then there's no active
        session, and a MessageBusError must be raised.
        """
        regexp = re.escape("[ToBus] No active session")
        with self.assertRaisesRegexp(MessageBusError, regexp):
            self.tobus.send(None)

    def test_no_senders_raises(self):
        """
        If there are no senders, then a MessageBusError must be raised.
        Note that this can only happen if someone has deliberately tampered with
        the ToBus object.
        """
        with self.tobus:
            del self.tobus.session.senders[0]
            regexp = re.escape("[ToBus] No senders")
            self.assertRaisesRegexp(MessageBusError, regexp,
                                    self.tobus.send, None)

    def test_multiple_senders_raises(self):
        """
        If there's more than one sender, then a MessageBusError must be raised.
        Note that this can only happen if someone has deliberately tampered with
        the ToBus object (e.g., by using the protected _add_queue() method).
        """
        with self.tobus:
            self.tobus._add_queue(QUEUE, {})
            regexp = re.escape("[ToBus] More than one sender")
            self.assertRaisesRegexp(MessageBusError, regexp,
                                    self.tobus.send, None)

    def test_send_invalid_message_raises(self):
        """
        If an invalid message is sent (i.e., not an LofarMessage), then an
        InvalidMessage must be raised.
        """
        with self.tobus:
            regexp = re.escape("Invalid message type")
            self.assertRaisesRegexp(InvalidMessage, regexp,
                                    self.tobus.send, "Blah blah blah")


# ========  Combined FromBus/ToBus unit tests  ======== #

class SendReceiveMessage(unittest.TestCase):
    """
    Class to test sending and receiving a message.
    """

    def setUp(self):
        self.frombus = FromBus(QUEUE)
        self.tobus = ToBus(QUEUE)

    def _test_sendrecv(self, send_msg):
        """
        Helper class that implements the send/receive logic and message checks.
        :param send_msg: Message to send
        """
        with self.tobus, self.frombus:
            self.tobus.send(send_msg)
            recv_msg = self.frombus.receive(timeout=TIMEOUT)
            self.frombus.ack(recv_msg)
        self.assertEqual(
            (send_msg.SystemName, send_msg.MessageId, send_msg.MessageType),
            (recv_msg.SystemName, recv_msg.MessageId, recv_msg.MessageType))
        self.assertEqual(
            (send_msg.content, send_msg.content_type),
            (recv_msg.content, recv_msg.content_type))

    def test_sendrecv_event_message(self):
        """
        Test send/receive of an EventMessage, containing a string.
        """
        content = "An event message"
        self._test_sendrecv(EventMessage(content))

    def test_sendrecv_monitoring_message(self):
        """
        Test send/receive of an MonitoringMessage, containing a python list.
        """
        content = ["A", "monitoring", "message"]
        self._test_sendrecv(MonitoringMessage(content))

    def test_sendrecv_progress_message(self):
        """
        Test send/receive of an ProgressMessage, containing a python dict.
        """
        content = {"Progress": "Message"}
        self._test_sendrecv(ProgressMessage(content))

    def test_sendrecv_request_message(self):
        """
        Test send/receive of an RequestMessage, containing a byte array.
        """
        content = {"request": "Do Something", "argument": "Very Often"}
        self._test_sendrecv(RequestMessage(content, reply_to=QUEUE))


if __name__ == '__main__':
    QUEUE = sys.argv[1] if len(sys.argv) > 1 else "testqueue"
    del sys.argv[1:]
    unittest.main()
