//# DH_PhaseCorr.h: dataholder to hold the phase correction information
//#                 to perform station synchronization       
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_DH_PHASECORR_H
#define TFLOPCORRELATOR_DH_PHASECORR_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>

using std::complex;

namespace LOFAR
{

class DH_PhaseCorr: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_PhaseCorr (const string& name, int nrChannels);

  DH_PhaseCorr(const DH_PhaseCorr&);

  virtual ~DH_PhaseCorr();

  DataHolder* clone() const;

  // Allocate the buffers.
  virtual void init();

  // accessor functions to the blob data
  const fcomplex getPhaseCorr(int index) const;
  void setPhaseCorr(int index, fcomplex value);
 
 private:
  /// Forbid assignment.
  DH_PhaseCorr& operator= (const DH_PhaseCorr&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  fcomplex* itsCorrPtr;

  int itsNrChannels;
};


}
#endif 
