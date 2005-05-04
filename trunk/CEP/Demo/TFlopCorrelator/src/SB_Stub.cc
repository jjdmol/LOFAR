//# SB_Stub.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#include <TFlopCorrelator/SB_Stub.h>
#include <Transport/TH_Socket.h>


using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  SB_Stub::SB_Stub (bool stubOnServer)
    : itsStubOnServer (stubOnServer)
  {
    itsPS = new ACC::ParameterSet("TFlopCorrelator.cfg");
    itsNSBF     = itsPS->getInt("NSBF");  // number of SubBand filters in the application
    
    for (int i=0; i<itsNSBF; i++) {
      itsSB.push_back(new DH_SubBand("noname",1)); //todo: get correct SubbandID
    }
  }

  SB_Stub::~SB_Stub()
  {}

  void SB_Stub::connect (int& SBF_nr,
			 DH_SubBand& sb)
  {
    const ParameterSet myPS("TFlopCorrelator.cfg");
    TH_Socket thSB(myPS.getString("SBConnection.ClientHost"), // sendhost
		   myPS.getString("SBConnection.ServerHost"),   // recvhost
		   myPS.getInt("SBConnection.RequestPort"),   // port
		   true
		   );
    itsSB[SBF_nr]->setID(itsPS->getInt("SBConnection.ID"));
    if (itsStubOnServer) {
      itsSB[SBF_nr]->connectTo (sb, thSB);
    } else {
      sb.connectTo (*(itsSB[SBF_nr]), thSB);
    }
  };

} //namespace

