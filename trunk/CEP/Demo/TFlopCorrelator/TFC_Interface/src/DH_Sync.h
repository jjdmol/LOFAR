//# DH_Sync.h: dataholder to hold the delay information to perform
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

#ifndef TFLOPCORRELATOR_DH_SYNC_H
#define TFLOPCORRELATOR_DH_SYNC_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>
//#include <complex>

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

  // Allocate the buffers.
  virtual void init();

  // accessor functions to the blob data
  const int getDelay() const;
  // return the primairy and secondary timestamps for the start of the next integration sequence
  const void getNextMainBeat(int& seqid,
			     int& blockid) const; 
  void setDelay(int);
  // set the next sequence ID to process 
  void setNextPrimairy(int seqid);
		   
 
 private:
  /// Forbid assignment.
  DH_Sync& operator= (const DH_Sync&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  int* itsDelayPtr;
  int* itsSeqIdPtr;
  int* itsBlockIdPtr;
};

inline const int DH_Sync::getDelay() const
  { return *itsDelayPtr; }

inline const void DH_Sync::getNextMainBeat(int& seqid,
			                   int& blockid) const
  { seqid = *itsSeqIdPtr; blockid = *itsBlockIdPtr;} 

inline void DH_Sync::setDelay(int delay)
  { *itsDelayPtr = delay; *itsBlockIdPtr += delay;}

inline void DH_Sync::setNextPrimairy(int seqid)
  { *itsSeqIdPtr = seqid; }

}
#endif 
