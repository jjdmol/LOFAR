# messages.py: Message classes used by the package lofar.messaging.
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
# $Id: messages.py 1580 2015-09-30 14:18:57Z loose $

"""
Message classes used by the package lofar.messaging.
"""

import qpid.messaging
import uuid

from lofar.common.factory import Factory
from lofar.messaging.exceptions import InvalidMessage, MessageFactoryError


# Valid QPID message fields (from qpid.messaging.Message)
_QPID_MESSAGE_FIELDS = set([
    'content', 'content_type', 'correlation_id', 'durable', 'id',
    'priority', 'properties', 'reply_to', 'subject', 'ttl', 'user_id'])


def _validate_qpid_message(qmsg):
    """
    Validate Qpid message `qmsg`. A Qpid message is required to contain the
    following properties in order to be considered valid:
    - "SystemName"  : "LOFAR"
    - "MessageType" : message type string
    - "MessageId"   : a valid UUID string
    :raises InvalidMessage: if any of the required properties are missing in
    the Qpid message
    """
    required_props = set(["SystemName", "MessageType", "MessageId"])
    if not isinstance(qmsg, qpid.messaging.Message):
        raise InvalidMessage(
            "Not a Qpid Message: %r" % type(qmsg)
        )
    msg_props = qmsg.properties
    if not isinstance(msg_props, dict):
        raise InvalidMessage(
            "Invalid message properties type: %r (expected %r)" %
            (type(msg_props), type(dict()))
        )
    illegal_props = _QPID_MESSAGE_FIELDS.intersection(msg_props)
    if illegal_props:
        raise InvalidMessage(
            "Illegal message propert%s (Qpid reserved): %r" %
            ("ies" if len(illegal_props) > 1 else "y", ', '.join(illegal_props))
        )
    missing_props = required_props.difference(msg_props)
    if missing_props:
        raise InvalidMessage(
            "Missing message propert%s: %s" %
            ("ies" if len(missing_props) > 1 else "y", ', '.join(missing_props))
        )
    sysname, _, msgid = (msg_props[prop] for prop in required_props)
    if sysname != "LOFAR":
        raise InvalidMessage(
            "Invalid message property 'SystemName': %s" % sysname
        )
    try:
        uuid.UUID(msgid)
    except Exception:
        raise InvalidMessage(
            "Invalid message property 'MessageId': %s" % msgid
        )


def to_qpid_message(msg):
    """
    Convert `msg` into a Qpid message.
    :param msg: Message to be converted into a Qpid message.
    :return: Qpid message
    :raise InvalidMessage if `msg` cannot be converted into a Qpid message.
    """
    if isinstance(msg, qpid.messaging.Message):
        return msg
    if isinstance(msg, LofarMessage):
        return msg.qpid_msg
    raise InvalidMessage("Invalid message type: %r" % type(msg))

class MessageFactory(Factory):
    """
    Factory to produce LofarMessage objects.
    """

    def create(self, qmsg):
        """
        Override the create method to restrict the number of arguments to one
        and to do some extra testing.
        :param qmsg: Qpid message that must be converted into an LofarMessage.
        :return: Instance of a child class of LofarMessage.
        :raise MessageFactoryError: if the MessageType property of the Qpid
        message `qmsg` contains a type name that is not registered with the
        factory.
        """
        _validate_qpid_message(qmsg)
        clsid = qmsg.properties['MessageType']
        msg = super(MessageFactory, self).create(clsid, qmsg)
        if msg is None:
            raise MessageFactoryError(
                "Don't know how to create a message of type %s. "
                "Did you register the class with the factory?" % clsid)
        return msg


# Global MessageFactory object, to be used by the lofar.messaging package,
MESSAGE_FACTORY = MessageFactory()


