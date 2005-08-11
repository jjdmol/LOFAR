//# Stub_FIR.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#include <TFC_Interface/Stub_FIR.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>


using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

  Stub_FIR::Stub_FIR (bool stubOnServer, const ACC::APS::ParameterSet pSet)
    : itsStubOnServer (stubOnServer),
      itsPS           (pSet),
      itsTHs          (0),
      itsConnections  (0)
  {
    itsNFIRF     = itsPS.getInt32("Input.NSubbands");  // number of SubBand filters in the application
    ASSERTSTR(itsNFIRF >= 0, "Number of subband filters must be greater than 0");
    itsTHs = new TH_Socket*[itsNFIRF];
    itsConnections = new Connection*[itsNFIRF];

    for (int i=0; i<itsNFIRF; i++) {
      itsTHs[i] = 0;
      itsConnections[i] = 0;
    }

  }

  Stub_FIR::~Stub_FIR()
  {
    for (int i=0; i<itsNFIRF; i++)
    {
      delete itsTHs[i];
      delete itsConnections[i];
    }
    delete [] itsTHs;
    delete [] itsConnections;
  }

  void Stub_FIR::connect (int FIRF_nr,
			 TinyDataManager& dm,
			 int dhNr)
  {
    DBGASSERTSTR(((FIRF_nr >= 0) && (FIRF_nr < itsNFIRF)),
		 "FIRF_nr argument out of boundaries; " << FIRF_nr 
		 << " / " << itsNFIRF);

    int port = itsPS.getInt32("FIRConnection.RequestPort");
    string service(formatString("%d", port+FIRF_nr));
    if (itsStubOnServer)    // On the cluster side, so start a server socket
    {
      LOG_TRACE_FLOW_STR("Server Stub_FIR initting on port " << service);
      DBGASSERTSTR(itsTHs[FIRF_nr] == 0, "Stub input " << FIRF_nr << 
		" has already been connected.");
      // Create a server socket
      itsTHs[FIRF_nr] = new TH_Socket(service);
      itsConnections[FIRF_nr] = new Connection("toBG", 
					       dm.getGeneralOutHolder(dhNr), 
					      0, itsTHs[FIRF_nr], true);
      dm.setOutConnection(dhNr, itsConnections[FIRF_nr]);
    }
    else             // On BG side , so start a client socket
    {
      DBGASSERTSTR(itsTHs[FIRF_nr] == 0, "Stub output " << FIRF_nr << 
		" has already been connected.");
      // Create a client socket
      itsTHs[FIRF_nr] = new TH_Socket(itsPS.getString("FIRConnection.ServerHost"),
				      service);
      itsConnections[FIRF_nr] = new Connection("fromInpSection", 0, 
					       dm.getGeneralInHolder(dhNr), 
					      itsTHs[FIRF_nr], true);
      dm.setInConnection(dhNr, itsConnections[FIRF_nr]);
      
    }

  };

  //todo: add connections for pre-correlation correction DH_?? 

} //namespace

