//# Types.h: Some global types.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_BBSTYPES_H
#define LOFAR_BBSCONTROL_BBSTYPES_H

// \file
// Some global types. The main purpose of these types is to bundle data
// that are logically related. Most of these types are used in more than one
// class, which justifies them being defined here, outside these classes.

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // Typedefs
    // @{
    typedef int32 CommandId;
    typedef int32 KernelIndex;
    // @}

    // Information about which correlation products (auto, cross, or both),
    // and which polarizations should be used.
    struct Correlation
    {
      Correlation() : selection("NONE") {}
      string selection;     ///< Valid values: "NONE", "AUTO", "CROSS", "ALL"
      vector<string> type;  ///< E.g., ["XX", "XY", "YX", "YY"]
    };

    // Two vectors of stations names, which, when paired element-wise, define
    // the baselines to be used in the current step. Names may contain
    // wildcards, like \c * and \c ?. If they do, then all possible baselines
    // will be constructed from the expanded names. Expansion of wildcards
    // will be done in the BBS kernel.
    // 
    // For example, suppose that: 
    // \verbatim 
    // station1 = ["CS*", "RS1"]
    // station2 = ["CS*", "RS2"] 
    // \endverbatim
    // Furthermore, suppose that \c CS* expands to \c CS1, \c CS2, and \c
    // CS3. Then, in the BBS kernel, seven baselines will be constructed:
    // \verbatim
    // [ CS1:CS1, CS1:CS2, CS1:CS3, CS2:CS2, CS2:CS3, CS3:CS3, RS1:RS2 ]
    // \endverbatim
    // 
    // \note Station names are \e not expanded by matching with all existing
    // %LOFAR stations, but only with those that took part in a particular
    // observation; i.e., only those stations that are mentioned in the \c
    // ANTENNA table in the Measurement Set.
    struct Baselines
    {
      vector<string> station1;
      vector<string> station2;
    };

    // Cell size is defined along the frequency and the time axis, in number
    // of channels and number of timeslots respectively.
    struct CellSize
    {
      CellSize() : freq(0), time(0) {}
      uint32 freq;	         ///< Size in frequency (number of channels).
      uint32 time;           ///< Size in time (number of timeslots).
    };

    // Options for the solver.
    struct SolverOptions
    {
      SolverOptions() : maxIter(0), epsValue(0), epsDerivative(0),
                        colFactor(0), lmFactor(0), balancedEqs(false),
                        useSVD(false) {}
      uint32 maxIter;        ///< Maximum number of iterations
      double epsValue;       ///< Value convergence threshold
      double epsDerivative;  ///< Derivative convergence threshold
      double colFactor;      ///< Colinearity factor
      double lmFactor;       ///< Levenberg-Marquardt factor
      bool   balancedEqs;    ///< Indicates well-balanced normal equations
      bool   useSVD;         ///< Use singular value decomposition.
    };

    // Write the contents of these types in human readable form.
    // @{
    ostream& operator<<(ostream&, const Correlation&);
    ostream& operator<<(ostream&, const Baselines&);
    ostream& operator<<(ostream&, const CellSize&);
    ostream& operator<<(ostream&, const SolverOptions&);
    // @}

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
