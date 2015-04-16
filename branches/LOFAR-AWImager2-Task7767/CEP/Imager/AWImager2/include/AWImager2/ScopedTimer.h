//# ATerm.h: Compute the LOFAR beam response on the sky.
//#
//# Copyright (C) 2011
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
//# $Id: $

#ifndef LOFAR_LOFARFT_SCOPEDTIMER_H
#define LOFAR_LOFARFT_SCOPEDTIMER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/PrecTimer.h>
#include <map>

namespace LOFAR {
namespace LofarFT {


class ScopedTimer
{
public:
  
  ScopedTimer(casa::String id);
  
  virtual ~ScopedTimer();
  
  static void show();
  
  
private:

  casa::PrecTimer itsPrecTimer;
  
  casa::Double* itsTime;
  
  static std::map<casa::String, casa::Double>& timings();
  
};

  
} // namespace LofarFT
} // namespace LOFAR

#endif

