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
  Message(
    // Name of the service or process producing this message
    const std::string &from,

    // End-user responsible for this request (if applicable)
    const std::string &forUser,

    // Human-readable summary describing this request
    const std::string &summary,

    // Service to send this message to
    const std::string &toService,

    // Version of the protocol we're using to describe the payload
    const std::string &toVersion
  );

  // Parse a message
  Message(const qpid::messaging::Message &qpidMsg);

  // Read a message from disk (header + payload)
  Message(const std::string &rawContent);

  // Set the payload, supporting various types
  void setXMLPayload (const std::string                &payload);
  void setTXTPayload (const std::string                &payload);
  void setMapPayload (const qpid::types::Variant::Map  &payload);
  void setListPayload(const qpid::types::Variant::List &payload);

  virtual ~Message();

  // Return properties of the constructed or received message
  std::string system() const;
  std::string headerVersion() const;
  std::string payload() const;
  std::string from() const;
  std::string forUser() const;
  std::string summary() const;
  std::string toService() const;
  std::string toVersion() const;

  // Construct the given fields as a QPID message
  qpid::messaging::Message getQpidMsg() const { return (itsQpidMsg); }

  // Return the raw message (header + payload)
  std::string getRawContent() const { return (itsQpidMsg.getContent()); }

private:
  // datamembers
  qpid::messaging::Message itsQpidMsg;
};

std::ostream &operator<<(std::ostream &s, const Message &msg);

} // namespace LOFAR

#endif

#endif

