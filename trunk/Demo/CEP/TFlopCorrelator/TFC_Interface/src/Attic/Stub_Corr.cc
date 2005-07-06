//# Stub_Corr.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$
//#
////////////////////////////////////////////////////////////////////

#include <TFC_Interface/Stub_Corr.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

  Stub_Corr::Stub_Corr (bool stubOnServer)
    : itsStubOnServer (stubOnServer),
      itsTHs          (0),
      itsConnections  (0)
  {
    itsPS = new ACC::APS::ParameterSet("TFlopCorrelator.cfg");
    itsNCorr = itsPS->getInt32("NSBF")/2;
    DBGASSERTSTR(NSBF%2 == 0, "NSBF should be an even number");
    LOG_TRACE_FLOW_STR("Total number of Correlators in the Stub_Corr is " << itsNCorr);
    ASSERTSTR(itsNCorr >= 0, "Number of Correlators in the Stub_Corr must be greater than 0");

    for (int i=0; i<itsNCorr; i++) {
      itsTHs[i] = 0;
      itsConnections[i] = 0;
    }
    
  }
  
  Stub_Corr::~Stub_Corr()
  {
    for (int i=0; i<itsNCorr; i++)
    {
      delete itsTHs[i];
      delete itsConnections[i];
    }
    delete [] itsTHs;
    delete [] itsConnections;
  }
  
  void Stub_Corr::connect (int C_nr,
			   TinyDataManager& dm,
			   int dhNr)
  {
    DBGASSERTSTR(((C_nr >= 0) && (C_nr < itsNCorr)),
		  "C_nr argument out of boundaries; " << C_nr << " / " << itsNCorr);

    int port = itsPS->getInt32("CorrConnection.RequestPort");
    string service(formatString("%d", port));

    if (itsStubOnServer) // on the cluster side, so start server socket
    {
     DBGASSERTSTR(itsTHs[C_nr] == 0, "Stub output " << C_nr << 
                " has already been connected.");
      // Create a server socket
      itsTHs[C_nr] = new TH_Socket(service);
      itsConnections[C_nr] = new Connection("fromInpSection", 0, 
					    dm.getInHolder(dhNr), 
					    itsTHs[C_nr], true);
      dm.setInConnection(dhNr, itsConnections[C_nr]);
    } 
    else                 // on BG side, so start client socket
    {
      DBGASSERTSTR(itsTHs[C_nr] == 0, "Stub input " << C_nr << 
		" has already been connected.");
      // Create a client socket
      itsTHs[C_nr] = new TH_Socket(itsPS->getString("CorrConnection.ServerHost"),
				   service);
      itsConnections[C_nr] = new Connection("toBG", dm.getOutHolder(dhNr), 
					    0, itsTHs[C_nr], true);
      dm.setOutConnection(dhNr, itsConnections[C_nr]);
    }

  };

} //namespace

