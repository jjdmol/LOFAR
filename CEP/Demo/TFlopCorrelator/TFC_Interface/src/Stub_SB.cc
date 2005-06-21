//# Stub_SB.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#include <TFC_Interface/Stub_SB.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>


using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  Stub_SB::Stub_SB (bool stubOnServer)
    : itsStubOnServer (stubOnServer),
      itsTHs          (0),
      itsConnections  (0)
  {
    itsPS = new ACC::ParameterSet("TFlopCorrelator.cfg");
    itsNSBF     = itsPS->getInt("NSBF");  // number of SubBand filters in the application
    ASSERTSTR(itsNSBF >= 0, "Number of subband filters must be greater than 0");
    itsTHs = new TH_Socket*[itsNSBF];
    itsConnections = new Connection*[itsNSBF];

    for (int i=0; i<itsNSBF; i++) {
      itsTHs[i] = 0;
      itsConnections[i] = 0;
    }

  }

  Stub_SB::~Stub_SB()
  {
    for (int i=0; i<itsNSBF; i++)
    {
      delete itsTHs[i];
      delete itsConnections[i];
    }
    delete [] itsTHs;
    delete [] itsConnections;
  }

  void Stub_SB::connect (int SBF_nr,
			 TinyDataManager& dm,
			 int dhNr)
  {
    DBGASSERTSTR(SBF_nr <= itsNSBF, "Subband filter number too large");
    const ParameterSet myPS("TFlopCorrelator.cfg");
    int port = myPS.getInt("SBConnection.RequestPort");
    string service(formatString("%d", port));
    if (itsStubOnServer)    // On the cluster side, so start a server socket
    {
      DBGASSERTSTR(itsTHs[SBF_nr] == 0, "Stub input " << SBF_nr << 
		" has already been connected.");
      // Create a server socket
      itsTHs[SBF_nr] = new TH_Socket(service);
      itsConnections[SBF_nr] = new Connection("toBG", dm.getOutHolder(dhNr), 
					      0, itsTHs[SBF_nr], true);
      dm.setOutConnection(dhNr, itsConnections[SBF_nr]);
    }
    else             // On BG side , so start a client socket
    {
      DBGASSERTSTR(itsTHs[SBF_nr] == 0, "Stub output " << SBF_nr << 
		" has already been connected.");
      // Create a client socket
      itsTHs[SBF_nr] = new TH_Socket(myPS.getString("SBConnection.ServerHost"),
				     service);
      itsConnections[SBF_nr] = new Connection("fromInpSection", 0, dm.getInHolder(dhNr), 
					      itsTHs[SBF_nr], true);
      dm.setInConnection(dhNr, itsConnections[SBF_nr]);
    }

  };

  //todo: add connections for pre-correlation correction DH_?? 

} //namespace

