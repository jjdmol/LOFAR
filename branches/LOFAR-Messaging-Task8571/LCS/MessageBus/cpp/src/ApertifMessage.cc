//# ApertifMessage.cc: Top-level message class for APERTIF messages.
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the APERTIF Software Suite.
//#
//# The APERTIF Software Suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or (at your
//# option) any later version.
//#
//# The APERTIF Software Suite is distributed in the hope that it will be
//# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along with
//# The APERTIF Software Suite.  If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: ApertifMessage.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>
#include <Messaging/ApertifMessage.h>
#include <Messaging/Exceptions.h>

#include <sstream>

namespace APERTIF
{
  namespace Messaging
  {
    using namespace std;

    //## ----  Definition of protected methods  ---- ##//

    ApertifMessage::ApertifMessage() :
      Message()
    {
      setProperty("SystemName", "APERTIF");
      setProperty("MessageId", qpid::types::Uuid(true).str());
    }


    ApertifMessage::ApertifMessage(const qpid::messaging::Message& qmsg) :
      Message(qmsg)
    {
      // Check systemName: must be "APERTIF"
      string systemName(getProperty("SystemName"));
      if (systemName != "APERTIF") {
        THROW (InvalidMessage, 
               "Message contains wrong system name: " << systemName);
      }

      // Check messageId: must be a stringified Uuid.
      string messageId(getProperty("MessageId"));
      istringstream iss(messageId);
      qpid::types::Uuid uuid;
      iss >> uuid;
      if (uuid.isNull()) {
        THROW (InvalidMessage, "Message contains invalid ID: " << messageId);
      }

    }

  }
}
