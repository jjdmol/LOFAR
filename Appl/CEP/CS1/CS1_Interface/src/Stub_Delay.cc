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

#include <lofar_config.h>

#include <CS1_Interface/Stub_Delay.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <APS/ParameterSet.h>

namespace LOFAR { 

  INIT_TRACER_CONTEXT(Stub_Delay, LOFARLOGGER_PACKAGE);
  
  Stub_Delay::Stub_Delay(bool isInput, const ACC::APS::ParameterSet &pSet)
    : itsIsInput    (isInput),
      itsPS         (pSet),
      itsTHs        (0),
      itsConnections(0)
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    itsNRSP = itsPS.getUint32("Input.NRSPBoards");
    LOG_TRACE_VAR_STR("Input.NRSPBoards = " << itsNRSP);

    itsPorts = itsPS.getUint16Vector("Connections.Input_Delay.Ports");
    ASSERTSTR(itsPorts.size() >= itsNRSP, 
              itsPorts.size() << " >= " << itsNRSP);

    itsTHs = new TH_Socket*[itsNRSP];
    itsConnections = new Connection*[itsNRSP];
  
    for (uint i=0; i<itsNRSP; i++) {
      itsTHs[i] = 0;
      itsConnections[i] = 0;
    }
  
  }
  
  Stub_Delay::~Stub_Delay()
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    for (uint i=0; i<itsNRSP; i++)
    {
      delete itsTHs[i];
      delete itsConnections[i];
    }
    delete [] itsTHs;
    delete [] itsConnections;
  }
  
  void Stub_Delay::connect(uint RSP_nr, TinyDataManager &dm, uint dhNr)
  {
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    ASSERTSTR(RSP_nr < itsNRSP, RSP_nr << " < " << itsNRSP);
    string service = toString(itsPorts[RSP_nr]);

    if (itsIsInput) // on the input side, start client socket
    {
      ASSERTSTR(itsTHs[RSP_nr] == 0, "Stub input " << RSP_nr << 
                " has already been connected.");
      // Create a client socket
      string server = itsPS.getString("Connections.Input_Delay.ServerHost");
      string service = toString(itsPorts[RSP_nr]);
      itsTHs[RSP_nr] = new TH_Socket(server,
                                     service,
                                     true,
                                     Socket::TCP,
                                     false);

      itsConnections[RSP_nr] = new Connection("toDelayCompensation", 0, 
                                              dm.getGeneralInHolder(dhNr),
                                              itsTHs[RSP_nr], true);
      dm.setInConnection(dhNr, itsConnections[RSP_nr]);
    } 
    else    // on the delay compensation side, start a server socket
    {
      ASSERTSTR(itsTHs[RSP_nr] == 0, "Stub output " << RSP_nr << 
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
  }

} //namespace

