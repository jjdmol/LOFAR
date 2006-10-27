//#  BBSControl.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <libgen.h>
#include <BBSControl/BBSProcessControl.h>
#include <PLC/ACCmain.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::PLC;

int main(int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);

  LOG_INFO_STR(progName << " is starting up ...");
  try {
    BBSProcessControl myProcess;
    if (!ACCmain(argc, argv, &myProcess)) {
      LOG_ERROR("ACCmain returned with an error status");
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
