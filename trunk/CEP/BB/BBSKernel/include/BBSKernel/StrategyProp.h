//# Strategyprop.h: Calibration strategy properties
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

#ifndef LOFAR_BB_BBS_STRATEGYPROP_H
#define LOFAR_BB_BBS_STRATEGYPROP_H

// \file
// Calibration strategy properties.

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <casa/Utilities/Regex.h>
#include <casa/Arrays/Vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class StrategyProp
{
public:
  // Default constructor (for containers).
  StrategyProp()
    : itsInColumn("DATA"), itsAutoCorr(false)
    {}

  // Set/get the input data column to use.
  // <group>
  void setInColumn (const string inColumn)
    { itsInColumn = inColumn; }
  const string& getInColumn() const
    { return itsInColumn; }
  // </group>

  // Set/get the antennas to use (0-based numbers).
  // <group>
  void setAntennas (const vector<int>& antNrs)
    { itsAntNrs = antNrs; }
  const vector<int>& getAntennas() const
    { return itsAntNrs; }
  // </group>

  // Set antennas by name patterns.
  // Each name can be a filename-like pattern.
  // <group>
  void setAntennas (const vector<string>& antNamePatterns);
  void expandPatterns (const vector<string>& antNames) const;
  // </group>

  // Set/get the autocorrelations flag.
  // <group>
  void setAutoCorr (bool autoCorr)
    { itsAutoCorr = autoCorr; }
  bool getAutoCorr() const
    { return itsAutoCorr; }
  // </group>

  // Set/get the correlations to use.
  // <group>
  void setCorr (const vector<bool>& corr)
    { itsCorr = corr; }
  const vector<bool>& getCorr() const
    { return itsCorr; }
  // </group>


private:
  string              itsInColumn;
  mutable vector<int> itsAntNrs;
  vector<casa::Regex> itsAntRegex;
  bool                itsAutoCorr;
  vector<bool>        itsCorr;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
