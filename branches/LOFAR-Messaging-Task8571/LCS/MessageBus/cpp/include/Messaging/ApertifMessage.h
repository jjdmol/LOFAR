//# ApertifMessage.h: Top-level message class for APERTIF messages.
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
//# $Id: ApertifMessage.h 1483 2015-08-23 19:19:44Z loose $

#ifndef APERTIF_MESSAGING_APERTIFMESSAGE_H
#define APERTIF_MESSAGING_APERTIFMESSAGE_H

// @file
// Top-level message class for APERTIF messages.

#include <Messaging/Message.h>
#include <Common/ObjectFactory.h>
#include <string>

namespace APERTIF
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // Top-level message class for APERTIF messages.
    // We may not need this class, but I've put it in just to be sure.
    class ApertifMessage : public Message
    {
    protected:
      ApertifMessage();
      ApertifMessage(const qpid::messaging::Message& msg);
    };
    // @}

  }
}

#endif
