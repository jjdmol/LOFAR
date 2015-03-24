//# ScopedTimer.cc:
//#
//# Copyright (C) 2014
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

#include <lofar_config.h>

#include <LofarFT/ScopedTimer.h>

#include <Common/OpenMP.h>

namespace LOFAR {
namespace LofarFT {
  
ScopedTimer::ScopedTimer(casa::String id)
{
  #pragma omp critical(ScopedTimer)
  itsTime = &ScopedTimer::timings()[id];
  itsPrecTimer.start();
}

ScopedTimer::~ScopedTimer()
{
  itsPrecTimer.stop();
  casa::Double t = itsPrecTimer.getReal();
  #pragma omp atomic
  *itsTime += t;
}

std::map<casa::String, casa::Double>& ScopedTimer::timings()
{
  static std::map<casa::String, casa::Double> t;
  return t;
}

void ScopedTimer::show()
{
  // show content:
  for (std::map<casa::String,casa::Double>::iterator it=timings().begin(); it!=timings().end(); ++it)
    std::cout << it->first << " => " << it->second << '\n';
}

  
} // namespace LofarFT
} // namespace LOFAR


