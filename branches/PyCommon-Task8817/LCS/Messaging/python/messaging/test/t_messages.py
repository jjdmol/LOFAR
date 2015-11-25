# t_message.py: Test program for the module lofar.messaging.message
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
# $Id: t_messages.py 1576 2015-09-29 15:22:28Z loose $

"""
Test program for the module lofar.messaging.message
"""

import unittest
import uuid
import struct
import qpid.messaging
from lofar.messaging.messages import LofarMessage, InvalidMessage


class DefaultLofarMessage(unittest.TestCase):
    """
    Class to test default constructed LofarMessage class
    """

    def setUp(self):
        """
        Create default constructed object
        """
        self.message = LofarMessage()

    def test_system_name(self):
        """
        Object attribute SystemName must be set to 'LOFAR'
        """
        self.assertEqual(self.message.SystemName, "LOFAR")

    def test_message_id(self):
        """
        Object attribute MessageId must be a valid UUID string
        """
        self.assertIsNotNone(uuid.UUID(self.message.MessageId))


class QpidLofarMessage(unittest.TestCase):
    """
    Class to test LofarMessage constructed from a Qpid message
    """

    def setUp(self):
        """
        Create Qpid message with all required properties set
        """
        self.qmsg = qpid.messaging.Message()
        self.qmsg.properties = {
            "SystemName": "LOFAR",
            "MessageType": None,
            "MessageId": str(uuid.uuid4())
        }

    def test_invalid_properties_type(self):
        """
        Test that exception is raised if Qpid message properties attribute is
        of incorrect type (i.e. not 'dict').
        """
        self.qmsg.properties = 42
        self.assertRaisesRegexp(InvalidMessage,
                                "^Invalid message properties type:",
                                LofarMessage, self.qmsg)

    def test_illegal_properties(self):
        """
        Test that exception is raised if a Qpid-reserved attribute (like
        'content', 'content_type', etc.) is used as property.
        """
        self.qmsg.properties['content'] = 'blah blah blah'
        self.assertRaisesRegexp(InvalidMessage,
                                "^Illegal message propert(y|ies).*:",
                                LofarMessage, self.qmsg)

    def test_missing_properties(self):
        """
        Test that exception is raised if required properties for constructing
        an LofarMessage are missing.
        """
        self.qmsg.properties = {}
        self.assertRaisesRegexp(InvalidMessage,
                                "^Missing message propert(y|ies):",
                                LofarMessage, self.qmsg)

    def test_missing_property_systemname(self):
        """
        Test that exception is raised if required property 'SystemName' is
        missing.
        """
        self.qmsg.properties.pop("SystemName")
        self.assertRaisesRegexp(InvalidMessage,
                                "^Missing message property: SystemName",
                                LofarMessage, self.qmsg)

    def test_missing_property_messageid(self):
        """
        Test that exception is raised if required property 'MessageId' is
        missing.
        """
        self.qmsg.properties.pop("MessageId")
        self.assertRaisesRegexp(InvalidMessage,
                                "^Missing message property: MessageId",
                                LofarMessage, self.qmsg)

    def test_missing_property_messagetype(self):
        """
        Test that exception is raised if required property 'MessageType' is
        missing.
        """
        self.qmsg.properties.pop("MessageType")
        self.assertRaisesRegexp(InvalidMessage,
                                "^Missing message property: MessageType",
                                LofarMessage, self.qmsg)

    def test_invalid_property_systemname(self):
        """
        Test that exception is raised if 'SystemName' has wrong value (i.e.
        not equal to 'LOFAR')
        """
        self.qmsg.properties["SystemName"] = "NOTLOFAR"
        self.assertRaisesRegexp(InvalidMessage,
                                "^Invalid message property 'SystemName':",
                                LofarMessage, self.qmsg)

    def test_invalid_property_messageid(self):
        """
        Test that exception is raised if 'MessageId' contains an invalid
        UUID-string.
        """
        self.qmsg.properties["MessageId"] = "Invalid-UUID-string"
        self.assertRaisesRegexp(InvalidMessage,
                                "^Invalid message property 'MessageId':",
                                LofarMessage, self.qmsg)

    def test_getattr_raises(self):
        """
        Test that exception is raised if a non-existent attribute is read.
        """
        msg = LofarMessage(self.qmsg)
        with self.assertRaisesRegexp(AttributeError, "object has no attribute"):
            _ = msg.non_existent

    def test_getattr_raises_on_properties(self):
        """
        Test that exception is raised if attribute 'properties' is read.
        This attribute should not be visible.
        """
        msg = LofarMessage(self.qmsg)
        with self.assertRaisesRegexp(AttributeError, "object has no attribute"):
            _ = msg.properties

    def test_setattr_raises_on_properties(self):
        """
        Test that exception is raised if attribute 'properties' is written.
        This attribute should not be visible.
        """
        msg = LofarMessage(self.qmsg)
        with self.assertRaisesRegexp(AttributeError, "object has no attribute"):
            msg.properties = {}

    def test_getattr_qpid_field(self):
        """
        Test that a Qpid message field becomes an LofarMessage attribute.
        """
        msg = LofarMessage(self.qmsg)
        msg.qpid_msg.ttl = 100
        self.assertEqual(self.qmsg.ttl, msg.ttl)
        self.assertEqual(msg.ttl, 100)

    def test_setattr_qpid_field(self):
        """
        Test that an LofarMessage attribute becomes a Qpid message field.
        """
        msg = LofarMessage(self.qmsg)
        msg.ttl = 100
        self.assertEqual(self.qmsg.ttl, msg.ttl)
        self.assertEqual(self.qmsg.ttl, 100)

    def test_getattr_qpid_property(self):
        """
        Test that a Qpid message property becomes an LofarMessage attribute.
        """
        self.qmsg.properties["NewProperty"] = "New Property"
        msg = LofarMessage(self.qmsg)
        self.assertEqual(msg.qpid_msg.properties["NewProperty"],
                         msg.NewProperty)

    def test_setattr_qpid_property(self):
        """
        Test that an LofarMessage attribute becomes a Qpid message property.
        """
        msg = LofarMessage(self.qmsg)
        msg.NewProperty = "New Property"
        self.assertEqual(msg.qpid_msg.properties["NewProperty"],
                         msg.NewProperty)

    def test_propname_not_contains_properties(self):
        """
        Test that prop_names() does not return the property 'properties'.
        This attribute should not be visible.
        """
        msg = LofarMessage(self.qmsg)
        self.assertNotIn('properties', msg.prop_names())


