//# Exceptions.h: Exception classes used by the Messaging package
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
//# $Id: Exceptions.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_EXCEPTIONS_H
#define LOFAR_MESSAGING_EXCEPTIONS_H

// @file
// Exception classes used by the Messaging package

#include <Common/Exception.h>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    EXCEPTION_CLASS(Exception, LOFAR::Exception);

    // Exception class used when an invalid message is received.
    EXCEPTION_CLASS(InvalidMessage, Exception);

    // Exception class used when a message property cannot be found.
    EXCEPTION_CLASS(MessagePropertyNotFound, Exception);

    // Exception class used when a message for the wrong system is received.
    // EXCEPTION_CLASS(WrongSystemName, Exception);

    // Exception class used when a message of unknown type is received.
    EXCEPTION_CLASS(UnknownMessageType, Exception);

    // Exception class used when errors occur while sending or receiving
    // messages.
    EXCEPTION_CLASS(MessagingException, Exception);

    // @}
  }
}

#endif
