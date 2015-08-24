//# MessageContent.h: Wrapper for messages to be exchanged using QPID.
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
#else
#include <MessageBus/NoQpidFallback.h>
#endif

#ifdef HAVE_LIBXMLXX
#include <libxml++/parsers/domparser.h>
#endif

#include <string>
#include <ostream>
#include <Common/Exception.h>

#include <MessageBus/XMLDoc.h>

namespace LOFAR {

/*
 * Encode the CONTENT of a message, that is sent to or received from a Message Bus.
 *
 * This is the base class that represents the LOFAR message protocol. Derived classes
 * can implement specific protocols. See the Protocols/ directory for examples.
 */
class MessageContent
{
public:
  struct Defaults {
    // Name of the system sending these messages
    static const std::string system;

    // Version of the header we write
    static const std::string headerVersion;
  };

  // Construct a message
  MessageContent();

  // With header info
  MessageContent(
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
  MessageContent(const qpid::messaging::Message &qpidMsg);

  // Create a copy
  MessageContent(const MessageContent &);

  // Set the payload, supporting various types
  void setXMLPayload (const std::string                &payload);
  void setTXTPayload (const std::string                &payload);

  virtual ~MessageContent();

  /*
   * A 'Property' is getter/setter for a part of the message content.
   *
   * It provides a string interface. Example usage (MessageContent::system is a property):
   *
   *   MessageContent content;
   *   content.system = "foo";                 // set the property to 'foo'
   *   content.system.set("foo");              // set the property to 'foo'
   *   cout << content.system.get() << endl;   // print the property
   *   cout << (string)content.system << endl; // print the property
   *   ASSERT(content.system == "foo");        // compare the property
   *   ASSERT(content.system != "bar");        // compare the property
   */
  class Property {
  public:
    // normal getters and setters
    void set(const std::string &value) { itsContent->setXMLvalue(itsKey, value); }
    std::string get() const { return itsContent->getXMLvalue(itsKey); }

    // C++ operator overloading (syntactic sugar)
    Property &operator=(const std::string &value) { set(value); return *this; }
    operator std::string () const { return get(); }

    bool operator==(const Property &other) const    { return this->get() == other.get(); }
    bool operator==(const std::string &other) const { return this->get() == other; }
    bool operator==(const char *other) const        { return this->get() == other; }

    bool operator!=(const Property &other) const    { return this->get() != other.get(); }
    bool operator!=(const std::string &other) const { return this->get() != other; }
    bool operator!=(const char *other) const        { return this->get() != other; }

    // To be used only by MessageContent and its subclasses
    Property(): itsContent(0), itsKey("") {}
    void attach(XMLDoc *content, const std::string &key) { itsContent = content; itsKey = key; }

  private:
    XMLDoc *itsContent;
    std::string itsKey;
  };

  // Return properties of the constructed or received message
  Property system;
  Property headerVersion;
  Property protocol;
  Property protocolVersion;

  Property name;
  Property user;
  Property uuid;
  Property summary;
  Property timestamp;
  Property momid;
  Property sasid;

  Property payload;
  Property header;

  // Return a NEW QPID message with our content
  qpid::messaging::Message qpidMsg() const;

  // Return the message content. Note that the content string
  // is possibly generated, and may not be an exact copy
  // of what was provided in the constructor. For a raw
  // copy of the exact message content, please use the
  // getContent() method of the QPID message itself.
  std::string getContent() const;

  // Return a short (one line) description of the message
  std::string short_desc() const;

  // function for printing
  std::ostream& print (std::ostream& os) const;

private:
  void addProperties();

protected:
  XMLDoc itsContent;
};

inline std::ostream &operator<<(std::ostream &os, const MessageContent &msg)
{
  return (msg.print(os));
}

inline std::ostream &operator<<(std::ostream &os, const MessageContent::Property &prop)
{
  os << (std::string)prop;
  return os;
}

class Message
{
public:
  Message() {}

  // Wrap a received meessage
  Message(const qpid::messaging::Message &qpidMsg) : itsQpidMsg(qpidMsg) {}

  // Create a new message
  Message(const MessageContent &content);

  // Return the wrapped QPID message
  qpid::messaging::Message& qpidMsg()             { return (itsQpidMsg); }
  const qpid::messaging::Message& qpidMsg() const { return (itsQpidMsg); }

  // Return the raw message content
  std::string rawContent() const { return itsQpidMsg.getContent(); }

  // Return a short (one line) description of the message
  std::string short_desc() const { MessageContent content(itsQpidMsg); return content.short_desc(); }

  // function for printing
  std::ostream& print (std::ostream& os) const { MessageContent content(itsQpidMsg); return content.print(os); }

private:
  // -- datamembers -- 
  qpid::messaging::Message itsQpidMsg;
};

inline std::ostream &operator<<(std::ostream &os, const Message &msg)
{    
  return (msg.print(os));
}


} // namespace LOFAR

#endif

