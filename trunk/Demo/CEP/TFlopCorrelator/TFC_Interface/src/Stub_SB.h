//# Stub_SB.h: Stub for connection of SB filter with outside world
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef LOFAR_TFLOPCORRELATOR_SBSTUB_H
#define LOFAR_TFLOPCORRELATOR_SBSTUB_H

#include <string>
#include <ACC/ParameterSet.h>
#include <TFC_Interface/DH_SubBand.h>
//#include <TFlopCorrelator/DH_FilterCoeff.h>


namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class Stub_SB
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_SB (bool onServer=false);

  ~Stub_SB();

  // Connect the given objects to the stubs.
  void connect (int SBF_nr,
		DH_SubBand* sb);

private:
  bool                itsStubOnServer;
  ACC::ParameterSet*  itsPS;
  vector<DH_SubBand*> itsSB;
  int                 itsNSBF;
};

} //namespace

#endif //include guard 

