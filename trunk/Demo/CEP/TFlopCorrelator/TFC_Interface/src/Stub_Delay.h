//# Stub_Delay.h: Stub for connection of delay control with RSP inputs
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef LOFAR_TFC_INTERFACE_STUB_DELAY_H
#define LOFAR_TFC_INTERFACE_STUB_DELAY_H

#include <ACC/ParameterSet.h>
#include <tinyCEP/TinyDataManager.h>

namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class TH_Socket;
class Connection;

class Stub_Delay
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_Delay (bool isInput);

  ~Stub_Delay();

  // Connect the given objects to the stubs.
  void connect (int RSP_nr,
		TinyDataManager& dm,
		int dhNr);

private:
  bool               itsIsInput;    // Is this stub an input for a step
  ACC::ParameterSet* itsPS;
  int                itsNRSP;  // total number of RSPinputs
  TH_Socket**        itsTHs;
  Connection**       itsConnections;
};

} //namespace

#endif // include guard
