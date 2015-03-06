//# Message.h: Wrapper for messages to be exchanged using QPID.
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_MESSAGEBUS_MESSAGE_H
#define LOFAR_MESSAGEBUS_MESSAGE_H

#ifdef HAVE_QPID
#include <qpid/messaging/Message.h>
#include <qpid/types/Variant.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string>
#include <ostream>

namespace LOFAR {

// Name of the system sending these messages
static const std::string system = "LOFAR";

// Version of the header we write
static const std::string headerVersion = "1.0.0";


class Message
{
public:
  // Construct a message
  Message() {};

  // With header info
  Message(
    // Name of the service or process producing this message
    const std::string &from,

    // End-user responsible for this request (if applicable)
    const std::string &forUser,

    // Human-readable summary describing this request
    const std::string &summary,

    // Protocol of the contents of the message
    const std::string &protocol,

    // Version of the protocol we're using to describe the payload
    const std::string &protocolVersion,

    // momID of the information (if available)
    const std::string &momID,

    // sasID of the information (if available)
    const std::string &sasID
  );

  // Parse a message
  Message(const qpid::messaging::Message qpidMsg);

  // Read a message from disk (header + payload)
  Message(const std::string &rawContent);

  // Set the payload, supporting various types
  void setXMLPayload (const std::string                &payload);
  void setTXTPayload (const std::string                &payload);

  //TODO: Not implemented
  void setMapPayload (const qpid::types::Variant::Map  &payload);
  //TODO: Not implemented
  void setListPayload(const qpid::types::Variant::List &payload);

  virtual ~Message();

  // Return properties of the constructed or received message
  std::string system() const		  { return (getXMLvalue("message.header.system")); }
  std::string headerVersion() const   { return (getXMLvalue("message.header.version")); }
  std::string protocol() const		  { return (getXMLvalue("message.header.protocol.name")); }
  std::string protocolVersion() const { return (getXMLvalue("message.header.protocol.version")); }
  std::string from() const			  { return (getXMLvalue("message.header.source.name")); }
  std::string forUser() const		  { return (getXMLvalue("message.header.source.user")); }
  std::string uuid() const			  { return (getXMLvalue("message.header.source.uuid")); }
  std::string summary() const		  { return (getXMLvalue("message.header.source.summary")); }
  std::string timestamp() const		  { return (getXMLvalue("message.header.source.timestamp")); }
  std::string momid() const			  { return (getXMLvalue("message.header.ids.momid")); }
  std::string sasid() const			  { return (getXMLvalue("message.header.ids.sasid")); }
  std::string payload() const		  { return (getXMLvalue("message.payload")); }
  std::string header() const		  { return (getXMLvalue("message.header")); }

  // Construct the given fields as a QPID message
  qpid::messaging::Message& qpidMsg() { return (itsQpidMsg); }

  // Return the raw message (header + payload)
  std::string rawContent() const { return (itsQpidMsg.getContent()); }

  // Return a short (one line) description of the message
  std::string short_desc() const;

  // function for printing
  std::ostream& print (std::ostream& os) const;

  // Parse internal xml representation
  std::string getXMLvalue(const std::string& key) const;

  // Parses the provided string and return an xml doc
  // return xml is owned by the caller, does not change the state
  // of the msg object
  xmlDocPtr parseXMLString(const std::string& xml_string);



private:
  // -- datamembers -- 
  qpid::messaging::Message itsQpidMsg;

  // xml representation of the msg head+payload
  xmlDocPtr content_as_xml_document;

  // -- Members --

  // parse the message to internal xml storage
  void parse_xml();

  // -- Members --

};

inline std::ostream &operator<<(std::ostream &os, const Message &msg)
{	
  return (msg.print(os));
}

} // namespace LOFAR

#endif

#endif

