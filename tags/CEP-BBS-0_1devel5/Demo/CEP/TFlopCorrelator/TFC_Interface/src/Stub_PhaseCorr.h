//# Stub_PhaseCorr.h: Stub for connection of delay control with 
//# poly phase filter
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$

#ifndef LOFAR_TFC_INTERFACE_STUB_PHASECORR_H
#define LOFAR_TFC_INTERFACE_STUB_PHASECORR_H

#include <APS/ParameterSet.h>
#include <tinyCEP/TinyDataManager.h>

namespace LOFAR {

// This class is a stub which is used to make the connection of the SubBandFilter
// to the TflopCorrelator input section 

class TH_Socket;
class Connection;

class Stub_PhaseCorr
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit Stub_PhaseCorr (bool inBGLProc, const ACC::APS::ParameterSet pSet);

  ~Stub_PhaseCorr();

  // Connect the given objects to the stubs.
  void connect (TinyDataManager& dm,
		int dhNr);

private:
  bool                   itsInBGLProc;    // Does this stub run in BGL Proc?
  ACC::APS::ParameterSet itsPS;
  TH_Socket*             itsTH;
  Connection*            itsConnection;
};

} //namespace

#endif // include guard
