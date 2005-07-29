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

  Stub_Corr::Stub_Corr (bool stubOnServer, const ACC::APS::ParameterSet pSet)
    : itsStubOnServer (stubOnServer),
      itsTHs          (0),
      itsConnections  (0),
      itsPS           (pSet)
  {
    itsNChan = itsPS.getInt32("Input.NSubbands");
    LOG_TRACE_FLOW_STR("Total number of channels in the Stub_Corr is " << itsNChan);
    ASSERTSTR(itsNChan >= 0, "Number of channels in the Stub_Corr must be greater than 0");

    itsTHs = new TH_Socket*[itsNChan];
    itsConnections = new Connection*[itsNChan];

    for (int i=0; i<itsNChan; i++) {
      itsTHs[i] = 0;
      itsConnections[i] = 0;
    }
    
  }
  
  Stub_Corr::~Stub_Corr()
  {
    for (int i=0; i<itsNChan; i++)
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
    DBGASSERTSTR(((C_nr >= 0) && (C_nr < itsNChan)),
		  "C_nr argument out of boundaries; " << C_nr << " / " << itsNChan);

    int port = itsPS.getInt32("CorrConnection.RequestPort");
    string service(formatString("%d", port+C_nr));

    if (itsStubOnServer) // on the cluster side, so start server socket
    {
     DBGASSERTSTR(itsTHs[C_nr] == 0, "Stub output " << C_nr << 
                " has already been connected.");
      // Create a server socket
      itsTHs[C_nr] = new TH_Socket(service);
      itsConnections[C_nr] = new Connection("fromInpSection", 0, 
					    dm.getGeneralInHolder(dhNr), 
					    itsTHs[C_nr], true);
      dm.setInConnection(dhNr, itsConnections[C_nr]);
    } 
    else                 // on BG side, so start client socket
    {
      DBGASSERTSTR(itsTHs[C_nr] == 0, "Stub input " << C_nr << 
		" has already been connected.");
      // Create a client socket
      itsTHs[C_nr] = new TH_Socket(itsPS.getString("CorrConnection.ServerHost"),
				   service);
      itsConnections[C_nr] = new Connection("toBG", 
					    dm.getGeneralOutHolder(dhNr), 
					    0, itsTHs[C_nr], true);
      dm.setOutConnection(dhNr, itsConnections[C_nr]);
    }

  };

} //namespace

