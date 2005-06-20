//# DH_RSPInputSync.h: DataHolder used to synchronize incoming RSP data
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_RSPINPUTSYNC_H
#define TFLOPCORRELATOR_DH_RSPINPUTSYNC_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <RSPTimeStamp.h>

namespace LOFAR
{
  class DH_RSPInputSync: public DataHolder
{
public:

  explicit DH_RSPInputSync (const string& name);

  DH_RSPInputSync(const DH_RSPInputSync&);

  virtual ~DH_RSPInputSync();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Set the sync stamp
  void setSyncStamp(const timestamp_t syncStamp);

  /// Get the sync stamp
  const timestamp_t getSyncStamp() const;
  void incrementStamp(const int value);

private:
  /// Forbid assignment.
  DH_RSPInputSync& operator= (const DH_RSPInputSync&);

  timestamp_t*  itsSyncStamp;

  void fillDataPointers();
};

inline void DH_RSPInputSync::setSyncStamp(const timestamp_t syncStamp)
  { *itsSyncStamp = syncStamp; }
 
inline const timestamp_t DH_RSPInputSync::getSyncStamp() const
  { return *itsSyncStamp;}

inline void DH_RSPInputSync::incrementStamp(const int value)
  { *itsSyncStamp += value;}
}
#endif 
