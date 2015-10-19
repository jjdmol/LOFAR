//# Helpers.h: Collection of helper functions.
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
//# $Id: Helpers.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_HELPERS_H
#define LOFAR_MESSAGING_HELPERS_H

// @file
// Collection of helper functions.

#include <qpid/messaging/Duration.h>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // Convert a time-out in seconds into a Duration object.
    // If \a secs is negative, the time-out will be infinite.
    qpid::messaging::Duration TimeOutDuration(double secs);

    // @}
  }
}

#endif
