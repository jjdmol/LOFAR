//# DH_CorrCube.h: CorrCube DataHolder
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

#ifndef ONLINEPROTO_DH_ARRAYTFPLANE_H
#define ONLINEPROTO_DH_ARRAYTFPLANE_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <ACC/ParameterSet.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_CorrCube: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_CorrCube (const string& name, 
			const ACC::ParameterSet& ps);

  DH_CorrCube(const DH_CorrCube&);

  virtual ~DH_CorrCube();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int station, int time, int freq);
  void setBufferElement(int station, int time, int freq, BufferType* value); 
   
   const int         getFBW() const;

private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

   /// Forbid assignment.
   DH_CorrCube& operator= (const DH_CorrCube&);
   
   /// pointers to elements in the blob; used for accessors
   int* itsBeamIDptr;            // beam direction ID
   int* itsStartFrequencyptr;    // frequency offste for this beamlet
   int* itsStartSeqNoptr;        // sequence number since last timestamp
   int* itsFBWptr;               // number of frequency channels within this beamlet
   complex<float>* itsBufferptr; // array containing frequency spectrum.

   int itsBufSize;  // calculate required buffer size in C'tor  
   const ACC::ParameterSet itsPS; // necessary configuration parameters
};

inline DH_CorrCube::BufferType* DH_CorrCube::getBuffer()
  { return itsBufferptr; }

inline const DH_CorrCube::BufferType* DH_CorrCube::getBuffer() const
  { return itsBufferptr; }

inline DH_CorrCube::BufferType* DH_CorrCube::getBufferElement(int station, 
							      int time, 
							      int freq)
{ 
  // todo: cache corr. sizes
  return itsBufferptr+station*itsPS.getInt("corr.tsize")*itsPS.getInt("corr.fsize")
    +time*itsPS.getInt("corr.fsize")+freq;
}
   
inline void DH_CorrCube::setBufferElement(int station, 
			     int time, 
			     int freq, 
  			     DH_CorrCube::BufferType* valueptr) 
{
  // todo: cache corr. sizes
  complex<float>* ptr= itsBufferptr+station*itsPS.getInt("corr.tsize")*
    itsPS.getInt("corr.fsize")+time*itsPS.getInt("corr.fsize")+freq;
  *ptr = *valueptr;
}

inline const int DH_CorrCube::getFBW() const
{ return *itsFBWptr; }

} //namespace

#endif 
