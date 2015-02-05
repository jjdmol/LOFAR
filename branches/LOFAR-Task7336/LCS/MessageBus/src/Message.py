#!/usr/bin/python
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

import qpid.messaging
import xml.dom.minidom as xml

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
   <payload>
   </payload>
</message>"""

class Message(object):
    def __init__(self, from_, forUser, summary, protocol, protocolVersion, momid, sasid):
      self.document = xml.parseString(LOFAR_MSG_TEMPLATE)

      for name, element in self._property_list().iteritems():
        self._add_property(name, element)

      # Set properties provided by constructor
      self.system          = "LOFAR"
      self.headerVersion   = "1.0.0"
      self.protocol        = protocol
      self.protocolVersion = protocolVersion
      self.from_           = from_
      self.forUser         = forUser
      self.summary         = summary
      self.uuid            = ""
      self.timestamp       = ""
      self.momid           = momid
      self.sasid           = sasid

    def _add_property(self, name, element):
      def getter(self):
        return self._getXMLdata(element)
      def setter(self, value):
        self._setXMLdata(element, value)

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
      return "Message(%s %s)" % (self.protocol, self.protocolVersion)

    def __str__(self):
      return ("system         : %s\n" % (self.system,) +
              "systemversion  : %s\n" % (self.headerVersion,) +
              "protocol       : %s\n" % (self.protocol,) +
              "protocolVersion: %s\n" % (self.protocolVersion,) +
              "summary        : %s\n" % (self.summary,) +
              "timestamp      : %s\n" % (self.timestamp,) +
              "source         : %s\n" % (self.from_,) +
              "user           : %s\n" % (self.forUser,) +
              "uuid           : %s\n" % (self.uuid,) +
              "momid          : %s\n" % (self.momid,) +
              "sasid          : %s\n" % (self.sasid,) +
              "payload        : %s\n" % (self.payload,)
             )

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

    def _getXMLdata(self, name):
      return self._get_data(self._getXMLnode(name))

    def _setXMLdata(self, name, data):
      return self._set_data(self._getXMLnode(name), data)

    def qpidMsg(self):
      qpidMsg = qpid.messaging.Message(self.document.toxml())

if __name__ == "__main__":
  m = Message("FROM", "FORUSER", "SUMMARY", "PROTOCOL", "1.2.3", "11111", "22222")
  print str(m)
  print m.document.toxml()

