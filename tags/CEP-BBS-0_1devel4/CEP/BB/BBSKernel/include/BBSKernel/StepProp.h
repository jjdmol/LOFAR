//# Stepprop.h: Calibration step properties
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

#ifndef LOFAR_BB_BBS_STEPPROP_H
#define LOFAR_BB_BBS_STEPPROP_H

// \file
// Calibration step properties.

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

class StepProp
{
public:
  // Default constructor (for containers).
  StepProp()
    : itsOutColumn("CORRECTED_DATA"), itsAutoCorr(false)
    {}

  // Set/get the instrument model.
  // <group>
  void setModel (const vector<string>& model)
    { itsModel = model; }
  const vector<string>& getModel() const
    { return itsModel; }
  // </group>

  // Set/get the output column to use.
  // <group>
  void setOutColumn (const string& outColumn)
    { itsOutColumn = outColumn; }
  const string& getOutColumn() const
    { return itsOutColumn; }
  // </group>

  // Set/get the sources to use.
  // <group>
  void setSources (const vector<string>& sources)
    { itsSources = sources; }
  const vector<string>& getSources() const
    { return itsSources; }
  // </group>

  // Set/get the baselines to use (0-based antenna numbers).
  // <group>
  void setBaselines (const vector<int>& ant1Nrs, const vector<int>& ant2Nrs);
  const vector<int>& getAnt1() const
    { return itsAnt1; }
  const vector<int>& getAnt2() const
    { return itsAnt2; }
  // </group>

  // Set baselines by groups.
  // Baselines are formed by combining the antennae in the
  // corresponding groups.
  void setBaselines (const vector<vector<int> >& ant1Groups,
		     const vector<vector<int> >& ant2Groups);

  // Set baselines by name patterns.
  // Each name can be a filename-like pattern.
  // Baselines are formed by function expandPatterns which expands each
  // pattern into a group of numbers of antenna names matching the pattern.
  // <group>
  void setBaselines (const vector<string>& ant1NamePatterns,
		     const vector<string>& ant2NamePatterns);
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

  // Set/get the phase center.
  // <group>
  void setPhaseCenter (double ra, double dec)
    { itsPCRA = ra; itsPCDEC = dec; }
  double getPCRA() const
    { return itsPCRA; }
  double getPCDEC() const
    { return itsPCDEC; }
  // </group>

private:
  // Form baselines by combining the antennae.
  void formBaselines (const vector<int>& ant1, const vector<int>& ant2) const;

  vector<string>      itsModel;
  string              itsOutColumn;
  vector<string>      itsSources;
  mutable vector<int> itsAnt1;
  mutable vector<int> itsAnt2;
  vector<casa::Regex> itsAnt1Regex;
  vector<casa::Regex> itsAnt2Regex;
  bool                itsAutoCorr;
  vector<bool>        itsCorr;
  double              itsPCRA;
  double              itsPCDEC;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
