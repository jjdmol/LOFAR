//#  AMCServer.cc: Astronomical Measures Conversions server.
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
//#  $Id: amcserver.cc 7785 2006-03-13 16:10:05Z loose $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <AMCImpl/ConverterServer.h>
#include <cstdlib>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int argc, char* argv[])
{
  // Init logsystem
  ConfigLocator	aCL;
  string	logpropFile(basename(argv[0]));
  logpropFile.append(".log_prop");
  INIT_VAR_LOGGER(aCL.locate(logpropFile).c_str(), "AMCServer");
  LOG_INFO_STR ("Initialized logsystem with: " << aCL.locate(logpropFile));

  // Report ourself to the LogggingProcessor
  LOG_INFO("MACProcessScope: LOFAR_PermSW_AMCServer");

  // Listen port (default: 31337)
  uint16 port = argc > 1 ? atoi(argv[1]) : 31337;

  LOG_INFO_STR("Starting AMCServer using port " << port);
  try {
    // Create a converter server, listening on port \a port.
    ConverterServer server(port);

    // Run the server; this will start the event loop 
    server.run();
  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("AMCServer terminated");
  return 0;
}
