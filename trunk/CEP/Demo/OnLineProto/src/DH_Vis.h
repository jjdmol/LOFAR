//# DH_Vis.h: Visibilities DataHolder
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

#ifndef ONLINEPROTO_DH_VIS_H
#define ONLINEPROTO_DH_VIS_H


#include <lofar_config.h>

#include "Transport/DataHolder.h"
#include <Common/lofar_complex.h>
#include <ACC/ParameterSet.h>

// ToDo: pass these values through configuration parameters
#include <OnLineProto/definitions.h>

namespace LOFAR
{

/**
   TBW
*/
class DH_Vis: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Vis (const string& name, const ACC::ParameterSet& ps);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int station1, int station2);
  const int         getFBW() const;

private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  /// ptrs to data in the blob; used for accessors
  int* itsBeamIDptr;          // beam direction ID
  int* itsStartFrequencyptr;  // frequency offste for this beamlet
  int* itsStartSeqNoptr;      // sequence number since last timestamp
  int* itsFBWptr; // number of frequency channels within this beamlet
  complex<float>*  itsBufferptr;    // array containing frequency spectrum.


  unsigned int itsBufSize;
  const ACC::ParameterSet itsPS;
};

inline DH_Vis::BufferType* DH_Vis::getBuffer()
  { return itsBufferptr; }

inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBufferptr; }

inline DH_Vis::BufferType* DH_Vis::getBufferElement(int s1, int s2)
{ 
  return itsBufferptr+s1*itsPS.getInt("general.nstations")+s2;
}

inline const int DH_Vis::getFBW() const
  { return *itsFBWptr; }

}

#endif 
