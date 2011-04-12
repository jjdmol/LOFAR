//# GlobalControl.cc: 
//#
//# Copyright (C) 2002-2007
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

#include <lofar_config.h>
#include <libgen.h>
#include <BBSControl/GlobalProcessControl.h>
#include <BBSControl/Package__Version.h>
#include <PLC/ACCmain.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::PLC;

int main(int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);

  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));
  try {
    GlobalProcessControl myProcess;
    int result = ACCmain(argc, argv, &myProcess);
    if (result != 0) {
      LOG_ERROR_STR("ACCmain returned with error status: " << result);
      return 1;
    }
  } 
  catch(Exception& e) {
    LOG_FATAL_STR(progName << " terminated due to fatal exception!\n" << e);
    return 1;
  }
  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}
