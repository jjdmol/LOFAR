//# Corr_Stub.h: Stub for connection of Correlators with outside world
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef LOFAR_TFLOPCORRELATOR_CORRSTUB_H
#define LOFAR_TFLOPCORRELATOR_CORRSTUB_H


#include <TFlopCorrelator/DH_Vis.h>

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
  void connect (int& C_nr,
		DH_SubBand& sb);

private:
  bool               itsStubOnServer;
  ACC::ParameterSet* itsPS;
  vector<DH_Vis*>    itsCorr;
  int                itsNCorr;  // total number of correlators in this interface
};

} //namespace

#endif // include guard
