//#  BDBSite.cc: Class that contains information about another Berkeley site
//#
//#  Copyright (C) 2002-2005
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

// General includes
#include <Common/LofarLogger.h>
#include <iostream>

#include <MNS/BDBSite.h>

// Application specific includes

using namespace LOFAR;
using namespace std;

BDBSite::BDBSite(const char* hostName, const int port, Socket* socket)
  :itsHostName(hostName), 
   itsPort(port),
   itsSocket(socket)
{
  itsConnectionDataBuffer = new char[sizeof(int) + 2 + itsHostName.size()];
  memset(itsConnectionDataBuffer, 0, sizeof(int) + 2 + itsHostName.size());
  memcpy(itsConnectionDataBuffer, &itsPort, sizeof(int));
  memcpy(itsConnectionDataBuffer + sizeof(int), itsHostName.c_str(), itsHostName.size());

  itsConnectionData.set_data(itsConnectionDataBuffer);
  itsConnectionData.set_size(sizeof(int) + 2 + itsHostName.size());
};

BDBSite:: ~BDBSite()
{
  delete [] itsConnectionDataBuffer;
  delete itsSocket;
}

bool BDBSite::operator==(BDBSite& other)
{
  return ((itsPort==other.itsPort) && (itsHostName == other.itsHostName));
}

ostream& operator<<(ostream& os, BDBSite& site)
{
  return os<<site.itsHostName<<":"<<site.itsPort;
}
