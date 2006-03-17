//# Stub_Delay.cc: Stub for connection from 
//#
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


#include <CS1_Interface/Stub_Delay.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;

namespace LOFAR { 

Stub_Delay::Stub_Delay(bool isInput, const ACC::APS::ParameterSet &pSet)
  : itsIsInput    (isInput),
    itsPS         (pSet),
    itsTHs        (0),
    itsConnections(0)
{
  itsNRSP = itsPS.getInt32("Data.NStations");
  LOG_TRACE_FLOW_STR("Total number of RSPinputs in the Stub_Delay is " << itsNRSP);
  ASSERTSTR(itsNRSP >= 0, "Number of RSPinputs in the Stub_Delay must be greater than 0");

  itsTHs = new TH_Socket*[itsNRSP];
  itsConnections = new Connection*[itsNRSP];
  
  for (int i=0; i<itsNRSP; i++) {
    itsTHs[i] = 0;
    itsConnections[i] = 0;
  }
  
}
  
Stub_Delay::~Stub_Delay()
{
  for (int i=0; i<itsNRSP; i++)
  {
    delete itsTHs[i];
    delete itsConnections[i];
  }
  delete [] itsTHs;
  delete [] itsConnections;
}
  
void Stub_Delay::connect(int RSP_nr, TinyDataManager &dm, int dhNr)
{
#if 0
  DBGASSERTSTR(((RSP_nr >= 0) && (RSP_nr < itsNRSP)),
 	       "RSP_nr argument out of boundaries; " << RSP_nr << " / " << itsNRSP);

  string service = itsPS.getStringVector("Connections.Input_Delay.Ports")[RSP_nr];

  if (itsIsInput) // on the input side, start client socket
  {
    DBGASSERTSTR(itsTHs[RSP_nr] == 0, "Stub input " << RSP_nr << 
		 " has already been connected.");
    // Create a client socket
    string server = itsPS.getString("Connections.Input_Delay.ServerHost");
    itsTHs[RSP_nr] = new TH_Socket(server,
				   service,
				   true,
				   Socket::TCP,
				   false);

    itsConnections[RSP_nr] = new Connection("toBG", 0, 
					    dm.getGeneralInHolder(dhNr),
					    itsTHs[RSP_nr], true);
    dm.setInConnection(dhNr, itsConnections[RSP_nr]);
  } 
  else    // on the delay compensation side, start a server socket
  {
    DBGASSERTSTR(itsTHs[RSP_nr] == 0, "Stub output " << RSP_nr << 
		 " has already been connected.");
    // Create a server socket
    itsTHs[RSP_nr] = new TH_Socket(service,
				   true,
				   Socket::TCP,
				   5,
				   false);
    itsConnections[RSP_nr] = new Connection("fromInpSection", 
					    dm.getGeneralOutHolder(dhNr), 
					    0, itsTHs[RSP_nr], true);
    dm.setOutConnection(dhNr, itsConnections[RSP_nr]);

  }
#endif  
};

} //namespace

