//# DH_DFTResult.h: Output DataHolder for the DFT
//#
//# Copyright (C) 2004
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

#ifndef BB_DH_OUTDFT_H
#define BB_DH_OUTDFT_H


#include <Common/LofarTypes.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{


class DH_DFTResult: public DataHolder
{
public:
  explicit DH_DFTResult();

  DH_DFTResult(const DH_DFTResult&);

  virtual ~DH_DFTResult();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get read access to the various data fields.
  // The data is a double complex, but it is handled as 2 doubles.
  // <group>
  const double* getValues() const;
  int getNFreq() const;
  int getNTime() const;
  int getNBaseline() const;
  // </group>

  // Set the size of the data fields.
  // It will create the data block.
  void set (int nFreq, int nTime, int nBaseline);

  /// Give write access to the variable sized.
  // The set function should have been called before.
  // <group>
  double* accessValues();

  /// Forbid assignment.
  DH_DFTResult& operator= (const DH_DFTResult&);

  // Fill the pointers to the data in the blob.
  virtual void fillDataPointers();


  double* itsValues;
  uint32  itsNTime;
  uint32  itsNFreq;
  uint32  itsNBaseline;
};


inline const double* DH_DFTResult::getValues() const
  { return itsValues; }
inline int DH_DFTResult::getNFreq() const
  { return itsNFreq; }
inline int DH_DFTResult::getNTime() const
  { return itsNTime; }
inline int DH_DFTResult::getNBaseline() const
  { return itsNBaseline; }

inline double* DH_DFTResult::accessValues()
  { return itsValues; }

   
}

#endif 
   
