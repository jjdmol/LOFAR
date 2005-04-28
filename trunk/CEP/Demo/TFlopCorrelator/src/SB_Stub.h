//# SB_Stub.h: Stub for connection of SB filter with outside world
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#include <string>
#include <TFlopCorrelator/DH_SubBand.h>
//#include <TFlopCorrelator/DH_FilterCoeff.h>


namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class SB_Stub
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit SB_Stub (bool onServer=false);

  ~SB_Stub();

  // Connect the given objects to the stubs.
  void connect (DH_SubBand& sb);

private:
  bool          itsStubOnServer;
  std::string   itsParmFileName;
  DH_SubBand    itsSB;

};

} //namespace

