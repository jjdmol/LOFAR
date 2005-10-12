//# Stub_Delay.cc: Stub for connection from 
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$
//#
////////////////////////////////////////////////////////////////////

#include <TFC_Interface/Stub_Delay.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

Stub_Delay::Stub_Delay (bool isInput, const ACC::APS::ParameterSet pSet)
  : itsIsInput      (isInput),
    itsPS           (pSet),
    itsTHs          (0),
    itsConnections  (0)
{
  itsNRSP = itsPS.getInt32("Input.NRSP");
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
  
void Stub_Delay ::connect (int RSP_nr,
			   TinyDataManager& dm,
			   int dhNr)
{
  DBGASSERTSTR(((RSP_nr >= 0) && (RSP_nr < itsNRSP)),
 	       "RSP_nr argument out of boundaries; " << RSP_nr << " / " << itsNRSP);

  string service = itsPS.getStringVector("DelayConnection.RequestPorts")[RSP_nr];

  if (itsIsInput) // on the input side, start client socket
  {
    DBGASSERTSTR(itsTHs[RSP_nr] == 0, "Stub input " << RSP_nr << 
		 " has already been connected.");
    // Create a client socket
    string server = itsPS.getString("DelayCompensation.ServerHost");
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
  
};

} //namespace

