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

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <Common/Debug.h>


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

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();

  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  float getElapsedTime () const;
  void setElapsedTime (float time);
  int getStationID() const;

protected:
  // Definition of the DataPacket type.
/*   class DataPacket: public DataHolder::DataPacket */
/*   { */
/*   public: */
/*     DataPacket(){}; */
/*     BufferType itsFill;         // to ensure alignment */

/*     int   itsStationID;        // source station ID */
/*     float itsElapsedTime;      // the hourangle */
/*   }; */

private:
  /// Forbid assignment.
    DH_Phase& operator= (const DH_Phase&);

    DataPacket*  itsDataPacket;    
    BufferType*  itsBuffer;     // array containing frequency spectrum.
    unsigned int itsBufSize;  

    float itsElapsedTime;
    int   itsStationID;

    void fillDataPointers();
};

inline DH_Phase::BufferType* DH_Phase::getBuffer()
  { return itsBuffer; }

inline const DH_Phase::BufferType* DH_Phase::getBuffer() const
  { return itsBuffer; }

inline float DH_Phase::getElapsedTime () const
  { DbgAssertStr(itsElapsedTime >= 0, "itsElapsedTime not initialised"); 
    return itsElapsedTime; 
  }

inline void DH_Phase::setElapsedTime (float time)
  {  itsElapsedTime = time; }

inline int DH_Phase::getStationID() const
  { DbgAssertStr(itsStationID >= 0, "itsStationID not initialised"); 
    return itsStationID; 
  }
}

#endif 