class LofarMessage(object):
    """
    Describes the content of a message, which can be constructed from either a
    set of fields, or from an existing QPID message.

    We want to provide a uniform interface to the user so that any message
    property or field, either defined by Qpid of by us, can be accessed as an
    object attribute. To do so, we needed to implement the `__getattr__` and
    `__setattr` methods; they "extract" our message properties from the Qpid
    message properties and provide direct access to them.
    """

    def __init__(self, content=None):
        """Constructor.

        :param content: Content can either be a qpid.messaging.Message object,
        or an object that is valid content for a qpid.messaging.Message. In the
        first case, `content` must contain all the message properties that are
        required by LOFAR.
        :raise InvalidMessage if `content` cannot be used to initialize an
        LofarMessage object.

        note:: Because every access to attributes will be caught by
        `__getattr__` and `__setattr__`, we need to use `self.__dict__` to
        initialize our attributes; otherwise a `KeyError` exception will be
        raised.
        """
        if isinstance(content, qpid.messaging.Message):
            _validate_qpid_message(content)
            self.__dict__['_qpid_msg'] = content
        else:
            try:
                if isinstance(content,basestring):
                    self.__dict__['_qpid_msg'] = qpid.messaging.Message(unicode(content))
                else:
                    self.__dict__['_qpid_msg'] = qpid.messaging.Message(content)

            except KeyError:
                raise InvalidMessage(
                    "Unsupported content type: %r" % type(content))
            else:
                self._qpid_msg.properties.update({
                    'SystemName': 'LOFAR',
                    'MessageId': str(uuid.uuid4()),
                    'MessageType': self.__class__.__name__})

    def __getattr__(self, name):
        """
        Catch read-access to attributes to fetch our message properties from
        the Qpid message properties field. Direct access to the Qpid message
        properties is not allowed.

        :raises: AttributeError
        """
        if name != 'properties':
            if name in _QPID_MESSAGE_FIELDS:
                return self.__dict__['_qpid_msg'].__dict__[name]
            if name in self.__dict__['_qpid_msg'].__dict__['properties']:
                return self.__dict__['_qpid_msg'].__dict__['properties'][name]
        raise AttributeError("%r object has no attribute %r" %
                             (self.__class__.__name__, name))

    def __setattr__(self, name, value):
        """
        Catch write-access to data members to put our message properties into
        the Qpid message properties field. Direct access to the Qpid message
        properties is not allowed.

        :raises: AttributeError
        """
        if name != 'properties':
            if name in _QPID_MESSAGE_FIELDS:
                self.__dict__['_qpid_msg'].__dict__[name] = value
            else:
                self.__dict__['_qpid_msg'].__dict__['properties'][name] = value
        else:
            raise AttributeError("%r object has no attribute %r" %
                                 (self.__class__.__name__, name))

    def prop_names(self):
        """
        Return a list of all the message property names that are currently
        defined.
        """
        return list(
            _QPID_MESSAGE_FIELDS.union(self._qpid_msg.properties) -
            set(['properties'])
        )

    @property
    def qpid_msg(self):
        """
        Return the Qpid message object itself.
        """
        return self._qpid_msg

    def show(self):
        """
        Print all the properties of the current message. Make a distinction
        between user-defined properties and standard Qpid properties.
        """
        print str(self)

    def __str__(self):
        result = ''
        for (key, value) in \
                self.__dict__['_qpid_msg'].__dict__['properties'].iteritems():
            result += "%s: %s\n" % (key, value)

        result += "---\n"

        for key in _QPID_MESSAGE_FIELDS:
            if (key != 'properties' and
                    self.__dict__['_qpid_msg'].__dict__[key] is not None):
                result += "%s:%s\n" % (key, self.__dict__['_qpid_msg'].__dict__[key])
        result += "===\n"
        return result


class EventMessage(LofarMessage):
    """
    Message class used for event messages. Events are messages that *must*
    be delivered. If the message cannot be delivered to the recipient, it
    will be stored in a persistent queue for later delivery.
    """

    def __init__(self, content=None, context=None):
        super(EventMessage, self).__init__(content)
        if (context!=None):
            self.durable = True
            self.subject = context


class MonitoringMessage(LofarMessage):
    """
    Message class used for monitoring messages. Monitoring messages are
    publish-subscribe type of messages. They will be not be queued, so they
    will be lost if there are no subscribers.
    """

    def __init__(self, content=None):
        super(MonitoringMessage, self).__init__(content)


class ProgressMessage(LofarMessage):
    """
    Message class used for progress messages. Progress messages are
    publish-subscribe type of messages. They will be not be queued, so they
    will be lost if there are no subscribers.
    """

    def __init__(self, content=None):
        super(ProgressMessage, self).__init__(content)


class RequestMessage(LofarMessage):
    """
    Message class used for service messages. Service messages are
    request-reply type of messages. They are typically used to query a
    subsystem. A service message must contain a valid ``ReplyTo`` property.
    """

    #TODO: refactor args kwargs quirks
    def __init__(self, content=None, reply_to=None,**kwargs): #reply_to=None, has_args=None, has_kwargs=None):
        super(RequestMessage, self).__init__(content)
        if (reply_to!=None):
            #if (len(kwargs)>0):
            #reply_to = kwargs.pop("reply_to",None)
            #if (reply_to!=None):
            self.reply_to = reply_to
            self.has_args   = kwargs.pop("has_args",False)
            self.has_kwargs = kwargs.pop("has_kwargs",False)

class ReplyMessage(LofarMessage):
    """
    Message class used for reply messages. Reply messages are part of the
    request-reply type of messages. They are typically used as a reply on a service
    message. These use topic exchanges and thus are routed by the 'subject' property
    """

    def __init__(self, content=None, reply_to=None):
        super(ReplyMessage, self).__init__(content)
        if (reply_to!=None):
            self.subject = reply_to



MESSAGE_FACTORY.register("EventMessage", EventMessage)
MESSAGE_FACTORY.register("MonitoringMessage", MonitoringMessage)
MESSAGE_FACTORY.register("ProgressMessage", ProgressMessage)
MESSAGE_FACTORY.register("RequestMessage", RequestMessage)
MESSAGE_FACTORY.register("ReplyMessage", ReplyMessage)

__all__ = ["EventMessage", "MonitoringMessage", "ProgressMessage",
	   "RequestMessage", "ReplyMessage"]
