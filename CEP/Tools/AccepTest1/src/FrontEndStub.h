//# FrontEndStub.h: Stub for connection between FrontEnd and Correlator
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#include <string>
#include <Acceptest1/DH_CorrCube.h>

namespace LOFAR {

// This class is a stub which is used to make the connection of the FrontEnd 
// to the Correlators

class FrontEndStub
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit FrontEndStub (bool FE_side,       
			 int NoCorrelators,  // #correlators in this interface
			 const std::string& parameterFileName = "DFTServer.param");
  
  ~FrontEndStub();
  
  // Connect the given objects to the stubs.
  void connect (DH_CorrCube&,  // reference to the DH we want to connect
		int corrNo);   // channel number
  
 private:
  bool           itsFE_side;          // defines at which side of the interface the instance is
  int            itsNoCorr;           // #correlators in this interface
  std::string    itsParmFileName;     // name of the param files used for configuration
  DH_CorrCube*   itsDummyDH;          // these are the "dummy" dataholders needed in the connect calls
};
 
} //namespace

