//#  tWH_DelayCompensation.cc: test program for WH_DelayCompensation class
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_DelayCompensation/WH_DelayCompensation.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace LOFAR::CS1;

int main (int argc, char* argv[]) {

  INIT_LOGGER (argv[0]);

  // Check invocation syntax
  if (argc < 1) {
    LOG_FATAL_STR ("Usage: " << argv[0]);
    return 1;
  }

  // Tell operator we are trying to start up.
  LOG_INFO_STR("Starting up: " << argv[0]);

  try {
    //    ... do work ...
    ParameterSet pset("CS1.cfg");
    WH_DelayCompensation wh("WH_DelayCompensation", pset);
    wh.basePreprocess();
    wh.baseProcess();
    wh.basePostprocess();
    LOG_INFO_STR("Shutting down: " << argv[0]);
  }
  catch (LOFAR::Exception& e) {
    LOG_FATAL_STR(e);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    return 1;
  }

  LOG_INFO_STR(argv[0] << " terminated normally");
  return (0);

}
