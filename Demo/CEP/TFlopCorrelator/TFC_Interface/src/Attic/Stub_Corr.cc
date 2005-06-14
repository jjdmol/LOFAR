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
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  Stub_Corr::Stub_Corr (bool stubOnServer)
    : itsStubOnServer (stubOnServer)
  {
    itsPS = new ACC::ParameterSet("TFlopCorrelator.cfg");
    itsNCorr = itsPS->getInt("NSBF") * itsPS->getInt("Corr_per_Filter");
    LOG_TRACE_FLOW_STR("Total number of Correlators in the Stub_Corr is " << itsNCorr);
    
    for (int i=0; i<itsNCorr; i++) {
      //      itsCorr.push_back(new DH_Vis());
    }
    
    //todo: Dump/storage side
  }
  
  Stub_Corr::~Stub_Corr()
  {}
  
  void Stub_Corr::connect (int C_nr,
			  DH_Vis& sb)
  {
    DBGASSERTSTR(((C_nr >= 0) && (C_nr < itsCorr.size())),
		  "C_nr argument out of boundaries; " << C_nr << " / " << itsCorr.size());

    TH_Socket thSB(itsPS->getString("CorrConnection.ClientHost"), // sendhost
		   itsPS->getString("CorrConnection.ServerHost"), // recvhost
		   itsPS->getInt("CorrConnection.RequestPort"),   // port
		   true
		   );
    if (itsStubOnServer) {
      //      itsSB[C_nr].connectTo (sb, thSB);
    } else {
      //      sb.connectTo (itsSB[C_nr], thSB);
    }
  };

} //namespace

