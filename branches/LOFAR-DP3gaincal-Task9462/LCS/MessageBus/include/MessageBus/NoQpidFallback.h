//# NoQpidFallback.h: A fake implementation of the QPID API in case QPID is not installed
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

#ifndef LOFAR_MESSAGEBUS_NO_QPID_FALLBACK_H
#define LOFAR_MESSAGEBUS_NO_QPID_FALLBACK_H

#ifndef HAVE_QPID
#include <Common/LofarLogger.h>
#include <string>
#include <map>
#include <list>
#include <ostream>
#include <climits>

/*
 * Provide all the classes and types in the qpid namespace
 * that are used by LOFAR. Where possible, return codes will
 * indicate that messages are not sent or received.
 *
 * No exceptions are thrown. Messages only keep track of their content.
 *
 * Also, using this mock will result in WARNINGS in the log
 * that messaging will not take place.
 */

namespace qpid {
  namespace types {
    typedef std::exception Exception;

    class Variant {
    public:
      typedef std::map<std::string, std::string> Map;
      typedef std::list<std::string> List;
    };

    class Uuid {
    public:
      Uuid(bool) {}

      std::string str() const { return "uuid"; }
    };
  }

  namespace messaging {
    class Duration {
    public:
      Duration(unsigned long) {}

      static const unsigned long FOREVER = ULONG_MAX;
    };

    typedef std::string Address;

    class Message {
    public:
      Message() {}
      Message(const std::string &content): content(content) {}

      void setContent(const std::string &newContent) { content = newContent; }
      std::string getContent() const { return content; }

      void setContentType(const std::string &) {}
      void setDurable(bool) {}

      std::string getMessageId() const     { return ""; }
      std::string getUserId() const        { return ""; }
      std::string getCorrelationId() const { return ""; }
      std::string getSubject() const       { return ""; }
      std::string getReplyTo() const       { return ""; }
      std::string getContentType() const   { return ""; }
      unsigned    getPriority() const      { return 0;  }
      bool        getDurable() const       { return false; }
      bool        getRedelivered() const   { return false; }
      types::Variant::Map getProperties() const { return types::Variant::Map(); }
      unsigned    getContentSize() const   { return content.size(); }
      unsigned    getTtl() const           { return 0; }
    protected:
      std::string content;
    };

    class Receiver {
    public:
      void setCapacity(unsigned) {}

      std::string getName() const { return ""; }

      bool get(Message &, Duration = Duration::FOREVER) { return false; }  
    };

    class Sender {
    public:
      std::string getName() const { return ""; }

      void send(const Message &, bool) {}
    };

    class Session {
    public:
      void acknowledge(Message&) {}
      void reject(Message&) {}
      void release(Message&) {}

      Sender createSender(const std::string &) { return Sender(); }
      Receiver createReceiver(const std::string &) { return Receiver(); }

      bool nextReceiver(Receiver&, Duration) { return false; }
    };

    class Connection {
    public:
      Connection(const std::string &, const std::string &) {
        LOG_WARN_STR("QPID support NOT enabled! Will NOT connect to any broker, and messages will be lost!");
      }

      std::string getUrl() const { return ""; }

      void open() {}
      void close() {}

      Session createSession() { return Session(); }
    };
  }
}

std::ostream& operator<<(std::ostream& out, const qpid::types::Variant& value);
std::ostream& operator<<(std::ostream& out, const qpid::types::Variant::Map& map);
std::ostream& operator<<(std::ostream& out, const qpid::types::Variant::List& list);

#endif

#endif

