//# DH_Phase.h: Phase DataHolder
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

#ifndef ONLINEPROTO_DH_PHASE_H
#define ONLINEPROTO_DH_PHASE_H


#include <lofar_config.h>

#include "Transport/DataHolder.h"
#include <complex>
#include <Common/Debug.h>

using std::complex;

namespace LOFAR
{
class DH_Phase: public DataHolder
{
public:
  typedef float BufferType;

  explicit DH_Phase (const string& name, 
		     const int StationID);

  DH_Phase(const DH_Phase&);

  virtual ~DH_Phase();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  // accessor functions to the blob data
  void setPhaseAngle(float);
  const complex<float> getPhaseFactor() const;
  const float getPhaseAngle() const;
  float getElapsedTime () const;
  void setElapsedTime (float time);
  int getStationID() const;


private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// Forbid assignment.
  DH_Phase& operator= (const DH_Phase&);
  
  /// ptrs to data in the blob; used for accessors
  complex<float>*  itsBufferptr;     // array containing frequency spectrum.
  float* itsPhaseptr;            // the phase rotation in rad
  int*   itsStationIDptr;        // source station ID
  float* itsElapsedTimeptr;      // the hourangle

  //temporary values are stored between C'tor and preprocess
  int itsStationID;
  float itsElapsedTime;
};
 
inline void DH_Phase::setPhaseAngle(float angle)
  { *itsPhaseptr = angle; }

inline const float DH_Phase::getPhaseAngle() const
  { return *itsPhaseptr; }

inline const complex<float> DH_Phase::getPhaseFactor() const
  { return complex<float>(std::cos(*itsPhaseptr),std::sin(*itsPhaseptr)); }

inline float DH_Phase::getElapsedTime () const
  { DbgAssertStr(*itsElapsedTimeptr >= 0, "itsElapsedTime not initialised"); 
    return *itsElapsedTimeptr; 
  }

inline void DH_Phase::setElapsedTime (float time)
  {  *itsElapsedTimeptr = time; }

inline int DH_Phase::getStationID() const
  { DbgAssertStr(*itsStationIDptr >= 0, "itsStationID not initialised"); 
    return *itsStationIDptr; 
  }
}

#endif 
