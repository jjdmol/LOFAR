//# DFTAll.h: Class to process all DFTs
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

#ifndef PSS3_DFTALL_H
#define PSS3_DFTALL_H


#include <DFTServer/DH_DFTRequest.h>
#include <DFTServer/DH_DFTResult.h>
#include <Common/LofarTypes.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{


class DFTAll
{
public:
  explicit DFTAll();

  DFTAll(const DFTAll&);

  ~DFTAll();

  // Set the values.
  void set (double startFreq, double stepFreq, int nFreq,
	    double startTime, double stepTime, int nTime,
	    int nAnt, int nBaseline);

  /// Give write access to the variable sized data fields.
  // The set function should have been called before.
  // <group>
  double* accessUVW();
  int32*  accessAnt();
  int32*  accessAnt1();
  int32*  accessAnt2();
  // </group>

  // Get access to the values in the result.
  const double* getValues() const;

private:
  /// Forbid assignment.
  DFTAll& operator= (const DFTAll&);

  DH_DFTRequest itsRequest;
  DH_DFTResult  itsResult;
};


inline double* DFTAll::accessUVW()
  { return itsRequest.accessUVW(); }
inline int32* DFTAll::accessAnt()
  { return itsRequest.accessAnt(); }
inline int32* DFTAll::accessAnt1()
  { return itsRequest.accessAnt1(); }
inline int32* DFTAll::accessAnt2()
  { return itsRequest.accessAnt2(); }

inline const double* DFTAll::getValues() const
  { return itsResult.getValues(); }
   
}

#endif 
