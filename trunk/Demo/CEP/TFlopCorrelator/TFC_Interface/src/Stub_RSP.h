//# Stub_RSP.h: Stub for connection of SB filter with outside world
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
#include <APS/ParameterSet.h>


namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class Stub_RSP
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_RSP (bool onServer=false);

  ~Stub_RSP();

  // Connect the given objects to the stubs.
  void connect ();

private:
  bool                itsStubOnServer;
  ACC::APS::ParameterSet*  itsPS;
};

} //namespace

#endif //include guard 

