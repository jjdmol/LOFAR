//# DH_CorrectionMatrix.h: CorrectionMatrix DataHolder
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

#ifndef ONLINEPROTO_DH_CORRECTIONMATRIX_H
#define ONLINEPROTO_DH_CORRECTIONMATRIX_H


#include <lofar_config.h>

#include "Transport/DataHolder.h"
#include <Common/lofar_complex.h>
#include <Common/Debug.h>


namespace LOFAR
{
class DH_CorrectionMatrix: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_CorrectionMatrix (const string& name, 
				const int nx,
				const int ny);

  DH_CorrectionMatrix(const DH_CorrectionMatrix&);

  virtual ~DH_CorrectionMatrix();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();


  /// Accessor functions to data in the blob
  complex<float>*       getBuffer();
  const complex<float>* getBuffer() const;
  complex<float>*       getBufferElement(int nx, int ny);
  float                 getElapsedTime () const;
  void                  setElapsedTime (float time);


private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// Forbid assignment.
    DH_CorrectionMatrix& operator= (const DH_CorrectionMatrix&);

    /// ptrs to data fileds in the blob; used for accessors
    complex<float>*  itsBufferptr;     // array containing frequency spectrum.
    float*           itsElapsedTimeptr;

    // administration; filled in C'tor, used in preprocess
    int itsNx; // ??
    int itsNy; // ??
};

inline DH_CorrectionMatrix::BufferType* DH_CorrectionMatrix::getBuffer()
  { return itsBufferptr; }

inline const DH_CorrectionMatrix::BufferType* DH_CorrectionMatrix::getBuffer() const
  { return itsBufferptr; }

inline DH_CorrectionMatrix::BufferType* DH_CorrectionMatrix::getBufferElement(int x, int y)
  { 
    DbgAssertStr(x < itsNx && y < itsNy,"Offset values out of range for the chosen correction matrix...");
    return itsBufferptr + x + y * itsNx;      
  }

inline float DH_CorrectionMatrix::getElapsedTime () const
  { DbgAssertStr((itsElapsedTimeptr > 0) 
		 && (*itsElapsedTimeptr >= 0), 
		 "itsElapsedTime not initialised"); 
    return *itsElapsedTimeptr; 
  }

inline void DH_CorrectionMatrix::setElapsedTime (float time)
  {  *itsElapsedTimeptr = time; }
}

#endif 
