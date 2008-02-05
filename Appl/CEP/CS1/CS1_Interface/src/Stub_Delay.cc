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
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>

namespace LOFAR 
{ 
  namespace CS1
  {

    INIT_TRACER_CONTEXT(Stub_Delay, LOFARLOGGER_PACKAGE);
  
    Stub_Delay::Stub_Delay(bool isInput, const CS1_Parset *pSet)
      : itsIsInput    (isInput),
        itsCS1PS      (pSet),
        itsTHs        (0),
        itsConnections(0)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      itsNRSP = itsCS1PS->getUint32("OLAP.nrRSPboards");
      LOG_TRACE_VAR_STR("OLAP.nrRSPboards = " << itsNRSP);

      itsPorts = itsCS1PS->delay_Ports();
      ASSERTSTR(itsPorts.size() >= itsNRSP, 
                itsPorts.size() << " >= " << itsNRSP);

      itsTHs = new TransportHolder*[itsNRSP];
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
      string service = itsPorts[RSP_nr];


      // Determine tranport type
      string transportType = itsCS1PS->getString("OLAP.OLAP_Conn.input_DelayComp_Transport");

      TransportHolder *th;
      if (transportType == "TCP") {

	 // on the input side, start client socket
	string service = itsPorts[RSP_nr];
	if (itsIsInput) { 
	  // Create a client socket
	  ASSERTSTR(itsTHs[RSP_nr] == 0, "Stub input " << RSP_nr << " has already been connected.");
	  
	  string server = itsCS1PS->getString("OLAP.DelayComp.hostname");
	  th = new TH_Socket(server, service, true, Socket::TCP, false);

	} else {    // on the delay compensation side, start a server socket
	  ASSERTSTR(itsTHs[RSP_nr] == 0, "Stub output " << RSP_nr << " has already been connected.");
	  // Create a server socket
	  th = new TH_Socket(service, true, Socket::TCP, 5, false);
	}
      } else if (transportType == "NULL") { 
	th = new TH_Null();
      }
      
      itsTHs[RSP_nr] = th;
      if (itsIsInput) {
	itsConnections[RSP_nr] = new Connection("toDelayCompensation", 0, dm.getGeneralInHolder(dhNr), th, true);
	dm.setInConnection(dhNr, itsConnections[RSP_nr]);
      } else {
	itsConnections[RSP_nr] = new Connection("fromInpSection", dm.getGeneralOutHolder(dhNr), 0, th, true);
	dm.setOutConnection(dhNr, itsConnections[RSP_nr]);
      }
    }
  } // namespace CS1
} //namespace LOFAR

