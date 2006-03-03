//#  tConverterStress.cc: test program for the AMC converter classes.
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
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <AMCBase/AMCClient/ConverterClient.h>
#include <AMCImpl/ConverterImpl.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/lofar_fstream.h>
#include <memory>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::AMC;

Converter* createConverter()
{
  ifstream ifs("tConverterStress.in");
  if (!ifs) {
    LOG_ERROR("Failed to open file \"tConverterStress.in\" for reading");
    return 0;
  }

  string type;
  string host("localhost");
  uint16 port(31337);

  ifs >> type >> host >> port;

  if (type == "impl" || type == "IMPL" || type == "Impl") 
    return new ConverterImpl();
  if (type == "client" || type == "CLIENT" || type == "Client")
    return new ConverterClient(host, port);

  LOG_ERROR_STR("Invalid converter type (" << type << ") specified");
  return 0;
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  uint fails(0);
  uint runs(100);

  SkyCoord sky(1, 0.3);  // arbitrary source direction
  EarthCoord earth(0.111646531, 0.921760253, 25, EarthCoord::WGS84); // DWL
  TimeCoord time(2004, 11, 19, 15, 22, 39);  // arbitrary time
      
  for (uint i = 0; i < runs; i++) {
    try {
      LOG_INFO_STR("Client #" << i);
      auto_ptr<Converter> conv(createConverter());
      SkyCoord result = conv->j2000ToAzel (sky, earth, time);
    }
    catch (Exception& e) {
      LOG_ERROR_STR(e);
      fails++;
    }
  }

  if (fails != 0) cout << fails << " failures in " << runs << " runs." << endl;
  else cout << "All " << runs << " runs succeeded." << endl;

  return 1;
  
}
