//# Stub_Corr.h: Stub for connection of Correlators with outside world
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef LOFAR_TFLOPCORRELATOR_CORRSTUB_H
#define LOFAR_TFLOPCORRELATOR_CORRSTUB_H

#include <APS/ParameterSet.h>
#include <tinyCEP/TinyDataManager.h>

namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class TH_Socket;
class Connection;

class Stub_Corr
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_Corr (bool onServer=false);

  ~Stub_Corr();

  // Connect the given objects to the stubs.
  void connect (int C_nr,
		TinyDataManager& dm,
		int dhNr);

private:
  bool               itsStubOnServer;
  ACC::APS::ParameterSet* itsPS;
  int                itsNCorr;  // total number of correlators in this interface
  TH_Socket**        itsTHs;
  Connection**       itsConnections;
};

} //namespace

#endif // include guard
