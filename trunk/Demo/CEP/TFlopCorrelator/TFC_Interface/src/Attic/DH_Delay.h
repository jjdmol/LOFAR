//# DH_Delay.h: dataholder to hold the delay information to perform
//#            station synchronizaion       
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_DH_DELAY_H
#define TFLOPCORRELATOR_DH_DELAY_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>

using std::complex;

namespace LOFAR
{

class DH_Delay: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_Delay (const string& name, int nrRSPs);

  DH_Delay(const DH_Delay&);

  virtual ~DH_Delay();

  DataHolder* clone() const;

  // Allocate the buffers.
  virtual void init();

  // accessor functions to the blob data
  const int getDelayChange(int index) const;
  void setDelayChange(int index, int value);
 
 private:
  /// Forbid assignment.
  DH_Delay& operator= (const DH_Delay&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  int* itsDelayPtr;

  int itsNrRSPs;
};


}
#endif 
