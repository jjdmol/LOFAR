//# DH_Sync.h: todo: description 
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_DH_SYNC_H
#define TFLOPCORRELATOR_DH_SYNC_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>
#include <complex>

using std::complex;

namespace LOFAR
{

class DH_Sync: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_Sync (const string& name);

  DH_Sync(const DH_Sync&);

  virtual ~DH_Sync();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// accessor functions to the blob data
  const int getDelay() const;
  // return the primairy and secondary timestamps for the start of the next integration sequence
  const void getNextMainBeat(int& primairy,
			     int& secondary) const; 
  void setDelay(int);
  // set the next primairy timestamp to process 
  void setNextPrimairy(int primairy);
		   
 
 private:
  /// Forbid assignment.
  DH_Sync& operator= (const DH_Sync&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  int itsDelay;
  int itsPrimairy;
  int itsSecondary;
};

}
#endif 
