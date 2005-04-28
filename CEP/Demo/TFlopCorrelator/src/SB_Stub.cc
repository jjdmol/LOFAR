//# SB_Stub.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#include <TFlopCorrelator/SB_Stub.h>
#include <Transport/TH_Socket.h>
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  SB_Stub::SB_Stub (bool stubOnServer)
    : itsStubOnServer (stubOnServer),
      itsSB("noname",1) // todo: bring in correct SBID!!
  {}

  SB_Stub::~SB_Stub()
  {}

  void SB_Stub::connect (DH_SubBand& sb)
  {
    const ParameterSet myPS("TFlopCorrelator.cfg");
    TH_Socket thSB(myPS.getString("SBConnection.ClientHost"), // sendhost
		   myPS.getString("SBConnection.ServerHost"),   // recvhost
		   myPS.getInt("SBConnection.RequestPort"),   // port
		   true
		   );
    itsSB.setID(myPS.getInt("SBConnection.ID"));
    if (itsStubOnServer) {
      itsSB.connectTo (sb, thSB);
    } else {
      sb.connectTo (itsSB, thSB);
    }
  };

} //namespace

