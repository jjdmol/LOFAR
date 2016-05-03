//# Util.h: Helper functions for QPID
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

#ifndef LOFAR_MESSAGEBUS_UTIL_H
#define LOFAR_MESSAGEBUS_UTIL_H

#ifdef HAVE_QPID
#include <qpid/messaging/Duration.h>
#else
#include <MessageBus/NoQpidFallback.h>
#endif

#include <string>

namespace LOFAR {

  // Convert a duration in seconds to a QPID duration object
  qpid::messaging::Duration TimeOutDuration(double secs);

  // Return the prefix that has to be prepended to all queue names
  std::string queue_prefix();

} // namespace LOFAR

#endif

