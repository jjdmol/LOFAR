//# Stub_RSP.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#include <TFC_Interface/Stub_RSP.h>
#include <Transport/TH_Socket.h>


using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

  Stub_RSP::Stub_RSP (bool stubOnServer)
    : itsStubOnServer (stubOnServer)
  {
    itsPS = new ACC::APS::ParameterSet("TFlopCorrelator.cfg");
    
    // todo: add DH_?? for pre-correlation correction factors 
    //    for (int i=0; i<itsNSBF; i++) {
      //      itsSB.push_back(new DH_SubBand("noname",1)); //todo: get correct SubbandID
    //    }
  }

  Stub_RSP::~Stub_RSP()
  {}

  void Stub_RSP::connect ()
  {
    const ParameterSet myPS("TFlopCorrelator.cfg");
  };

  //todo: add connections for pre-correlation correction DH_?? 

} //namespace