class ContentLofarMessage(unittest.TestCase):
    """
    Class to test that an LofarMessage can be constructed from different types
    of content. The content is used to initialize a Qpid Message object.
    """

    def test_construct_from_string(self):
        """
        Test that an LofarMessage can be constructed from an ASCII string.
        """
        content = "ASCII string"
        msg = LofarMessage(content)
        self.assertEqual((msg.content, msg.content_type),
                         (unicode(content), 'text/plain'))

    def test_construct_from_unicode(self):
        """
        Test that an LofarMessage can be constructed from a Unicode string.
        :return:
        """
        content = u"Unicode string"
        msg = LofarMessage(content)
        self.assertEqual((msg.content, msg.content_type),
                         (content, "text/plain"))

    def test_construct_from_list(self):
        """
        Test that an LofarMessage can be constructed from a python list.
        """
        content = range(10)
        msg = LofarMessage(content)
        self.assertEqual((msg.content, msg.content_type),
                         (content, "amqp/list"))

    def test_construct_from_dict(self):
        """
        Test that an LofarMessage can be constructed from a python dict.
        """
        content = {1: 'one', 2: 'two', 3: 'three'}
        msg = LofarMessage(content)
        self.assertEqual((msg.content, msg.content_type),
                         (content, "amqp/map"))

    # def test_construct_from_binary(self):
    #    """
    #    Test that an LofarMessage can be constructed from binary data.
    #    Use struct.pack() to create a byte array
    #    """
    #    content = struct.pack("<256B", *range(256))
    #    msg = LofarMessage(content)
    #    self.assertEqual((msg.content, msg.content_type),
    #                     (content, None))

    def test_construct_from_unsupported(self):
        """
        Test that an LofarMessage cannot be constructed from unsupported
        data type like 'int'.
        """
        content = 42
        self.assertRaisesRegexp(InvalidMessage, "^Unsupported content type:",
                                LofarMessage, content)


if __name__ == '__main__':
    unittest.main()
