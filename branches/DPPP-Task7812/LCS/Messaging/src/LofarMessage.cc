//# LofarMessage.cc: Top-level message class for LOFAR messages.
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR Software Suite.
//#
//# The LOFAR Software Suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or (at your
//# option) any later version.
//#
//# The LOFAR Software Suite is distributed in the hope that it will be
//# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along with
//# The LOFAR Software Suite.  If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: LofarMessage.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>
#include <Messaging/LofarMessage.h>
#include <Messaging/Exceptions.h>

#include <sstream>

namespace LOFAR
{
  namespace Messaging
  {
    using namespace std;

    //## ----  Definition of protected methods  ---- ##//

    LofarMessage::LofarMessage() :
      Message()
    {
      setProperty("SystemName", "LOFAR");
      setProperty("MessageId", qpid::types::Uuid(true).str());
    }


    LofarMessage::LofarMessage(const qpid::messaging::Message& qmsg) :
      Message(qmsg)
    {
      // Check systemName: must be "LOFAR"
      string systemName(getProperty("SystemName"));
      if (systemName != "LOFAR") {
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
