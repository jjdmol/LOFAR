//# MSDataArray.h: Class holding pointers to the MS data
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBS_MSDATAARRAY_H
#define LOFAR_BBS_MSDATAARRAY_H

// \file
// Class holding pointers to the MS data.
// The data can be in a mapped file or in an array in memory.

//# Includes

namespace LOFAR
{
namespace BBS
{
 
// \ingroup BBSKernel
// @{

class MSDataArray
{
public:
  MSDataArray (float* ptr, int stepChan, int stepTime)
    : itsPtr(ptr), itsStepChan(stepChan), itsStepTime(stepTime)
  {}

  float* getData (int timeStep)
    { return itsPtr + timeStep*stepTime; }

  int stepChan() const
    { return itsStepChan; }

private:
  float* itsData;
  int    itsStepChan;
  int    itsStepTime;
};


// @}

} // namespace BBS
} // namespace LOFAR

#endif
