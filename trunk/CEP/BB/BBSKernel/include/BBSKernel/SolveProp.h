//# SolveProp.h: Solve properties
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

#ifndef LOFAR_BB_BBS_SOLVEPROP_H
#define LOFAR_BB_BBS_SOLVEPROP_H

// \file
// Solve properties.

#include <BBSKernel/MNS/MeqDomain.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class SolveProp
{
public:
  // Default constructor (for containers).
  SolveProp()
    : itsMaxIter(10), itsEpsilon(1e-5), itsFraction(0.95), itsUseSVD(true)
    {}

  // Set/get the parameter name patterns.
  // <group>
  void setParmPatterns (const vector<string>& parms)
    { itsParmPatterns = parms; }
  const vector<string>& getParmPatterns() const
    { return itsParmPatterns; }
  // </group>

  // Set/get the parameter name patterns to be excluded.
  // <group>
  void setExclPatterns (const vector<string>& parms)
    { itsExclPatterns = parms; }
  const vector<string>& getExclPatterns() const
    { return itsExclPatterns; }
  // </group>

  // Set/get the solve domains.
  // <group>
  void setDomains (const vector<MeqDomain>& domains)
    { itsDomains = domains; }
  const vector<MeqDomain>& getDomains() const
    { return itsDomains; }
  // </group>

  // Set/get the max nr of iterations.
  // By default it is 10.
  // <group>
  void setMaxIter (int maxIter)
    { itsMaxIter = maxIter; }
  int getMaxIter() const
    { return itsMaxIter; }
  // </group>

  // Set/get the convergence epsilon.
  // A fitter has converged if
  //    abs(sol - lastsol) / max(abs(lastsol), abs(sol)) < epsilon.
  // By default it is 1e-5.
  // <group>
  void setEpsilon (double epsilon)
    { itsEpsilon = epsilon; }
  double getEpsilon() const
    { return itsEpsilon; }
  // </group>

  // Set/get the fraction of fitters that have to converge.
  // By default it is 0.95.
  // <group>
  void setFraction (double fraction)
    { itsFraction = fraction; }
  double getFraction() const
    { return itsFraction; }
  // </group>

  // Set/get if Singular Value Decompostion has to be used.
  // By default it is used.
  // <group>
  void setUseSVD (double useSVD)
    { itsUseSVD = useSVD; }
  double getUseSVD() const
    { return itsUseSVD; }
  // </group>

private:
  vector<string>    itsParmPatterns;
  vector<string>    itsExclPatterns;
  vector<MeqDomain> itsDomains;
  int               itsMaxIter;
  double            itsEpsilon;
  double            itsFraction;    // fraction of fitters to be converged
  bool              itsUseSVD;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
