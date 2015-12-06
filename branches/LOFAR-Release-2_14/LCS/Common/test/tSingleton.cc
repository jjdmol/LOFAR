//# tSingleton.cc: Test program for the Meyers singleton class.
//#
//# Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/Singleton.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  template<typename T>
  class Unique
  {
  private:
    friend class Singleton<Unique>;
    Unique() { LOG_TRACE_FLOW(AUTO_FUNCTION_NAME); }
    ~Unique() { LOG_TRACE_FLOW(AUTO_FUNCTION_NAME); }
  };
}

using namespace LOFAR;

int main()
{
  INIT_LOGGER("tSingleton");
  LOG_INFO("Starting up...");

  Unique<int>& ui1    = Singleton< Unique<int> >::instance();
  Unique<int>& ui2    = Singleton< Unique<int> >::instance();
  Unique<double>& ud1 = Singleton< Unique<double> >::instance();
  Unique<double>& ud2 = Singleton< Unique<double> >::instance();

  ASSERT((void*)&ui1 == (void*)&ui2);
  ASSERT((void*)&ud1 == (void*)&ud2);
  ASSERT((void*)&ui1 != (void*)&ud1);
  ASSERT((void*)&ui2 != (void*)&ud2);

  LOG_INFO("Program terminated successfully");
  return 0;
}
