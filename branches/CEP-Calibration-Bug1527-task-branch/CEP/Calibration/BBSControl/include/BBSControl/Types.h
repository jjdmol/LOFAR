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
#include <Common/ParameterSet.h>

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
    struct Selection
    {
      Selection() : type("ANY") {}
      Selection(const ParameterSet &ps);

      string                    type; // One of "ANY", "AUTO", "CROSS".
      vector<vector<string> >   baselines;
      vector<string>            correlations;
    };

    // Attempt to read the contents of a Selection instance from a ParameterSet.
    void fromParameterSet(const ParameterSet &ps, Selection &selection);

    // Cell size is defined along the frequency and the time axis, in number
    // of channels and number of timeslots respectively.
    struct CellSize
    {
      CellSize() : freq(0), time(0) {}
      CellSize(uint32 freq, uint32 time) : freq(freq), time(time) {}
      uint32 freq;	         ///< Size in frequency (number of channels).
      uint32 time;           ///< Size in time (number of timeslots).
    };

    // Write the contents of these types in human readable form.
    // @{
    ostream& operator<<(ostream&, const Selection&);
    ostream& operator<<(ostream&, const CellSize&);
    // @}

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
