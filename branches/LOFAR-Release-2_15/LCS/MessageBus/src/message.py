#!/usr/bin/env python
# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
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

try:
  import qpid.messaging as messaging
  MESSAGING_ENABLED = True
except ImportError:
  import noqpidfallback as messaging
  MESSAGING_ENABLED = False

import xml.dom.minidom as xml
import xml.parsers.expat as expat
from xml.sax.saxutils import escape
import datetime

#
# The template for the LOFAR message format.
#
LOFAR_MSG_TEMPLATE = """
<message>
   <header>
      <system/>
      <version/>
      <protocol>
         <name/>
         <version/>
      </protocol>
      <source>
         <name/>
         <user/>
         <uuid/>
         <timestamp/>
         <summary/>
      </source>
      <ids>
         <momid/>
         <sasid/>
      </ids>
   </header>
   <payload/>
</message>"""

def _timestamp():
  """
    Return the current time as YYYY-MM-DDTHH:MM:SS
  """
  now = datetime.datetime.now()
  return now.strftime("%FT%T")

def _uuid():
  """
    Return an UUID
  """
  return str(messaging.uuid4())

class MessageException(Exception):
    pass

class XMLDoc(object):
    def __init__(self, content):
      try:
        self.document = xml.parseString(content)
      except expat.ExpatError, e:
        #print "Could not parse XML message content: ", e, qpidMsg.content
        raise MessageException(e)

    def content(self):
      """ Return the XML document in string form. """
      return self.document.toxml()

    def getXMLdata(self, name):
      """ Return the value of an XML key, given by its XPath.
          Raise MessageException if name is N/A. """
      node = self._getXMLnode(name)
      if node is None:
          raise MessageException('node ' + name + ' not in XML document')

      return self._get_data(node)

    def setXMLdata(self, name, data):
      """ Set the value of an XML key, given by its XPath.
          Raise MessageException if name is N/A. """
      node = self._getXMLnode(name)
      if node is None:
          raise MessageException('node ' + name + ' not in XML document')

      return self._set_data(node, data)

    def insertXML(self, parent, xmlStr):
      """ Insert XML into the current message. """
      doc = xml.parseString(xmlStr)

      self._getXMLnode(parent).appendChild(doc.firstChild)

    """ XML support functions. See also lofarpipe/support/xmllogging.py. """

    def _get_child(self, node, name):
      """ Return a specific child node. """

      for child in node.childNodes:
        if child.nodeName == name and child.nodeType == child.ELEMENT_NODE:
          return child

      return None

    def _get_data(self, node):
      """ Return the textual content of a node. """

      data = []
      for child in node.childNodes:
        if child.nodeType == child.TEXT_NODE:
          data.append(child.data)

      return ''.join(data)

    def _set_data(self, node, data):
      """ Set the textual content of a node. """

      newchild = self.document.createTextNode(data)

      for child in node.childNodes:
        if child.nodeType == child.TEXT_NODE:
          node.replaceChild(newchild, child)
          break;
      else:
        node.appendChild(newchild)

    def _getXMLnode(self, name):
      """ Return a node given by its dot-separated path name.
          So a.b.c returns the inner node of <a><b><c></c></b></a>. """
      parts = name.split(".")
      node = self.document

      for p in parts:
        node = self._get_child(node,p)

        if node is None:
          return None

      return node

class MessageContent(object):
    """
      Describes the content of a message, which can be constructed from either a set of fields, or from
      an existing QPID message.
    """

    class Defaults(object):
      system = "LOFAR"
      headerVersion = "1.0.0"

    def __init__(self, from_="", forUser="", summary="", protocol="", protocolVersion="", momid="", sasid="", qpidMsg=None):
      # Add properties to get/set header fields
      for name, element in self._property_list().iteritems():
        self._add_property(name, element)

      # Set the content from either the parameters or from the provided qpidMsg
      if qpidMsg is None:
        self.document = XMLDoc(LOFAR_MSG_TEMPLATE)

        # Set properties provided by constructor
        self.system          = self.Defaults.system
        self.headerVersion   = self.Defaults.headerVersion
        self.protocol        = protocol
        self.protocolVersion = protocolVersion
        self.from_           = from_
        self.forUser         = forUser
        self.summary         = summary
        self.uuid            = _uuid()
        self.timestamp       = _timestamp()
        self.momid           = momid
        self.sasid           = sasid
      else:  # Set properties by provided qpidMsg
        # MessageBus msg fmt def and C++ side are broken, hence deprecated.
        # Try to encode '<', '&', '>' in the content payload, whenever possible.
        # Content header should not have these. For C++ MessageBus non-libxml++
        # builds, skip encode if XML tags continue in <payload>. Hack ahead!
        if qpidMsg.content is None:
          qpidMsg.content = ''  # avoid find() or replace() via escape() on None
        plIdx = qpidMsg.content.find('<payload>')
        if plIdx != -1:
          plIdx += len('<payload>')
          plEndIdx = qpidMsg.content.rfind('</payload>', plIdx)
          if plEndIdx != -1:
            eqIdx = qpidMsg.content.find('=', plIdx, plEndIdx)  # non-empty parset
            if eqIdx != -1 and eqIdx < qpidMsg.content.find('<', plIdx, plEndIdx):
              qpidMsg.content = qpidMsg.content[ : plIdx] + \
                                escape(qpidMsg.content[plIdx : plEndIdx]) + \
                                qpidMsg.content[plEndIdx : ]
        self.document = XMLDoc(qpidMsg.content)  # may raise MessageException

    def _add_property(self, name, element):
      def getter(self):
        return self.document.getXMLdata(element)
      def setter(self, value):
        self.document.setXMLdata(element, str(value))

      setattr(self.__class__, name, property(getter, setter))

    def _property_list(self):
      """ List of XML elements that are exposed as properties. """
      return { 
        "system":          "message.header.system",
        "headerVersion":   "message.header.version",
        "protocol":        "message.header.protocol.name",
        "protocolVersion": "message.header.protocol.version",
        "from_":           "message.header.source.name",
        "forUser":         "message.header.source.user",
        "uuid":            "message.header.source.uuid",
        "summary":         "message.header.source.summary",
        "timestamp":       "message.header.source.timestamp",
        "momid":           "message.header.ids.momid",
        "sasid":           "message.header.ids.sasid",
        "payload":         "message.payload",
        "header":          "message.header",
      }

    """ API (apart from properties). """

    def __repr__(self):
      return "MessageContent(%s %s)" % (self.protocol, self.protocolVersion)

    def __str__(self):
      return "[%s] [sasid %s] %s" % (self.uuid, self.sasid, self.summary)

    def content(self):
      """ Construct the literal message content. """

      return self.document.content()

    def qpidMsg(self):
      """ Construct a NEW QPID message. """

      msg = messaging.Message(content_type="text/plain", durable=True)
      msg.content = self.content()

      return msg


class Message(object):
    """
      Describes a QPID message, which can be received or sent.
    """

    def __init__(self, qpidMsg=None):
      self._qpidMsg = qpidMsg

    """ API (apart from properties). """

    def qpidMsg(self):
      return self._qpidMsg

    def content(self):
      return MessageContent(qpidMsg=self._qpidMsg)

    def raw_content(self):
      return self._qpidMsg.content

    def __repr__(self):
      msg = self.content()
      return "Message(%s %s)" % (msg.protocol, msg.protocolVersion)

if __name__ == "__main__":
  m = MessageContent("FROM", "FORUSER", "SUMMARY", "PROTOCOL", "1.2.3", "11111", "22222")
  print str(m)
  print m.content()

