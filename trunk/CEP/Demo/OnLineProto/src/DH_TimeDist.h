//# DH_TimeDist.h: TimeDist DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef ONLINEPROTO_DH_TIMEDIST_H
#define ONLINEPROTO_DH_TIMEDIST_H


#include <lofar_config.h>

#include "Transport/DataHolder.h"
#include <Common/lofar_complex.h>
#include <Common/Debug.h>


namespace LOFAR
{
class DH_TimeDist: public DataHolder
{
public:
  typedef float BufferType;

  explicit DH_TimeDist (const string& name);

  DH_TimeDist(const DH_TimeDist&);

  virtual ~DH_TimeDist();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Accessors to Blob fields
  int getTime() const;
  int getDeltaTime() const;

  //  float getElapsedTime () const;
  //void setElapsedTime (float time);
  //int getStationID() const;

private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// Forbid assignment.
  DH_TimeDist& operator= (const DH_TimeDist&);
  
  /// ptrs to the blobl fields; used for accessors
  int*  itsTimeptr;     
  int*  itsDeltaTimeptr;     
  
};
 
inline int DH_TimeDist::getTime() const
  { return *itsTimeptr; }

inline int DH_TimeDist::getDeltaTime() const
  { return *itsDeltaTimeptr; }
}

#endif 
