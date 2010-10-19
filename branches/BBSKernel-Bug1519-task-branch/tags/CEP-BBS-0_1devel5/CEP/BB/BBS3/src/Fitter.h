//# Fitter.h: Hold one or more fitter objects.
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

#ifndef LOFAR_BB_BBS3_FITTER_H
#define LOFAR_BB_BBS3_FITTER_H

// \file
// Calculates parameter values using a least squares fitter

#include <scimath/Fitting/LSQFit.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

// Fitter calculates new parameter values from the equations given by the
// Prediffer class.
class Fitter
{

public:
  // Set the nr of fitters and the nr of unknowns in each fitter.
  // Setting nfitter to 0 removes all fitters.
  void set (int nfitter, int nunknown=0);

  // Merge another Fitter object into this one.
  // It must have the same nr of fitters and unknowns per fitter.
  void merge (const Fitter&);

  // Marshall the fitter object into a buffer.
  void marshall (void* buffer, int bufferSize) const;

  // Demarshall the fitter object from a buffer.
  void demarshall (const void* buffer, int bufferSize);

private:
  std::vector<casa::LSQFit> itsFitters;
};

// @}

} // namespace LOFAR

#endif
