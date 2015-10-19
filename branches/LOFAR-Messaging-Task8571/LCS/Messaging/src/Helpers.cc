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
//# $Id: Helpers.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>
#include "Helpers.h"

namespace LOFAR
{
  namespace Messaging
  {
    using namespace qpid::messaging;

    Duration TimeOutDuration(double secs)
    {
      if (secs < 0.0) return Duration::FOREVER;
      else return Duration(Duration::SECOND.getMilliseconds() * secs);
    }

  }
}
