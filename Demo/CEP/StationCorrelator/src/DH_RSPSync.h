//# DH_RSPSync.h: DataHolder used to synchronise the WH_RSPs
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//# $Id$

#ifndef DH_RSPSYNC_H
#define DH_RSPSYNC_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <SyncStamp.h>

namespace LOFAR
{
  class DH_RSPSync: public DataHolder
{
public:
  typedef SyncStamp syncStamp_t;

  explicit DH_RSPSync (const string& name);

  DH_RSPSync(const DH_RSPSync&);

  virtual ~DH_RSPSync();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Set the sync stamp
  void setSyncStamp(const syncStamp_t syncStamp);

  /// Get the sync stamp
  const syncStamp_t getSyncStamp() const;
  void incrementStamp(const int value);

private:
  /// Forbid assignment.
  DH_RSPSync& operator= (const DH_RSPSync&);

  syncStamp_t*  itsSyncStamp;

  void fillDataPointers();
};

inline void DH_RSPSync::setSyncStamp(const DH_RSPSync::syncStamp_t syncStamp)
  { *itsSyncStamp = syncStamp; }
 
inline const DH_RSPSync::syncStamp_t DH_RSPSync::getSyncStamp() const
  { return *itsSyncStamp;}

inline void DH_RSPSync::incrementStamp(const int value)
  { *itsSyncStamp += value;}
}
#endif 
