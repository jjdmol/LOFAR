//# FrontEndStub.cc: Stub for connection between FrontEnd and Correlator
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//# $Id$

#include <DFTServer/FrontEndStub.h>
#include <Transport/TH_Socket.h>
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  FrontEndStub::FrontEndStub (bool FE_side,
		    int NoCorrelators,
		    const std::string& parameterFileName)
    : itsFE_side (FE_side),
      itsNoCorr  (NoCorrelators),
      itsParmFileName (parameterFileName),
    
  {
    LOG_TRACE_OBJ_STR("FrontEndStub(" << FE_side << ", " << NoCorrelators 
		      << ", " << parameterFileName << ")");
    for (int i=0; i<NoCorrelators; i++) {
      itsDummyDH[i] = new DH_CorrCube(string("dummyCorrCube_") + str,
				      itsNelements, 
				      itsNsamples, 
				      itsNchannels);
    }
  }

  FrontEndStub::~FrontEndStub()
  {}

  void FrontEndStub::connect (DH_CorrCube& aDH,
			 int corrNo)
  {
    DBGASSERTSTR((corrNo>=0) && (corrNo<itsNoCorr),"corrNo out of Range");
    DBGASSERTSTR((corrNo < 999),"corrNo too high; problem with ID pattern");
    const ParameterSet myPS(itsParmFileName);

    std::string FEIP = myPS.getString("FEConnection.FEIP");
    int         Port = myPS.getInt("FEConnection.FE_Port_Offset") + corrNo;
    LOG_TRACE_FLOW_STR("Create Socket"
		       << " FE IP = " << FEIP
		       << " Port  = " << Port );

    TH_Socket thReq(FEIP,
		    "notused",
		    Port,
		    true); // server at FE side
    
    if (itsFE_side) {  
      LOG_TRACE_COND("we are at the FE side, so we have to connect to the Correlator");
      itsDummyDH[corrNo].setID(2000+corrNo);    // Corr side ID
      aDH.setID(1000+corrNo);                   // FE side ID
      aDH.connectTo (itsCorr_DHs[corrNo],th);   
    } else {
      LOG_TRACE_COND("we are at the Correlatorside, so the FrontEnd will connect to us");
      itsDummyDH[corrNo].setID(1000+corrNo);    // FE side ID
      aDH.setID(2000+corrNo);                   // Corr side ID
      itsFE_DHs[corrNo].connectTo(aDH,th); 
    }
  };



} //namespace
