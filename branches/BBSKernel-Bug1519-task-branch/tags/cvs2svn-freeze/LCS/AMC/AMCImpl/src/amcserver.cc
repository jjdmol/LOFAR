//#  amcserver.cc: Astronomical Measures Conversions server.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <AMCImpl/ConverterServer.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int argc, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  // Listen port (default: 31337)
  uint16 port = argc > 1 ? atoi(argv[1]) : 31337;

  LOG_INFO_STR("Starting amcserver using port " << port);
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
  LOG_INFO("amcserver terminated");
  return 0;
}
