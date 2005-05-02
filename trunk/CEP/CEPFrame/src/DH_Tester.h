//# DH_Tester.h:
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

#ifndef CEPFRAME_DH_TESTER_H
#define CEPFRAME_DH_TESTER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/lofar_complex.h>
#include <Transport/DataHolder.h>
#include <Transport/BaseSim.h>

namespace LOFAR
{

/**
   This class is a simple DataHolder test class for the program Tester.
*/

class DH_Tester: public DataHolder
{
public:
  typedef fcomplex DataBufferType;

  explicit DH_Tester (const string& name);
  DH_Tester (const DH_Tester&);
  virtual ~DH_Tester();

  DataHolder* clone() const;

  void setCounter (int counter);
  int getCounter() const;

  /// Allocate the buffers.
  virtual void preprocess();
  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the buffer
  DataBufferType* getBuffer();
  /// Get read access to the buffer
  const DataBufferType* getBuffer() const;

private:
  /// Forbid assignment.
  DH_Tester& operator= (const DH_Tester&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*             itsCounter;
  DataBufferType*  itsBuffer;

};

inline void DH_Tester::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_Tester::getCounter() const
  { return *itsCounter; }

inline DH_Tester::DataBufferType* DH_Tester::getBuffer()
  { return itsBuffer; }

inline const DH_Tester::DataBufferType* DH_Tester::getBuffer() const
  { return itsBuffer; }

}

#endif 
